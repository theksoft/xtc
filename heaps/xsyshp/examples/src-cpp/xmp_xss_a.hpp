#ifndef _XMP_XSS_A_HPP_
#define _XMP_XSS_A_HPP_

#include <new>

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
