/*
  This example demonstrates 3 different points:
    - Usage of a static memory buffer for local heap allocation
    - Unprotected local heap
  We assume the heap lives until the program ends.
  Cleaner implementation should take care of ending heap with xlh_end().
*/

#define XMP_XLH_STATIC_SIZE     (1024*1024)

#include "xtc_lclhp.h"
#include "xmp_xlh_static.h"

// Encapsulated heap declaration
static xlh_heap_t* heap();

// Allocation wrapper
void* xlh_alloc_s(size_t size) {
  return xlh_alloc(heap(), size);
}

// Release wrapper
void xlh_free_s(void *p) {
  xlh_free(heap(), p);
}

// Encapsulated heap
static xlh_heap_t* heap() {

  static int initialized = 0;
  // Static memory area
  static char memory[XMP_XLH_STATIC_SIZE];
  // Heap management structure
  static xlh_heap_t the_heap;

  if (!initialized) {
    // Structure heap initialization
    xlh_init(&the_heap, memory, XMP_XLH_STATIC_SIZE, NULL); // -> unprotected
    initialized = 1;
  }
  return &the_heap;
}
