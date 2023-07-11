#include "xmp_xsh_a.hpp"
#include "xtc_strhp.h"
#include <iostream>

static xsh_heap_t* heap();

A::A() {
  std::cout << "\tHello from A!" << std::endl;
}

A::~A() {
  std::cout << "\tBye from A!" << std::endl;
}

void* A::operator new(std::size_t count) {
  void *p = xsh_alloc(heap(), count);
  if (!p) {
    throw std::bad_alloc();
  }
  return p;
}

void A::operator delete(void* ptr) {
  xsh_free(heap(), ptr);
}

void* A::operator new[](std::size_t count) {
  (void)count;
  std::cout << "Array allocation not supported!" << std::endl;
  throw std::bad_alloc();
}

void A::operator delete[](void* ptr) {
  (void)ptr;
  throw std::bad_exception();
}

static xsh_heap_t* heap() {
  static int initialized = 0;
  static char memory[XSH_HEAP_LENGTH(A, A_HEAP_SIZE)];
  static xsh_heap_t the_heap;
  if (!initialized) {
    xsh_init(
      &the_heap, memory,
      XSH_HEAP_LENGTH(A, A_HEAP_SIZE),
      sizeof(A), NULL
    );
    initialized = 1;
  }
  return &the_heap;
}
