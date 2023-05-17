#ifndef _XMP_XSH_MYCLASS_HPP_
#define _XMP_XSH_MYCLASS_HPP_

#include <new>

#define MYCLASS_HEAP_SIZE   10

class MyClass {
public:
  MyClass();
  ~MyClass();

  void* operator new(std::size_t count);
  void operator delete(void* ptr);
  void* operator new[](std::size_t count);
  void operator delete[](void* ptr);
};

#endif