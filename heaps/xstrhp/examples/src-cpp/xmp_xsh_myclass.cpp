#include "xmp_xsh_myclass.hpp"
#include "xtc_strhp.h"
#include <iostream>

static xsh_heap_t* heap();

MyClass::MyClass() {
  std::cout << "\tHello from MyClass!" << std::endl;
}

MyClass::~MyClass() {
  std::cout << "\tBye from MyClass!" << std::endl;
}

void* MyClass::operator new(std::size_t count) {
  void *p = xsh_alloc(heap(), count);
  if (!p) {
    throw std::bad_alloc();
  }
  return p;
}

void MyClass::operator delete(void* ptr) {
  xsh_free(heap(), ptr);
}

void* MyClass::operator new[](std::size_t count) {
  (void)count;
  std::cout << "\tArray allocation not supported!" << std::endl;
  throw std::bad_alloc();
}

void MyClass::operator delete[](void* ptr) {
  (void)ptr;
  throw std::bad_exception();
}

static xsh_heap_t* heap() {
  static int initialized = 0;
  static char memory[XSH_HEAP_LENGTH(MyClass, MYCLASS_HEAP_SIZE)];
  static xsh_heap_t the_heap;
  if (!initialized) {
    xsh_init(
      &the_heap, memory,
      XSH_HEAP_LENGTH(MyClass, MYCLASS_HEAP_SIZE),
      sizeof(MyClass), NULL
    );
    initialized = 1;
  }
  return &the_heap;
}
