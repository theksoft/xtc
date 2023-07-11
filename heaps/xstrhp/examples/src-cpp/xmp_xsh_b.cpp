#include "xmp_xsh_b.hpp"
#include "xtc_strhp.h"
#include <iostream>

xtc::xsh::Allocator<B, B_HEAP_SIZE> B::_heap;

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
  (void)count;
  std::cout << "Array allocation not supported!" << std::endl;
  throw std::bad_alloc();
}

void B::operator delete[](void* ptr) {
  (void)ptr;
  throw std::bad_exception();
}
