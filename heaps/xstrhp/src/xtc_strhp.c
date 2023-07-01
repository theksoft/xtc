#include "xtc_strhp.h"
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


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
#ifdef __DEBUG
static void dump(xsh_heap_t *heap);
#endif

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
    if (!protect || (protect->lock && protect->unlock)) {
      memset(mem, 0, length);
      this->interface.id = get_id();
#ifndef __DEBUG
      this->interface.alloc = (xtc_alloc_t)xsh_alloc;
#else
      this->interface.dump = (xtc_dump_t)xsh_dump;
      this->interface.alloc_dbg = (xtc_alloc_t)xsh_alloc_dbg;
#endif
      this->interface.free = (xtc_free_t)xsh_free;
      this->interface.count = (xtc_count_t)xsh_count;
      this->interface.end = (xtc_end_t)xsh_end;
      this->mem_pool = mem;
      this->mem_length = length;
      this->size = size;
      this->node_size = node_size;
      this->node_offset = sizeof(xsh_node_t);
      this->count = 0;
      // Initialize free list
      {
        this->lfree = (xsh_node_t*)this->mem_pool;
        this->free_count = this->mem_length / this->node_size;
        assert(this->free_count);
        size_t i;
        xsh_node_t *node;
        for (i = 0, node = this->lfree; i < this->free_count - 1; node = node->next, i++) {
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

void* xsh_end(xsh_heap_t *this, size_t *count) {
  void *rtn = NULL;
  xsh_heap_t *heap = check(this);
  size_t tmp = 0, *cnt;
  cnt = (count) ? count : &tmp;
  *cnt = 0;
  if (heap) {
    xtc_protect_t protect = heap->protect;
    protect.lock();
    *cnt = heap->count;
#ifdef __DEBUG
    if (*cnt) {
      dump(heap);
    }
#endif
    rtn = heap->mem_pool;
    memset(heap, 0, sizeof(xsh_heap_t));
    protect.unlock();
    if (*cnt) {
      fprintf(stderr, "WARNING: %ld memory leak(s) detected when ending heap!\n", *cnt);
    }
  }
  return rtn;
}
#ifndef __DEBUG
void* xsh_alloc(xsh_heap_t *this, size_t size)
#else
void* xsh_alloc_dbg(xsh_heap_t *this, size_t size, const char *fn, int line)
#endif
{
  void *rtn = NULL;
  xsh_heap_t *heap = check(this);
  if(heap && size == heap->size) {
    heap->protect.lock();
    xsh_node_t *node = heap->lfree;
    if (node) {
      assert(heap->free_count);
      heap->lfree = node->next;
      node->next = NULL;
      node->allocated = 1;
#ifdef __DEBUG
      node->fn = fn;
      node->line = line;
#endif
      rtn = (void*)(node + 1);
      heap->count++;
      heap->free_count--;
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
    if (node && node->allocated) {
      assert(heap->count);
#ifdef __DEBUG
      node->fn = NULL;
      node->line = 0;
#endif
      node->allocated = 0;
      node->next = heap->lfree;
      heap->lfree = node;
      heap->count--;
      heap->free_count++;
    }
    heap->protect.unlock();
  }
}

size_t xsh_count(xsh_heap_t *this) {
  size_t rtn = 0;
  xsh_heap_t *heap = check(this);
  if (heap) {
    heap->protect.lock();
    rtn = heap->count;
    heap->protect.unlock();
  }
  return rtn;
}

size_t xsh_free_count(xsh_heap_t *this) {
  size_t rtn = 0;
  xsh_heap_t *heap = check(this);
  if (heap) {
    heap->protect.lock();
    rtn = heap->free_count;
    heap->protect.unlock();
  }
  return rtn;
}

#ifdef __DEBUG

void xsh_dump(xsh_heap_t *this) {
  xsh_heap_t *heap = check(this);
  if (heap) {
    heap->protect.lock();
    dump(heap);
    heap->protect.unlock();
  }
}

#endif

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
 * @param heap The heap to which the provided pointer ptr belongs.
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

#ifdef __DEBUG

/**
 * @brief Dump the heap allocated block information.
 * 
 * @param heap Structure heap
 */

static void dump(xsh_heap_t *heap) {
  xsh_node_t *node = (xsh_node_t*)heap->mem_pool;
  printf("HEAP %p[%ld], node size = %ld, allocated count = %ld\n",
    (void*)heap->mem_pool, heap->mem_length, heap->node_size, heap->count);
  size_t i = heap->count, n = i + heap->free_count;
  for (; i!=0 && n!=0; n--, node = (xsh_node_t*)((uint8_t*)node + heap->node_size)) {
    if (node->allocated) {
      i--;
      printf("\t%p[%ld] @ %s (%d)\n",
        (void*)(node + 1), heap->node_size, node->fn, node->line);
    }
  }
}

#endif
