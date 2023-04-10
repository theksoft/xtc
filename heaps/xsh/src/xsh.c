#include "xsh.h"
#include <stdint.h>
#include <assert.h>

//==================================================================================================
// Local inline functions
//==================================================================================================

/**
 * @brief Get the structure heap object identifier.
 * 
 * @return
 * This function returns the structure heap identifier as @c xtc_heaps_id_t.
 */

static xtc_heaps_id_t get_id() {
  static unsigned int ref = 0x0FEEDBABE;
  return (xtc_heaps_id_t)(uintptr_t)&ref;
}

/**
 * @brief Check that structure heap node size is compatible with integer alignment.
 * 
 * C langage ref.: Structure must be aligned on integer boundary.
 * 
 * This function is used only during heap structure initialization.
 * It returns nothing but assert() if @c xsh_node_t type size is not compatible.
 */

static inline void check_node_size() {
#ifdef __DEBUG
  struct {
    xsh_node_t n1;
    struct {
      int dummy;
    } n2;
  } x;
  // Check possible integer size issue
  assert(sizeof(xsh_node_t) == (uintptr_t)&x.n2 - (uintptr_t)&x.n1);
#endif
}

/**
 * @brief Check that the provided heap is the expected type.
 *
 * As the heap structure has been cast in more generic type @c xtc_heap_t,
 * this function purpose ensure the right type of heap has been provided
 * e.g. is a @c xsh_heap_t type.
 *
 * @param this Provided heap structure.
 * @return
 * This function returns back the provided heap structure if the latter is correct
 * or @c NULL if it is not the expected structure.
 */

static inline xsh_heap_t* check(xsh_heap_t *this) {
  return this && this->interface.id == get_id() ? this : NULL;
}

//==================================================================================================
// Local functions declaration
//==================================================================================================

static xsh_node_t* get_node(xsh_heap_t *this, void *ptr);

/**
 * @brief Dummy protection function with empty implementation.
 * 
 * It is called when no protection function is called when no valid protection function is called.
 * It avoids testing protection function validity during control flow execution.
 */
static void dummy() {}

//==================================================================================================
// Interface functions
//==================================================================================================

xtc_heap_t* xsh_init(xsh_heap_t *this, void *mem, size_t length, size_t size, xtc_protect_t *protect) {
  check_node_size();
  xtc_heap_t *rtn = NULL;
  size_t node_size = XTC_ALIGNED_SIZE(size) + sizeof(xsh_node_t);
  if (this && mem && size && node_size <= length) {
    if (protect && protect->lock && protect->unlock) {
      this->interface.id = get_id();
      this->interface.alloc = (xtc_alloc_t)xsh_alloc;
      this->interface.free = (xtc_free_t)xsh_free;
      this->mem_pool = mem;
      this->mem_length = length;
      this->size = size;
      this->node_size = node_size;
      this->node_offset = sizeof(xsh_node_t);
      // Initialize free list
      {
        this->lfree = (xsh_node_t*)this->mem_pool;
        size_t count = this->mem_length / this->node_size;
        assert(count);
        size_t i;
        xsh_node_t *node;
        for (i = 0, node = this->lfree; i < count - 1; node = node->next, i++) {
          node->next = (xsh_node_t*)((uint8_t*)node + this->node_size);
        }
        node->next = NULL;
      }
      this->protect.lock = (protect) ? protect->lock : dummy;
      this->protect.unlock = (protect) ? protect->unlock : dummy;
      rtn = &this->interface;
    }
  }
  return rtn;
}

void* xsh_alloc(xsh_heap_t *this, size_t size) {
  void *rtn = NULL;
  xsh_heap_t *heap = check(this);
  if(heap && size == heap->size) {
    heap->protect.lock();
    if (heap && size == heap->size) {
      xsh_node_t *node = heap->lfree;
      if (node) {
        heap->lfree = node->next;
        node->next = NULL;
        rtn = (void*)(node+1);
      }
    }
    heap->protect.unlock();
  }
  return rtn;
}

void xsh_free(xsh_heap_t *this, void *ptr) {
  xsh_heap_t *heap = check(this);
  if (heap && ptr) {
    heap->protect.lock();
    xsh_node_t *node = get_node(heap, ptr);
    if (node) {
      node->next = heap->lfree;
      heap->lfree = node;
    }
    heap->protect.unlock();
  }
}

//==================================================================================================
// Local functions implementation
//==================================================================================================

/**
 * @brief Get the structure heap node object related to the provided pointer.
 * 
 * Structure heap validity must have been checked by caller.
 * The provided pointer @c ptr is valid if the address content is located in the range of the
 * initial configured memory pool and if the difference between the associated node address
 * and the base memory pool address is a modulo of the node size.
 * 
 * @param this The heap to which the provided pointer ptr belongs.
 * @param ptr The allocated structure heap node.
 * @return
 * The function returns valid @c xsh_node_t* related to the provided pointer @c ptr
 * or @c NULL if the provided pointer does not belong to the provided heap.
 */

static xsh_node_t* get_node(xsh_heap_t *heap, void *ptr) {
  xsh_node_t *rtn = NULL;
  intptr_t delta = (intptr_t)ptr - (intptr_t)heap->mem_pool;
  // Must be in the range
  if (delta > 0 && delta < (intptr_t)heap->mem_length) {
    // Realign
    delta -= heap->node_offset;
    // Offset to base must be a modulo of the node size
    if (0 == delta % heap->node_size) {
      rtn = (xsh_node_t*)ptr - 1;
    }
  }
  return rtn;
}


