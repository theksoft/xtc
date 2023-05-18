#ifndef _XTC_STRHP_HXX_
#define _XTC_STRHP_HXX_

#include "xtc_strhp.h"
#include "xtc_gnrhp.hxx"
#include <cstring>

namespace xtc {
  namespace xsh {

  template<class T, int Count>
  class Allocator : public xtc::Allocator {
  protected:

    char _memory[XSH_HEAP_LENGTH(T, Count)];
    xsh_heap_t _heap;

  public:

    Allocator(xtc_protect_t *protect = NULL) {
      memset(_memory, 0, XSH_HEAP_LENGTH(T, Count));
      xsh_init(&_heap, _memory, XSH_HEAP_LENGTH(T, Count), sizeof(T), protect);
    }

    virtual ~Allocator() {
      memset(_memory, 0, XSH_HEAP_LENGTH(T, Count));
      memset(&_heap, 0, sizeof(xsh_heap_t));
    }

    virtual void* allocate(size_t size) {
      return (T*)xsh_alloc(&_heap, size);
    }

    virtual void free(void *ptr) {
      xsh_free(&_heap, ptr);
    }

    T* allocate() {
      return (T*)allocate(sizeof(T));
    }

    void free(T* ptr) {
      free((void*)ptr);
    }

    size_t freeCount() {
      return xsh_free_count(&_heap);
    }
  };

}}

#endif
