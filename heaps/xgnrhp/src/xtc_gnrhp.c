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
