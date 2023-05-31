#ifndef _XTC_GNRHP_HXX_
#define _XTC_GNRHP_HXX_

#include "xtc_gnrhp.h"

namespace xtc {

  class Allocator {
  public:
    Allocator() {}
    virtual ~Allocator() {}
    virtual void* allocate(size_t size) = 0;
    virtual void free(void* ptr) = 0;
    virtual size_t count() = 0;
  };

}

#endif
