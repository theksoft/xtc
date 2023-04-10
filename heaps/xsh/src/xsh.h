#ifndef _XSH_H_
#define _XSH_H_

#include "xtc_heaps.h"
#include <stdlib.h>

// Structure must be aligned on integer boundary
#define XSH_NODE_SIZE(name)           (XTC_ALIGNED_STRUCT(name) + sizeof(xsh_node_t))
#define XSH_HEAP_LENGTH(name, count)  ((count) * XSH_NODE_SIZE(name))

/* Node for list of free structure areas
    - improve alloc & free speed
    - next act as allocated indicator e.g. NULL when allocated */
typedef struct xsh_node_s {
  struct xsh_node_s *next;
} xsh_node_t;

// Heap description
typedef struct {
  xtc_heap_t interface;           // Common heap interface
  xtc_protect_t protect;          // Protection interface
  void *mem_pool;                 // Initially allocated pool
  size_t mem_length;              // Length of initially allocated pool
  size_t size;                    // Size of managed structure
  size_t node_size, node_offset;  // Size and node offset
  xsh_node_t *lfree;              // List of free nodes
} xsh_heap_t;

xtc_heap_t* xsh_init(xsh_heap_t *this, void *mem, size_t length, size_t size, xtc_protect_t *protect);
void* xsh_alloc(xsh_heap_t *this, size_t size);
void xsh_free(xsh_heap_t *this, void *ptr);

#endif // _XSH_H_
