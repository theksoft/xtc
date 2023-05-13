/*
  This example demonstrates 3 different points:
    - Usage of a static memory buffer for structure heap allocation
    - Unprotected structure heap
*/

#define XMP_STATIC_XSH_SIZE     30

#include "xtc_strhp.h"
#include "xmp_static_xsh.h"

// Encapsulated heap declaration
static xsh_heap_t* heap();

// Allocation wrapper
my_struct* xsh_alloc_s() {
  return (my_struct*)xsh_alloc(heap(), sizeof(my_struct));
}

// Release wrapper
void xsh_free_s(my_struct *p) {
  xsh_free(heap(), (void*)p);
}

// Encapsulated heap
static xsh_heap_t* heap() {

  static int initialized = 0;
  // Static memory area
  static char memory[XSH_HEAP_LENGTH(my_struct, XMP_STATIC_XSH_SIZE)];
  // Heap management structure
  static xsh_heap_t the_heap;

  if (!initialized) {
    // Structure heap initialization
    xsh_init(
      &the_heap, memory,
      XSH_HEAP_LENGTH(my_struct, XMP_STATIC_XSH_SIZE),
      sizeof(my_struct), NULL // -> unprotected
    );
    initialized = 1;
  }
  return &the_heap;
}
