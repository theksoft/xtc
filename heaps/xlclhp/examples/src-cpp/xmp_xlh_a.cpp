#include "xmp_xlh_a.hpp"
#include "xtc_lclhp.h"
#include <iostream>

static xlh_heap_t* heap();

A::A() {
  std::cout << "\tHello from A!" << std::endl;
}

A::~A() {
  std::cout << "\tBye from A!" << std::endl;
}

void* A::operator new(std::size_t count) {
  void *p = xlh_alloc(heap(), count);
  if (!p) {
    throw std::bad_alloc();
  }
  return p;
}

void A::operator delete(void* ptr) {
  xlh_free(heap(), ptr);
}

void* A::operator new[](std::size_t count) {
  std::cout << "Array allocation is supported!" << std::endl;
  void *p = xlh_alloc(heap(), count);
  if (!p) {
    throw std::bad_alloc();
  }
  return p;
}

void A::operator delete[](void* ptr) {
  xlh_free(heap(), ptr);
}

static xlh_heap_t* heap() {
  static int initialized = 0;
  static char memory[A_HEAP_SIZE];
  static xlh_heap_t the_heap;
  if (!initialized) {
    xlh_init(&the_heap, memory, A_HEAP_SIZE, NULL);
    initialized = 1;
  }
  return &the_heap;
}
