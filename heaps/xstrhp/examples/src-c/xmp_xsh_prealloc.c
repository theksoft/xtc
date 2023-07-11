/*
  This example demonstrates 3 different points:
    - Usage of a preallocated memory buffer for structure heap allocation
    - Protected structure heap

  We assume the heap lives until the program ends.
  Cleaner implementation should take care of ending heap with xsh_end()
  and destroying associated protection mutex.
*/

#define XMP_PREALLOCATED_XSH_SIZE     30

#include "xtc_strhp.h"
#include "xmp_xsh_prealloc.h"

// Protection interface declaration with C11 built-in threading support
// -----------------------------------------------------------------------------

#ifdef __STDC_NO_THREADS__

// Finally unprotected if no builin support
// Consider implmenting on Posix threads or anything alike
#define GUARDS  NULL
#warning No C11 threading builtin support

#else

#include <threads.h>

// Encapsulated lock
static mtx_t* guard() {
  static mtx_t lock;
  static int initialized = 0;
  if (!initialized) {
    mtx_init(&lock, mtx_plain|mtx_recursive);
    initialized = 1;
  }
  return &lock;
}

static void lock()    { mtx_lock(guard());    }
static void unlock()  { mtx_unlock(guard());  }
static xtc_protect_t guards = { .lock = lock, .unlock = unlock };
#define GUARDS  &guards

#endif

// Heap implementation
// -----------------------------------------------------------------------------

// Encapsulated heap declaration
static xsh_heap_t* heap();

// Allocation wrapper
my_struct* xsh_alloc_p() {
  return (my_struct*)xsh_alloc(heap(), sizeof(my_struct));
}

// Release wrapper
void xsh_free_p(my_struct *p) {
  xsh_free(heap(), (void*)p);
}

// Encapsulated heap
static xsh_heap_t* heap() {

  static int initialized = 0;
  // Dynamically preallocated memory area
  static char* memory;
  // Heap management structure
  static xsh_heap_t the_heap;

  xsh_heap_t *rtn = &the_heap;

  if (!initialized) {
    // Memory preallocation
    memory = (char*)malloc(XSH_HEAP_LENGTH(my_struct, XMP_PREALLOCATED_XSH_SIZE));
    if (memory) {
      // Structure heap initialization
      xsh_init(
        &the_heap, memory,
        XSH_HEAP_LENGTH(my_struct, XMP_PREALLOCATED_XSH_SIZE),
        sizeof(my_struct), GUARDS
      );
      initialized = 1;
    } else {
      rtn = NULL;
    }
  }

  return rtn;
}
