#ifndef _XMP_XSH_B_HPP_
#define _XMP_XSH_B_HPP_

#include <new>
#include "xtc_strhp.hxx"

#define B_HEAP_SIZE   7

class B {
public:
  B();
  ~B();

  void* operator new(std::size_t count);
  void operator delete(void* ptr);
  void* operator new[](std::size_t count);
  void operator delete[](void* ptr);

private:
  static xtc::xsh::Allocator<B, B_HEAP_SIZE> _heap;
};

#endif
