#ifndef _XTC_SYSHP_H_
#define _XTC_SYSHP_H_

#include "xtc_gnrhp.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xss_node_s {
  uintptr_t heap;
  size_t size;
#ifdef __DEBUG
  struct xss_node_s *next, *previous;
  const char *fn;
  int line;
#endif
} xss_node_t;

typedef struct {
  xtc_heap_t interface;
  xtc_protect_t protect;
  size_t count;
  size_t total_size;
#ifdef __DEBUG
  xss_node_t *head;
#endif
} xss_heap_t;

xtc_heap_t* xss_init(xss_heap_t *heap, xtc_protect_t *protect);

void* xss_end(xss_heap_t *heap, size_t *count);

#ifndef __DEBUG

void* xss_alloc(xss_heap_t *heap, size_t size);

#else // __DEBUG

void* xss_alloc_dbg(xss_heap_t *heap, size_t size, const char *fn, int line);
#define xss_alloc(heap, size)     xss_alloc_dbg(heap, size, __FILE__, __LINE__)

#endif  // __DEBUG

void xss_free(xss_heap_t *heap, void *ptr);

size_t xss_count(xss_heap_t *heap);

size_t xss_total_size(xss_heap_t *heap);

#ifdef __DEBUG
void xss_dump(xss_heap_t *heap);
#endif  // __DEBUG

#ifdef __cplusplus
}
#endif

#endif // _XTC_SYSHP_H_
