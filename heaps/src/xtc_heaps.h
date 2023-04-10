#ifndef _XTC_HEAPS_H_
#define _XTC_HEAPS_H_

#include <stdlib.h>

// Precomputed at compilation time
#define XTC_INT_MOD(size)             ((size) % sizeof(int))
#define XTC_INT_MOD_COMPLEMENT(size)  (XTC_INT_MOD(size) ? sizeof(int) - XTC_INT_MOD(size) : 0)
#define XTC_ALIGNED_SIZE(size)        ((size) + XTC_INT_MOD_COMPLEMENT(size))
#define XTC_ALIGNED_STRUCT(name)      XTC_ALIGNED_SIZE(sizeof(name))

typedef unsigned int xtc_heaps_id_t;

typedef struct xtc_heap_s* xtc_heap_ptr_t;
typedef void* (*xtc_alloc_t)(xtc_heap_ptr_t heap, size_t size);
typedef void (*xtc_free_t)(xtc_heap_ptr_t heap, void *ptr);

// Common heaps interface
typedef struct xtc_heap_s {
  xtc_heaps_id_t id;
  xtc_alloc_t alloc;
  xtc_free_t free;
} xtc_heap_t;

void* xtc_alloc(xtc_heap_t *heap, size_t size);
void xtc_free(xtc_heap_t *heap, void *ptr);

// System heap in the XTC form
xtc_heap_t* sys_heap();

// Protection interface for thread safety if required
typedef struct {
  void (*lock)();
  void (*unlock)();
} xtc_protect_t;

#endif // _XTC_HEAPS_H_
