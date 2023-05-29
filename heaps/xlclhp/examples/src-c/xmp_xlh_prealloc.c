/*
  This example demonstrates 3 different points:
    - Usage of a preallocated memory buffer for local heap allocation
    - Protected local heap

  We assume the heap lives until the program ends.
  Cleaner implementation should take care of releasing allocated memory
  and destroying protection mutex.
*/

#define XMP_PREALLOCATED_XLH_SIZE     1024*1024

#include "xtc_lclhp.h"
#include "xmp_xlh_prealloc.h"

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
static xlh_heap_t* heap();

// Allocation wrapper
void* xlh_alloc_p(size_t size) {
  return xlh_alloc(heap(), size);
}

// Release wrapper
void xlh_free_p(void *p) {
  xlh_free(heap(), p);
}

// Encapsulated heap
static xlh_heap_t* heap() {

  static int initialized = 0;
  // Dynamically preallocated memory area
  static char* memory;
  // Heap management structure
  static xlh_heap_t the_heap;

  xlh_heap_t *rtn = &the_heap;

  if (!initialized) {
    // Memory preallocation
    memory = (char*)malloc(XMP_PREALLOCATED_XLH_SIZE);
    if (memory) {
      // Structure heap initialization
      xlh_init(&the_heap, memory, XMP_PREALLOCATED_XLH_SIZE, GUARDS);
      initialized = 1;
    } else {
      rtn = NULL;
    }
  }

  return rtn;
}
