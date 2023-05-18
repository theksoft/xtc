#ifndef _XMP_XSH_A_HPP_
#define _XMP_XSH_A_HPP_

#include <new>

#define A_HEAP_SIZE   5

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
