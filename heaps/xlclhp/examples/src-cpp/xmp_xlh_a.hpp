#ifndef _XMP_XLH_A_HPP_
#define _XMP_XLH_A_HPP_

#include <new>

#define A_HEAP_SIZE   512

class A {
public:
  A();
  ~A();

  void* operator new(std::size_t count);
  void operator delete(void* ptr);
  void* operator new[](std::size_t count);
  void operator delete[](void* ptr);
};

#endif
