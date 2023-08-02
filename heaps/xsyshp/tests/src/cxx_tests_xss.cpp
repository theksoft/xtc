#include <CUnit/CUnit.h>
#include <cassert>
#include "cxx_tests_xss.h"
#include "xtc_syshp.hxx"

#define HEAP_ROUND  12

static xtc::xss::Allocator *sysalloc;

extern "C" int cxx_init_allocator() {
  sysalloc = new xtc::xss::Allocator();
  return 0;
}

extern "C" int cxx_cleanup_allocator() {
  delete sysalloc;
  return 0;
}

extern "C" void cxx_alloc_0_size() {
  size_t count = sysalloc->count();
  CU_ASSERT_PTR_NULL(sysalloc->allocate(0));
  CU_ASSERT_EQUAL(sysalloc->count(), count);
}

extern "C" void cxx_simple_alloc_free() {
  CU_ASSERT_EQUAL(sysalloc->count(), 0);
  void *ptr = sysalloc->allocate(42);
  CU_ASSERT_PTR_NOT_NULL(ptr);
  CU_ASSERT_EQUAL(sysalloc->count(), 1);
  sysalloc->free(ptr);
  CU_ASSERT_EQUAL(sysalloc->count(), 0);
}

extern "C" void cxx_sequential_alloc_free() {
  void* arr[HEAP_ROUND];
  memset(arr, 0, sizeof(arr));
  int i;
  // Allocate all sequentially
  for (i = 0; i < HEAP_ROUND; i++) {
    CU_ASSERT_EQUAL(sysalloc->count(), i);
    arr[i] = sysalloc->allocate(12*(i+1));
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  // Cannot check there is no more space (unlimited to some extent)
  // Free all blocks
  for (i = 0; i < HEAP_ROUND; i++) {
    CU_ASSERT_EQUAL(sysalloc->count(), HEAP_ROUND - i);
    sysalloc->free(arr[i]);
  }
  CU_ASSERT_EQUAL(sysalloc->count(), 0);
}

extern "C" void cxx_unordered_alloc_free() {
  void* arr[HEAP_ROUND];
  memset(arr, 0, sizeof(arr));
  int i;
  // Alloc all
  for (i = 0; i < HEAP_ROUND; i++) {
    CU_ASSERT_EQUAL(sysalloc->count(), i);
    arr[i] = sysalloc->allocate(12*(i+1));
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  // Free 1 over 2
  size_t count = sysalloc->count();
  CU_ASSERT_EQUAL(count, HEAP_ROUND);
  for (i = 0; i < HEAP_ROUND; i+=2) {
    CU_ASSERT_EQUAL(sysalloc->count(), count--);
    sysalloc->free(arr[i]);
    arr[i] = NULL;
  }
  // Alloc again half of the previously freed
  for (i = 0; i < HEAP_ROUND; i+=4) {
    assert(!arr[i]);
    CU_ASSERT_EQUAL(sysalloc->count(), count++);
    arr[i] = sysalloc->allocate(12*(i+1)*(i+1));
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  for (i = 0; count < HEAP_ROUND; ) {
    while(arr[i]) i++;
    assert(i < HEAP_ROUND);
    CU_ASSERT_EQUAL(sysalloc->count(), count++);
    arr[i] = sysalloc->allocate(12*(i+1)*(i+1));
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  CU_ASSERT_EQUAL(sysalloc->count(), HEAP_ROUND);
  for (i = 0; i < HEAP_ROUND; i++) {
    if (!arr[i]) continue;
    count--;
  }
  CU_ASSERT_EQUAL(count, 0);
  for (i = 0; i < HEAP_ROUND; i++) {
    CU_ASSERT_EQUAL(sysalloc->count(), HEAP_ROUND - i);
    sysalloc->free(arr[i]);
    arr[i] = NULL;
  }
  CU_ASSERT_EQUAL(sysalloc->count(), 0);
}

extern "C" void cxx_free_null() {
  void* ptr = sysalloc->allocate(42);
  int count = sysalloc->count();
  sysalloc->free((void*)NULL);
  CU_ASSERT_EQUAL(sysalloc->count(), count);
  sysalloc->free(ptr);
  CU_ASSERT_EQUAL(sysalloc->count(), --count);
}

extern "C" void cxx_free_invalid() {
  int bullshit;
  CU_ASSERT_EQUAL(sysalloc->count(), 0);
  sysalloc->free(&bullshit);
  CU_ASSERT_EQUAL(sysalloc->count(), 0);
  void* ptr = sysalloc->allocate(42);
  CU_ASSERT_EQUAL(sysalloc->count(), 1);
  sysalloc->free((void*)((char*)ptr - 1));
  CU_ASSERT_EQUAL(sysalloc->count(), 1);
  sysalloc->free((void*)((char*)ptr + 1));
  CU_ASSERT_EQUAL(sysalloc->count(), 1);
  sysalloc->free(ptr);
  CU_ASSERT_EQUAL(sysalloc->count(), 0);
}

extern "C" void cxx_free_double() {
  cxx_cleanup_allocator(); cxx_init_allocator();
  CU_ASSERT_EQUAL(sysalloc->count(), 0);
  void* ptr = sysalloc->allocate(42);
  CU_ASSERT_EQUAL(sysalloc->count(), 1);
  sysalloc->free(ptr);
  CU_ASSERT_EQUAL(sysalloc->count(), 0);
  sysalloc->free(ptr);
  CU_ASSERT_EQUAL(sysalloc->count(), 0);
}

extern "C" void cxx_free_not_allocated() {
  CU_ASSERT_EQUAL(sysalloc->count(), 0);
  char buf[42];
  sysalloc->free((void*)buf);
  CU_ASSERT_EQUAL(sysalloc->count(), 0);
  // Fake node cannot be built - class inner
}

extern "C" void cxx_free_wrong() {
  // Declare a second heap on same structure - on stack
  xtc::xss::Allocator heap2;
  CU_ASSERT_EQUAL(heap2.count(), 0);

  void* ptr1 = sysalloc->allocate(42);
  CU_ASSERT_EQUAL(sysalloc->count(), 1);
  void* ptr2 = heap2.allocate(51);
  CU_ASSERT_EQUAL(heap2.count(), 1);

  heap2.free(ptr1);
  CU_ASSERT_EQUAL(sysalloc->count(), 1);
  CU_ASSERT_EQUAL(heap2.count(), 1);
  sysalloc->free(ptr2);
  CU_ASSERT_EQUAL(sysalloc->count(), 1);
  CU_ASSERT_EQUAL(heap2.count(), 1);

  heap2.free(ptr2);
  CU_ASSERT_EQUAL(sysalloc->count(), 1);
  CU_ASSERT_EQUAL(heap2.count(), 0);
  sysalloc->free(ptr1);
  CU_ASSERT_EQUAL(sysalloc->count(), 0);
  CU_ASSERT_EQUAL(heap2.count(), 0);
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

static xtc::xss::Allocator *ptdalloc;

extern "C" int cxx_init_protected_allocator() {
  xtc_protect_t protect = { lock, unlock };
  reset_protect();
  ptdalloc = new xtc::xss::Allocator(&protect);
  return 0;
}

extern "C" int cxx_cleanup_protected_allocator() {
  delete ptdalloc;
  return 0;
}

extern "C" void cxx_simple_test_protect() {
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  CU_ASSERT_EQUAL(ptdalloc->count(), 0);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  void *ptr = ptdalloc->allocate(42);
  CU_ASSERT_PTR_NOT_NULL(ptr);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  CU_ASSERT_EQUAL(ptdalloc->count(), 1);
  CU_ASSERT_EQUAL(lock_called, 2);
  CU_ASSERT_EQUAL(unlock_called, 2);
  void *pstr = ptdalloc->allocate(51);
  CU_ASSERT_PTR_NOT_NULL(pstr);
  CU_ASSERT_EQUAL(lock_called, 3);
  CU_ASSERT_EQUAL(unlock_called, 3);
  CU_ASSERT_EQUAL(ptdalloc->count(), 2);
  CU_ASSERT_EQUAL(lock_called, 4);
  CU_ASSERT_EQUAL(unlock_called, 4);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  ptdalloc->free(ptr);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  ptdalloc->free(pstr);
  CU_ASSERT_EQUAL(lock_called, 2);
  CU_ASSERT_EQUAL(unlock_called, 2);
  CU_ASSERT_EQUAL(ptdalloc->count(), 0);
  CU_ASSERT_EQUAL(lock_called, 3);
  CU_ASSERT_EQUAL(unlock_called, 3);
  reset_protect();
}

extern "C" void cxx_multiple_test_protect() {
  void* arr[HEAP_ROUND];
  memset(arr, 0, sizeof(arr));
  int i;
  reset_protect();
  for(i = 0; i < HEAP_ROUND; i++) {
    arr[i] = ptdalloc->allocate(42*(i+1));
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  CU_ASSERT_EQUAL(ptdalloc->count(), HEAP_ROUND);
  CU_ASSERT_EQUAL(lock_called, i+1);
  CU_ASSERT_EQUAL(unlock_called, i+1);
  reset_protect();
  for(i = 0; i < HEAP_ROUND; i++) {
    ptdalloc->free(arr[i]);
    arr[i] = NULL;
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  CU_ASSERT_EQUAL(ptdalloc->count(), 0);
  CU_ASSERT_EQUAL(lock_called, i+1);
  CU_ASSERT_EQUAL(unlock_called, i+1);
  reset_protect();
}

extern "C" void cxx_error_test_protect() {
  reset_protect();

  CU_ASSERT_PTR_NULL(ptdalloc->allocate(0));
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  ptdalloc->free((void*)0);
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  int bullshit;
  ptdalloc->free(&bullshit);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  void* ptr = ptdalloc->allocate(42);
  ptdalloc->free((void*)((char*)ptr - 1));
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  ptdalloc->free((void*)((char*)ptr + 1));
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  ptdalloc->free(ptr);
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  cxx_cleanup_protected_allocator(); cxx_init_protected_allocator();
  ptr = ptdalloc->allocate(51);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  ptdalloc->free(ptr);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  ptdalloc->free(ptr);
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  char buf[69];
  ptdalloc->free((void*)buf);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  // Fake node not done -> cannot get allocator inner node size

  reset_protect();
}
