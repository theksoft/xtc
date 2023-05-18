#include <CUnit/CUnit.h>
#include <cassert>
#include "cxx_tests.h"
#include "xtc_strhp.hxx"

class TestClass {
public:
  TestClass() {}
  virtual ~TestClass() {}
private:
  int a, b, c, d;
};

#define CLASS_HEAP_SIZE     12
static xtc::xsh::Allocator<TestClass, CLASS_HEAP_SIZE> *clsalloc;

extern "C" int cxx_init_allocator() {
  clsalloc = new xtc::xsh::Allocator<TestClass, CLASS_HEAP_SIZE>();
  return 0;
}

extern "C" int cxx_cleanup_allocator() {
  delete clsalloc;
  return 0;
}

extern "C" void cxx_alloc_0_size() {
  size_t count = clsalloc->freeCount();
  CU_ASSERT_PTR_NULL(clsalloc->allocate(0));
  CU_ASSERT_EQUAL(clsalloc->freeCount(), count);
}

extern "C" void cxx_alloc_bad_size() {
  size_t count = clsalloc->freeCount();
  CU_ASSERT_PTR_NULL(clsalloc->allocate(sizeof(TestClass) - 1));
  CU_ASSERT_EQUAL(clsalloc->freeCount(), count);
  CU_ASSERT_PTR_NULL(clsalloc->allocate(sizeof(TestClass) + 1));
  CU_ASSERT_EQUAL(clsalloc->freeCount(), count);
}

extern "C" void cxx_simple_alloc_free() {
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE);
  void *ptr = clsalloc->allocate(sizeof(TestClass));
  CU_ASSERT_PTR_NOT_NULL(ptr);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE - 1);
  TestClass *pcls = clsalloc->allocate();
  CU_ASSERT_PTR_NOT_NULL(pcls);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE - 2);
  clsalloc->free(ptr);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE - 1);
  clsalloc->free(pcls);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE);
}

extern "C" void cxx_sequential_alloc_free() {
  void* arr[CLASS_HEAP_SIZE];
  memset(arr, 0, sizeof(arr));
  int i;
  // Allocate all sequentially
  for (i = 0; i < CLASS_HEAP_SIZE; i++) {
    CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE - i);
    arr[i] = (i%2) ? clsalloc->allocate(sizeof(TestClass)) : clsalloc->allocate();
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  // Check there is no more space
  CU_ASSERT_EQUAL(clsalloc->freeCount(), 0);
  CU_ASSERT_PTR_NULL(clsalloc->allocate(sizeof(TestClass)));
  CU_ASSERT_EQUAL(clsalloc->freeCount(), 0);
  CU_ASSERT_PTR_NULL(clsalloc->allocate());
  CU_ASSERT_EQUAL(clsalloc->freeCount(), 0);
  // Free all blocks
  for (i = 0; i < CLASS_HEAP_SIZE; i++) {
    CU_ASSERT_EQUAL(clsalloc->freeCount(), i);
    clsalloc->free(arr[i]);
  }
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE);
}

