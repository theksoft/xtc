#ifndef _XMP_XSS_B_HPP_
#define _XMP_XSS_B_HPP_

#include <new>
#include "xtc_syshp.hxx"

class B {
public:
  B();
  ~B();

  void* operator new(std::size_t count);
  void operator delete(void* ptr);
  void* operator new[](std::size_t count);
  void operator delete[](void* ptr);

private:
  static xtc::xss::Allocator _heap;
};

#endif
