#include "xmp_xss_a.hpp"
#include "xtc_syshp.h"
#include <iostream>

static xss_heap_t* heap();

A::A() {
  std::cout << "\tHello from A!" << std::endl;
}

A::~A() {
  std::cout << "\tBye from A!" << std::endl;
}

void* A::operator new(std::size_t count) {
  void *p = xss_alloc(heap(), count);
  if (!p) {
    throw std::bad_alloc();
  }
  return p;
}

void A::operator delete(void* ptr) {
  xss_free(heap(), ptr);
}

void* A::operator new[](std::size_t count) {
  std::cout << "Array allocation is supported!" << std::endl;
  void *p = xss_alloc(heap(), count);
  if (!p) {
    throw std::bad_alloc();
  }
  return p;
}

void A::operator delete[](void* ptr) {
  xss_free(heap(), ptr);
}

static xss_heap_t* heap() {
  static int initialized = 0;
  static xss_heap_t the_heap;
  if (!initialized) {
    xss_init(&the_heap, NULL);
    initialized = 1;
  }
  return &the_heap;
}
