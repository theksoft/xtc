#ifndef _XTC_GNRHP_H_
#define _XTC_GNRHP_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Precomputed at compilation time
#define XTC_INT_MOD(size)             ((size) % sizeof(int))
#define XTC_INT_MOD_COMPLEMENT(size)  (XTC_INT_MOD(size) ? sizeof(int) - XTC_INT_MOD(size) : 0)
#define XTC_ALIGNED_SIZE(size)        ((size) + XTC_INT_MOD_COMPLEMENT(size))
#define XTC_ALIGNED_STRUCT(name)      XTC_ALIGNED_SIZE(sizeof(name))

typedef unsigned int xtc_heaps_id_t;

typedef struct xtc_heap_s* xtc_heap_ptr_t;
#ifdef __DEBUG
  typedef void (*xtc_dump_t)(xtc_heap_ptr_t heap);
  typedef void* (*xtc_alloc_t)(xtc_heap_ptr_t heap, size_t size, const char *fn, int line);
#else
  typedef void* (*xtc_alloc_t)(xtc_heap_ptr_t heap, size_t size);
#endif
typedef void (*xtc_free_t)(xtc_heap_ptr_t heap, void *ptr);
typedef size_t(*xtc_count_t)(xtc_heap_ptr_t heap);
typedef void* (*xtc_end_t)(xtc_heap_ptr_t heap, size_t *count);

// Common heaps interface
typedef struct xtc_heap_s {
  xtc_heaps_id_t id;
#ifdef __DEBUG
  xtc_dump_t dump;
  xtc_alloc_t alloc_dbg;
#else
  xtc_alloc_t alloc;
#endif
  xtc_free_t free;
  xtc_count_t count;
  xtc_end_t end;
} xtc_heap_t;

#ifdef __DEBUG
  #define alloc(heap, size)       alloc_dbg(heap, size, __FILE__, __LINE__)
  #define xtc_alloc(heap, size)   xtc_alloc_dbg(heap, size, __FILE__, __LINE__)
  void xtc_dump(xtc_heap_t *heap);
  void* xtc_alloc_dbg(xtc_heap_t *heap, size_t size, const char *fn, int line);
#else
  void* xtc_alloc(xtc_heap_t *heap, size_t size);
#endif
void xtc_free(xtc_heap_t *heap, void *ptr);
size_t xtc_count(xtc_heap_t *heap);
void* xtc_end(xtc_heap_t *heap, size_t *count);

// Protection interface for thread safety if required
typedef struct {
  void (*lock)();
  void (*unlock)();
} xtc_protect_t;

#ifdef __cplusplus
}
#endif

#endif // _XTC_GNRHP_H_
