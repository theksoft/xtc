#include "xtc_gnrhp.h"
#include <stdint.h>

void* xtc_alloc(xtc_heap_t *this, size_t size) {
  return (this) ? this->alloc(this, size) : NULL;
}

void xtc_free(xtc_heap_t *this, void *ptr) {
  if (this) {
    this->free(this, ptr);
  }
}

size_t xtc_count(xtc_heap_t *this) {
  return (this) ? this->count(this) : (size_t)-1;
}

void* xtc_end(xtc_heap_t *this, size_t* count) {
  void *rtn = NULL;
  if (this) {
    rtn = this->end(this, count);
  }
  return rtn;
}
