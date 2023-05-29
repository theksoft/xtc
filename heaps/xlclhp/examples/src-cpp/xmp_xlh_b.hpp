#ifndef _XMP_XLH_B_HPP_
#define _XMP_XLH_B_HPP_

#include <new>
#include "xtc_lclhp.hxx"

#define B_HEAP_SIZE   512

class B {
public:
  B();
  ~B();

  void* operator new(std::size_t count);
  void operator delete(void* ptr);
  void* operator new[](std::size_t count);
  void operator delete[](void* ptr);

private:
  static xtc::xlh::Allocator<B_HEAP_SIZE> _heap;
};

#endif
