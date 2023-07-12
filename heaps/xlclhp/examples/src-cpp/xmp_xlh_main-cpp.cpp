#include "xmp_xlh_a.hpp"
#include "xmp_xlh_b.hpp"
#include "xmp_xlh_g.hpp"
#include <iostream>
#include <cassert>

static void useA();
static void useB();
static void useC();
static void useD();

int main() {
  std::cout << std::endl;
  std::cout << "-------------------------------------------------------------------------------" << std::endl;
  std::cout << "Direct class usage" << std::endl;
  std::cout << "-------------------------------------------------------------------------------" << std::endl;
  std::cout << std::endl;
  useA();
  std::cout << std::endl;
  std::cout << "-------------------------------------------------------------------------------" << std::endl;
  std::cout << "Allocator usage on class" << std::endl;
  std::cout << "-------------------------------------------------------------------------------" << std::endl;
  std::cout << std::endl;
  useB();  
  std::cout << std::endl;
  std::cout << "-------------------------------------------------------------------------------" << std::endl;
  std::cout << "Allocator usage on structure" << std::endl;
  std::cout << "-------------------------------------------------------------------------------" << std::endl;
  std::cout << std::endl;
  useC();  
  std::cout << std::endl;
  std::cout << "-------------------------------------------------------------------------------" << std::endl;
  std::cout << "Allocator usage for default allocation" << std::endl;
  std::cout << "-------------------------------------------------------------------------------" << std::endl;
  std::cout << std::endl;
  useD();  
  std::cout << std::endl << "Done!" << std::endl;
  return 0;
}

static void useA() {

  A* table[30];
  int i;
  for (i = 0; i < 30; i++) {
    try {
      table[i] = new A();
    } catch(const std::exception& e) {
      std::cerr << "ERROR: [iteration:" << i+1 << "] Specific heap for A is exhausted!" << std::endl;
      std::cerr << e.what() << std::endl;
      break;
    }
  }

  int n = i;
  for(i = 0; i < n; i++) {
    delete table[i];
  }

  A* tbl = NULL;
  try {
    tbl = new A[3];
  } catch(const std::exception& e) {
    std::cerr << "ERROR: Not expecting specific heap for A to be exhausted!" << std::endl;
    std::cout << e.what() << std::endl;
  }
  if (tbl) {
    delete [] tbl;
  }
}

static void useB() {

  B* table[30];
  int i;
  for (i = 0; i < 30; i++) {
    try {
      table[i] = new B();
    } catch(const std::exception& e) {
      std::cerr << "ERROR: [iteration:" << i << "] Specific heap for B is exhausted!" << std::endl;
      std::cerr << e.what() << std::endl;
      break;
    }
  }

  int n = i;
  for(i = 0; i < n; i++) {
    delete table[i];
  }

  B* tbl = NULL;
  try {
    tbl = new B[3];
  } catch(const std::exception& e) {
    std::cerr << "ERROR: Not expecting specific heap for B to be exhausted!" << std::endl;
    std::cout << e.what() << std::endl;
  }
  if (tbl) {
    delete [] tbl;
  }
}

#define C_HEAP_SIZE   256

static void useC() {
  typedef struct {
    int A, B, C, D;
  } C;
  static xtc::xlh::Allocator<C_HEAP_SIZE> hc;

  C* table[30];
  int i;
  for (i = 0; i < 30; i++) {
    table[i] = (C*)hc.allocate(sizeof(C));
    if (!table[i]) {
      std::cerr << "ERROR: [iteration:" << i << "] Specific heap for C is exhausted!" << std::endl;
      break;
    }
  }

  int n = i;
  for(i = 0; i < n; i++) {
    hc.free(table[i]);
  }
}

static void useD() {
  typedef struct {
    int a, b, c, d;
  } D;

  D* table[30];
  int i;
  for (i = 0; i < 30; i++) {
    try {
      table[i] = new D;
    } catch(const std::exception& e) {
      std::cerr << "ERROR: [iteration:" << i << "] Global heap is exhausted!" << std::endl;
      std::cout << e.what() << std::endl;
      break;
    }
  }
  
  int n = i;
  for(i = 0; i < n; i++) {
    delete table[i];
  }
}
