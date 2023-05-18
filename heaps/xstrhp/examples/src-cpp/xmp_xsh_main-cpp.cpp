#include "xmp_xsh_a.hpp"
#include "xmp_xsh_b.hpp"
#include <iostream>
#include <cassert>

void useA();
void useB();
void useC();

int main() {
  std::cout << std::endl << "Direct class usage" << std::endl;
  useA();
  std::cout << std::endl << "Allocator usage on class" << std::endl;
  useB();  
  std::cout << std::endl << "Allocator usage on structure" << std::endl;
  useC();  
  std::cout << std::endl << "Done!" << std::endl;
  return 0;
}

void useA() {

  A* table[A_HEAP_SIZE];
  for (int i = 0; i < A_HEAP_SIZE; i++) {
    try {
      table[i] = new A();
    } catch(const std::exception& e) {
      std::cerr << "ERROR: Not expecting an exception to be thrown!" << std::endl;
      std::cerr << e.what() << std::endl;
    }
  }

  std:: cout << "Expecting next new() operation to throw a memory exception ..." << std::endl;
  try {
    A *p = new A();
    delete p; // Should not be reached
  } catch(const std::exception& e) {
    std::cout << e.what() << std::endl;
  }

  for(int i = 0; i < A_HEAP_SIZE; i++) {
    delete table[i];
  }

  std:: cout << "Expecting next new []() operation to throw a memory exception ..." << std::endl;
  try
  {
    A *tbl = new A[1];
    delete [] tbl; // Should not be reached
  }
  catch(const std::exception& e)
  {
    std::cout << e.what() << std::endl;
  }
}

void useB() {

  B* table[B_HEAP_SIZE];
  for (int i = 0; i < B_HEAP_SIZE; i++) {
    try {
      table[i] = new B();
    } catch(const std::exception& e) {
      std::cerr << "ERROR: Not expecting an exception to be thrown!" << std::endl;
      std::cerr << e.what() << std::endl;
    }
  }

  std:: cout << "Expecting next new() operation to throw a memory exception ..." << std::endl;
  try {
    B *p = new B();
    delete p; // Should not be reached
  } catch(const std::exception& e) {
    std::cout << e.what() << std::endl;
  }

  for(int i = 0; i < B_HEAP_SIZE; i++) {
    delete table[i];
  }

  std:: cout << "Expecting next new []() operation to throw a memory exception ..." << std::endl;
  try
  {
    B *tbl = new B[1];
    delete [] tbl; // Should not be reached
  }
  catch(const std::exception& e)
  {
    std::cout << e.what() << std::endl;
  }
}

#define C_HEAP_SIZE   10

void useC() {
  typedef struct {
    int A, B, C, D;
  } C;
  static xtc::xsh::Allocator<C,C_HEAP_SIZE> hc;

  C* table[C_HEAP_SIZE];
  for (int i = 0; i < C_HEAP_SIZE; i++) {
    table[i] = hc.allocate();
    assert(table[i]);
  }

  C *p = hc.allocate();
  assert(!p);
  (void)p;

  for(int i = 0; i < C_HEAP_SIZE; i++) {
    hc.free(table[i]);
  }
  assert(hc.freeCount() == C_HEAP_SIZE);
}

