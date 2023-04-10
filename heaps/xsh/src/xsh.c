#include "xsh.h"
#include <stdint.h>
#include <assert.h>

static xtc_heaps_id_t get_id() {
  static unsigned int ref = 0x0FEEDBABE;
  return (xtc_heaps_id_t)(uintptr_t)&ref;
}

static inline void check_node_size() {
#ifdef __DEBUG
  xsh_node_t n1;
  xsh_node_t n2;
  // Check possible integer size issue
  assert(sizeof(xsh_node_t) == (uintptr_t)&n2 - (uintptr_t)&n1);
#endif
}

static inline xsh_heap_t* check(xsh_heap_t *this) {
  return this && this->interface.id == get_id() ? this : NULL;
}

static xsh_node_t* get_node(xsh_heap_t *this, void *ptr);
static void dummy() {}

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


