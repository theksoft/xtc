#include "xmp_xss_g.hpp"
#include <iostream>

#define G_HEAP_SIZE   256

static xtc::xss::Allocator hg;


void* operator new(std::size_t count) {
  std::cout << "Global new called redirecting on global allocator" << std::endl;
  void *p = hg.allocate(count);
  if (!p) {
    throw std::bad_alloc();
  }
  return p;
}

void operator delete(void* ptr) {
  std::cout << "Global delete called redirecting on global allocator" << std::endl;
  hg.free(ptr);
}

void* operator new[](std::size_t count) {
  std::cout << "Global new called redirecting on global allocator" << std::endl;
  void *p = hg.allocate(count);
  if (!p) {
    throw std::bad_alloc();
  }
  return p;
}

void operator delete[](void* ptr) {
  std::cout << "Global delete called redirecting on global allocator" << std::endl;
  hg.free(ptr);
}
