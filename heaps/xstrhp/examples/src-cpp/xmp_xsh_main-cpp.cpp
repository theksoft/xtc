#include "xmp_xsh_myclass.hpp"
#include <iostream>

int main() {
  MyClass* table[MYCLASS_HEAP_SIZE];
  for (int i = 0; i < MYCLASS_HEAP_SIZE; i++) {
    try {
      table[i] = new MyClass();
    } catch(const std::exception& e) {
      std::cerr << "ERROR: Not expecting an exception to be thrown!" << std::endl;
      std::cerr << e.what() << std::endl;
    }
  }

  std:: cout << "Expecting next new() operation to throw a memory exception ..." << std::endl;
  try {
    MyClass *p = new MyClass();
    delete p; // Should not be reached
  } catch(const std::exception& e) {
    std::cout << e.what() << std::endl;
  }

  for(int i = 0; i < MYCLASS_HEAP_SIZE; i++) {
    delete table[i];
  }

  std:: cout << "Expecting next new []() operation to throw a memory exception ..." << std::endl;
  try
  {
    MyClass *tbl = new MyClass[1];
    delete [] tbl; // Should not be reached
  }
  catch(const std::exception& e)
  {
    std::cout << e.what() << std::endl;
  }
  
  return 0;
}