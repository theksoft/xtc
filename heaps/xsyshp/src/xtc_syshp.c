#include "xtc_syshp.h"
#include <stdint.h>

/*
  System heap in the XTC form.
*/

static xtc_heaps_id_t get_id() {
  static unsigned int ref = 0x0FEEDBABE;
  return (xtc_heaps_id_t)(uintptr_t)&ref;
}

static inline xtc_heap_t* check(xtc_heap_t *this) {
  return this && this->id == get_id() ? this : NULL;
}

#ifndef __DEBUG

static void* sys_alloc(xtc_heap_t *this, size_t size) {
  void *rtn = NULL;
  xtc_heap_t *heap = check(this);
  if (heap) {
    rtn = malloc(size);
  }
  return rtn;
}

#endif

static void sys_free(xtc_heap_t *this, void *ptr) {
  xtc_heap_t *heap = check(this);
  if (heap) {
    free(ptr);
  }
}

xtc_heap_t* sys_heap() {
  static int initialized = 0;
  static xtc_heap_t heap = {
    .id = 0,
  #ifndef __DEBUG
    .alloc = sys_alloc,
  #else
    .alloc_dbg = NULL,
  #endif
    .free = sys_free
  };
  if (!initialized) {
    heap.id = get_id();
    initialized = 1;
  }
  return &heap;
}