extern "C" void cxx_unordered_alloc_free() {
  void* arr[CLASS_HEAP_SIZE];
  memset(arr, 0, sizeof(arr));
  int i;
  // Alloc all
  for (i = 0; i < CLASS_HEAP_SIZE; i++) {
    CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE - i);
    arr[i] = clsalloc->allocate(sizeof(TestClass));
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  // Free 1 over 2
  size_t count = clsalloc->freeCount();
  CU_ASSERT_EQUAL(count, 0);
  for (i = 0; i < CLASS_HEAP_SIZE; i+=2) {
    CU_ASSERT_EQUAL(clsalloc->freeCount(), count++);
    clsalloc->free(arr[i]);
    arr[i] = NULL;
  }
  // Alloc again half of the previously freed
  for (i = 0; i < CLASS_HEAP_SIZE; i+=4) {
    assert(!arr[i]);
    CU_ASSERT_EQUAL(clsalloc->freeCount(), count--);
    arr[i] = clsalloc->allocate();
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  for (i = 0; count; ) {
    while(arr[i]) i++;
    assert(i < CLASS_HEAP_SIZE);
    CU_ASSERT_EQUAL(clsalloc->freeCount(), count--);
    arr[i] = clsalloc->allocate(sizeof(TestClass));
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  CU_ASSERT_EQUAL(clsalloc->freeCount(), 0);
  CU_ASSERT_PTR_NULL(clsalloc->allocate());
  for (i = 0; i < CLASS_HEAP_SIZE; i++) {
    if (!arr[i]) continue;
    count++;
  }
  CU_ASSERT_EQUAL(count, CLASS_HEAP_SIZE);
  for (i = 0; i < CLASS_HEAP_SIZE; i++) {
    CU_ASSERT_EQUAL(clsalloc->freeCount(), i);
    clsalloc->free((TestClass*)arr[i]);
    arr[i] = NULL;
  }
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE);
}

extern "C" void cxx_free_null() {
  void* ptr = clsalloc->allocate();
  int count = clsalloc->freeCount();
  clsalloc->free((void*)NULL);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), count);
  clsalloc->free(ptr);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), ++count);
}

extern "C" void cxx_free_invalid() {
  int bullshit;
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE);
  clsalloc->free(&bullshit);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE);
  void* ptr = clsalloc->allocate(sizeof(TestClass));
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE - 1);
  clsalloc->free((void*)((char*)ptr - 1));
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE - 1);
  clsalloc->free((void*)((char*)ptr + 1));
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE - 1);
  clsalloc->free(ptr);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE);
}

extern "C" void cxx_free_double() {
  cxx_cleanup_allocator(); cxx_init_allocator();
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE);
  TestClass* ptr = clsalloc->allocate();
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE - 1);
  clsalloc->free(ptr);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE);
  clsalloc->free(ptr);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE);
}

extern "C" void cxx_free_not_allocated() {
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE);
  char buf[sizeof(TestClass)];
  clsalloc->free((void*)buf);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE);
  // Fake node cannot be built - class inner
}

extern "C" void cxx_free_wrong() {
  // Declare a second heap on same structure - on stack
  xtc::xsh::Allocator<TestClass, CLASS_HEAP_SIZE> heap2;
  CU_ASSERT_EQUAL(heap2.freeCount(), CLASS_HEAP_SIZE);

  void* ptr1 = clsalloc->allocate();
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE - 1);
  void* ptr2 = heap2.allocate();
  CU_ASSERT_EQUAL(heap2.freeCount(), CLASS_HEAP_SIZE - 1);

  heap2.free(ptr1);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE - 1);
  CU_ASSERT_EQUAL(heap2.freeCount(), CLASS_HEAP_SIZE - 1);
  clsalloc->free(ptr2);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE - 1);
  CU_ASSERT_EQUAL(heap2.freeCount(), CLASS_HEAP_SIZE - 1);

  heap2.free(ptr2);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE - 1);
  CU_ASSERT_EQUAL(heap2.freeCount(), CLASS_HEAP_SIZE);
  clsalloc->free(ptr1);
  CU_ASSERT_EQUAL(clsalloc->freeCount(), CLASS_HEAP_SIZE);
  CU_ASSERT_EQUAL(heap2.freeCount(), CLASS_HEAP_SIZE);
}

static int lock_called = 0;
static int unlock_called = 0;

static void reset_protect() {
  lock_called = 0;
  unlock_called = 0;
}

static void lock() {
  lock_called++;
}

static void unlock() {
  unlock_called++;
}

typedef struct {
  int a, b, c, d;
} test_struct;

#define STRUCT_HEAP_SIZE     42
static xtc::xsh::Allocator<test_struct, STRUCT_HEAP_SIZE> *stralloc;

extern "C" int cxx_init_protected_allocator() {
  xtc_protect_t protect = { lock, unlock };
  reset_protect();
  stralloc = new xtc::xsh::Allocator<test_struct, STRUCT_HEAP_SIZE>(&protect);
  return 0;
}

