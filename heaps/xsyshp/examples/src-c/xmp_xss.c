/*
  This example demonstrates 3 different points:
    - Usage of a static memory buffer for local heap allocation
    - Unprotected local heap
  We assume the heap lives until the program ends.
  Cleaner implementation should take care of ending heap with xlh_end().
*/

#include "xtc_syshp.h"
#include "xmp_xss.h"

// Encapsulated heap declaration
static xss_heap_t* heap();

// Allocation wrapper
void* sys_alloc(size_t size) {
  return xss_alloc(heap(), size);
}

// Release wrapper
void sys_free(void *p) {
  xss_free(heap(), p);
}

// Encapsulated heap
static xss_heap_t* heap() {

  static int initialized = 0;
  // Heap management structure
  static xss_heap_t the_heap;

  if (!initialized) {
    // Structure heap initialization
    xss_init(&the_heap, NULL); // -> unprotected
    initialized = 1;
  }
  return &the_heap;
}
