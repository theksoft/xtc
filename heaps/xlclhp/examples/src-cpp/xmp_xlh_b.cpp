#include "xmp_xlh_b.hpp"
#include "xtc_lclhp.h"
#include <iostream>

xtc::xlh::Allocator<B_HEAP_SIZE> B::_heap;

B::B() {
  std::cout << "\tHello from B!" << std::endl;
}

B::~B() {
  std::cout << "\tBye from B!" << std::endl;
}

void* B::operator new(std::size_t count) {
  void *p = _heap.allocate(count);
  if (!p) {
    throw std::bad_alloc();
  }
  return p;
}

void B::operator delete(void* ptr) {
  _heap.free(ptr);
}

void* B::operator new[](std::size_t count) {
  std::cout << "Array allocation is supported!" << std::endl;
  void *p = _heap.allocate(count);
  if (!p) {
    throw std::bad_alloc();
  }
  return p;
}

void B::operator delete[](void* ptr) {
  _heap.free(ptr);
}