extern "C" int cxx_cleanup_protected_allocator() {
  delete stralloc;
  return 0;
}

extern "C" void cxx_simple_test_protect() {
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  CU_ASSERT_EQUAL(stralloc->freeCount(), STRUCT_HEAP_SIZE);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  void *ptr = stralloc->allocate(sizeof(test_struct));
  CU_ASSERT_PTR_NOT_NULL(ptr);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  CU_ASSERT_EQUAL(stralloc->freeCount(), STRUCT_HEAP_SIZE - 1);
  CU_ASSERT_EQUAL(lock_called, 2);
  CU_ASSERT_EQUAL(unlock_called, 2);
  test_struct *pstr = stralloc->allocate();
  CU_ASSERT_PTR_NOT_NULL(pstr);
  CU_ASSERT_EQUAL(lock_called, 3);
  CU_ASSERT_EQUAL(unlock_called, 3);
  CU_ASSERT_EQUAL(stralloc->freeCount(), STRUCT_HEAP_SIZE - 2);
  CU_ASSERT_EQUAL(lock_called, 4);
  CU_ASSERT_EQUAL(unlock_called, 4);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  stralloc->free(ptr);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  stralloc->free(pstr);
  CU_ASSERT_EQUAL(lock_called, 2);
  CU_ASSERT_EQUAL(unlock_called, 2);
  CU_ASSERT_EQUAL(stralloc->freeCount(), STRUCT_HEAP_SIZE);
  CU_ASSERT_EQUAL(lock_called, 3);
  CU_ASSERT_EQUAL(unlock_called, 3);
  reset_protect();
}

extern "C" void cxx_multiple_test_protect() {
  void* arr[STRUCT_HEAP_SIZE];
  memset(arr, 0, sizeof(arr));
  int i;
  reset_protect();
  for(i = 0; i < STRUCT_HEAP_SIZE; i++) {
    arr[i] = stralloc->allocate(sizeof(test_struct));
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  CU_ASSERT_EQUAL(stralloc->freeCount(), 0);
  CU_ASSERT_EQUAL(lock_called, i+1);
  CU_ASSERT_EQUAL(unlock_called, i+1);
  reset_protect();
  for(i = 0; i < STRUCT_HEAP_SIZE; i++) {
    CU_ASSERT_PTR_NULL(stralloc->allocate());
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  CU_ASSERT_EQUAL(stralloc->freeCount(), 0);
  CU_ASSERT_EQUAL(lock_called, i+1);
  CU_ASSERT_EQUAL(unlock_called, i+1);
  reset_protect();
  for(i = 0; i < STRUCT_HEAP_SIZE; i++) {
    stralloc->free(arr[i]);
    arr[i] = NULL;
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  CU_ASSERT_EQUAL(stralloc->freeCount(), i);
  CU_ASSERT_EQUAL(lock_called, i+1);
  CU_ASSERT_EQUAL(unlock_called, i+1);
  reset_protect();
}

extern "C" void cxx_error_test_protect() {
  reset_protect();

  CU_ASSERT_PTR_NULL(stralloc->allocate(0));
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  CU_ASSERT_PTR_NULL(stralloc->allocate(sizeof(test_struct) - 1));
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  CU_ASSERT_PTR_NULL(stralloc->allocate(sizeof(test_struct) + 1));
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  stralloc->free((void*)42);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  stralloc->free((void*)0);
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  int bullshit;
  stralloc->free(&bullshit);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  void* ptr = stralloc->allocate(sizeof(test_struct));
  stralloc->free((void*)((char*)ptr - 1));
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  stralloc->free((void*)((char*)ptr + 1));
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  stralloc->free(ptr);
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  cxx_cleanup_protected_allocator(); cxx_init_protected_allocator();
  ptr = stralloc->allocate(sizeof(test_struct));
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  stralloc->free(ptr);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  stralloc->free(ptr);
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  char buf[sizeof(test_struct)];
  stralloc->free((void*)buf);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  // Fake node not done -> cannot get allocator inner node size

  reset_protect();

}
