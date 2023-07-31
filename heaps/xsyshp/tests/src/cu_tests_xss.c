#include "cu_tests.h"
#include "xtc_syshp.h"
#include "cxx_tests_xss.h"
#include <CUnit/CUnit.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#define TEST_PERFORMANCE_ROUND    10000000

static test_suite_t* get_ts_init_end_error();
static test_suite_t* get_ts_init_end();
static test_suite_t* get_ts_allocation_errors();
static test_suite_t* get_ts_operational();
static test_suite_t* get_ts_free_errors();
static test_suite_t* get_ts_end();
static test_suite_t* get_ts_protect();
static test_suite_t* get_ts_random();

static test_suite_t* get_ts_cxx_allocator();
static test_suite_t* get_ts_cxx_protected_allocator();

const test_suite_getter_t tsg_table[] = {
  get_ts_init_end_error,
  get_ts_init_end,
  get_ts_allocation_errors,
  get_ts_operational,
  get_ts_free_errors,
  get_ts_end,
  get_ts_protect,
  get_ts_random,
  get_ts_cxx_allocator,
  get_ts_cxx_protected_allocator,
  TEST_SUITE_GETTER_END
};

static void init_error_heap_mem();
static void init_error_protect();
static void end_error();

static test_suite_t* get_ts_init_end_error() {

  static test_t tests[] = {
    { "Init with bad heap / mem parameters", init_error_heap_mem },
    { "Init with bad protect parameter", init_error_protect },
    { "End with bad heap parameter", end_error },
    { NULL, NULL }
  };

  static test_suite_t suite = {
    .title = "Initialization errors",
    .init = NULL,
    .cleanup = NULL,
    .tests = tests
  };

  return &suite;
}

static void init_std();

static test_suite_t* get_ts_init_end() {

  static test_t tests[] = {
    { "Standard initialization and termination", init_std },
    { NULL, NULL }
  };

  static test_suite_t suite = {
    .title = "Initialization success",
    .init = NULL,
    .cleanup = NULL,
    .tests = tests
  };

  return &suite;
}

static int init_heap();
static int cleanup_heap();

static void alloc_null_heap();
static void alloc_bad_heap();
static void alloc_0_size();

static test_suite_t* get_ts_allocation_errors() {

  static test_t tests[] = {
    { "Allocate on NULL heap", alloc_null_heap },
    { "Allocate on bad heap", alloc_bad_heap },
    { "Allocate with 0 size", alloc_0_size },
    { NULL, NULL }
  };

  static test_suite_t suite = {
    .title = "Allocation errors",
    .init = init_heap,
    .cleanup = cleanup_heap,
    .tests = tests
  };

  return &suite;
}

static void simple_alloc_free();
static void sequential_alloc_free();
static void unordered_alloc_free();

static test_suite_t* get_ts_operational() {

  static test_t tests[] = {
    { "Simple allocation and free cycle", simple_alloc_free },
    { "Sequential allocation and sequential free", sequential_alloc_free },
    { "Unordered sequence of allocation and free", unordered_alloc_free },
    { NULL, NULL }
  };

  static test_suite_t suite = {
    .title = "Standard operations",
    .init = init_heap,
    .cleanup = cleanup_heap,
    .tests = tests
  };
  
  return &suite;
}

static void free_null();
static void free_invalid();
static void free_double();
static void free_not_allocated();
static void free_sys();
static void free_wrong();

static test_suite_t* get_ts_free_errors() {

  static test_t tests[] = {
    { "Free with NULL arguments", free_null },
    { "Free with invalid arguments", free_invalid },
    { "Double free of a element", free_double },
    { "Free on bad element", free_not_allocated },
    { "Free with system free", free_sys },
    { "Free with wrong heap", free_wrong },
    { NULL, NULL }
  };

  static test_suite_t suite = {
    .title = "Free errors",
    .init = init_heap,
    .cleanup = cleanup_heap,
    .tests = tests
  };

  return &suite;
}

static void end_none();
static void end_remain();

static test_suite_t* get_ts_end() {

  static test_t tests[] = {
    { "End with no remaining blocks", end_none },
    { "End with remaining blocks", end_remain },
    { NULL, NULL }
  };

  static test_suite_t suite = {
    .title = "Heap end",
    .init = NULL,
    .cleanup = NULL,
    .tests = tests
  };

  return &suite;
}

static int init_protected_heap();
static int cleanup_protected_heap();

static void simple_test_protect();
static void multiple_test_protect();
static void error_test_protect();
static void end_test_protect();

static test_suite_t* get_ts_protect() {

  static test_t tests[] = {
    { "Simple sequence on protected structure heap", simple_test_protect },
    { "Multiple sequences on protected structure heap", multiple_test_protect },
    { "Error sequences on protected structure heap", error_test_protect },
    { "Ending protected heap", end_test_protect },
    { NULL, NULL }
  };

  static test_suite_t suite = {
    .title = "Protected operations",
    .init = init_protected_heap,
    .cleanup = cleanup_protected_heap,
    .tests = tests
  };
  
  return &suite;
}

static void test_perf();

static test_suite_t* get_ts_random() {

  static test_t tests[] = {
    { "Test random && performance", test_perf },
    { NULL, NULL }
  };

  static test_suite_t suite = {
    .title = "Randomized allocation and free operations",
    .init = init_heap,
    .cleanup = cleanup_heap,
    .tests = tests
  };
  
  return &suite;
}

static test_suite_t* get_ts_cxx_allocator() {

  static test_t tests[] = {
    { "[C++] Allocate with 0 size", cxx_alloc_0_size },
    { "[C++] Simple allocation and free cycle", cxx_simple_alloc_free },
    { "[C++] Sequential allocation and sequential free", cxx_sequential_alloc_free },
    { "[C++] Unordered sequence of allocation and free", cxx_unordered_alloc_free },
    { "[C++] Free with NULL arguments", cxx_free_null },
    { "[C++] Free with invalid arguments", cxx_free_invalid },
    { "[C++] Double free of a element", cxx_free_double },
    { "[C++] Free on bad element", cxx_free_not_allocated },
    { "[C++] Free with wrong allocator", cxx_free_wrong },
    { NULL, NULL }
  };

  static test_suite_t suite = {
    .title = "C++ Unprotected Allocator",
    .init = cxx_init_allocator,
    .cleanup = cxx_cleanup_allocator,
    .tests = tests
  };

  return &suite;
}

static test_suite_t* get_ts_cxx_protected_allocator() {

  static test_t tests[] = {
    { "[C++] Simple sequence on protected system allocator", cxx_simple_test_protect },
    { "[C++] Multiple sequences on protected system allocator", cxx_multiple_test_protect },
    { "[C++] Error sequences on protected system allocator", cxx_error_test_protect },
    { NULL, NULL }
  };

  static test_suite_t suite = {
    .title = "C++ Protected Allocator",
    .init = cxx_init_protected_allocator,
    .cleanup = cxx_cleanup_protected_allocator,
    .tests = tests
  };

  return &suite;
}

static void init_error_heap_mem() {
  xtc_protect_t guard;
  CU_ASSERT_PTR_NULL(xss_init(NULL, &guard));
  CU_ASSERT_PTR_NULL(xss_init(NULL, NULL));
}

static void dummy() {}

static void init_error_protect() {
  xss_heap_t heap;
  xtc_protect_t guard = { .lock = NULL, .unlock = NULL };
  CU_ASSERT_PTR_NULL(xss_init(&heap, &guard));
  guard.lock = dummy;
  CU_ASSERT_PTR_NULL(xss_init(&heap, &guard));
  guard.lock = NULL;
  guard.unlock = dummy;
  CU_ASSERT_PTR_NULL(xss_init(&heap, &guard));
}

static void end_error() {
  xss_heap_t heap;
  size_t count;
  CU_ASSERT_PTR_NULL(xss_end(NULL, NULL));
  CU_ASSERT_PTR_NULL(xss_end(NULL, &count));
  CU_ASSERT_PTR_NULL(xss_end(&heap, &count));
  CU_ASSERT_PTR_NULL(xss_end(&heap, NULL));
}

static void init_std() {
  xss_heap_t heap, ref0;
  xtc_protect_t guard = { .lock = dummy, .unlock = dummy };
  memset(&ref0, 0, sizeof(xss_heap_t));

  memset(&heap, 0, sizeof(xss_heap_t));
  CU_ASSERT_PTR_NOT_NULL(xss_init(&heap, &guard));
  CU_ASSERT_EQUAL(xss_count(&heap), 0);
  CU_ASSERT_PTR_EQUAL(xss_end(&heap, NULL), NULL);
  CU_ASSERT_EQUAL(0, memcmp(&heap, &ref0, sizeof(xss_heap_t)));

  size_t count = 42;
  CU_ASSERT_PTR_NOT_NULL(xss_init(&heap, NULL));
  CU_ASSERT_EQUAL(xss_count(&heap), 0);
  CU_ASSERT_PTR_EQUAL(xss_end(&heap, &count), NULL);
  CU_ASSERT_EQUAL(count, 0);
  CU_ASSERT_EQUAL(0, memcmp(&heap, &ref0, sizeof(xss_heap_t)));
}

static xss_heap_t* test_heap() {
  static xss_heap_t heap;
  return &heap;
}

static int init_heap() {
  memset(test_heap(), 0, sizeof(xss_heap_t));
  xss_init(test_heap(), NULL);
  return 0;
}

static int cleanup_heap() {
  size_t count;
  void *pool = xss_end(test_heap(), &count);
  assert(NULL == pool); (void)pool;
  assert(!count);
  return 0;
}

static void alloc_null_heap() {
  CU_ASSERT_PTR_NULL(xss_alloc(NULL, 42));
}

static void alloc_bad_heap() {
  xss_heap_t heap;
  /* Uninitialized heap */
  CU_ASSERT_PTR_NULL(xss_alloc(&heap, 42));
  /* Invalid address */
  CU_ASSERT_PTR_NULL(xss_alloc((xss_heap_t*)(((char*)(&heap)) + 2), 42));
  CU_ASSERT_PTR_NULL(xss_alloc((xss_heap_t*)test_heap()->count, 42));
}

static void alloc_0_size() {
  size_t count = xss_count(test_heap());
  CU_ASSERT_PTR_NULL(xss_alloc(test_heap(), 0));
  CU_ASSERT_EQUAL(xss_count(test_heap()), count);
}

static void simple_alloc_free() {
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  void *ptr = xss_alloc(test_heap(), 42);
  CU_ASSERT_PTR_NOT_NULL(ptr);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 1);
  xss_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
}

#define TEST_HEAP_ROUND 42

static void sequential_alloc_free() {
  // Difficult to exhaust system heap on host PC
  void* arr[TEST_HEAP_ROUND];
  memset(arr, 0, sizeof(arr));
  int i;
  // Allocate all sequentially
  for (i = 0; i < TEST_HEAP_ROUND; i++) {
    CU_ASSERT_EQUAL(xss_count(test_heap()), i);
    arr[i] = xss_alloc(test_heap(), TEST_HEAP_ROUND*(i+1));
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  // Cannot check there is no more space (not exhausted)
  // Free all blocks
  for (i = 0; i < TEST_HEAP_ROUND; i++) {
    CU_ASSERT_EQUAL(xss_count(test_heap()), TEST_HEAP_ROUND-i);
    xss_free(test_heap(), arr[i]);
  }
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
}

static void unordered_alloc_free() {
  void* arr[TEST_HEAP_ROUND];
  memset(arr, 0, sizeof(arr));
  int i;
  // Alloc all
  for (i = 0; i < TEST_HEAP_ROUND; i++) {
    CU_ASSERT_EQUAL(xss_count(test_heap()), i);
    arr[i] = xss_alloc(test_heap(), (i+1)*TEST_HEAP_ROUND);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  // Free 1 over 2
  size_t count = xss_count(test_heap());
  CU_ASSERT_EQUAL(count, TEST_HEAP_ROUND);
  for (i = 0; i < TEST_HEAP_ROUND; i+=2) {
    CU_ASSERT_EQUAL(xss_count(test_heap()), count--);
    xss_free(test_heap(), arr[i]);
    arr[i] = NULL;
  }
  // Alloc again half of the previously freed
  for (i = 0; i < TEST_HEAP_ROUND; i+=4) {
    assert(!arr[i]);
    CU_ASSERT_EQUAL(xss_count(test_heap()), count++);
    arr[i] = xss_alloc(test_heap(), TEST_HEAP_ROUND * (i+1) * 2);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  for (i = 0; count < TEST_HEAP_ROUND; ) {
    while(arr[i]) i++;
    assert(i < TEST_HEAP_ROUND);
    CU_ASSERT_EQUAL(xss_count(test_heap()), count++);
    arr[i] = xss_alloc(test_heap(), TEST_HEAP_ROUND * (i+1) * 3);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  CU_ASSERT_EQUAL(xss_count(test_heap()), TEST_HEAP_ROUND);
  for (i = 0, count = 0; i < TEST_HEAP_ROUND; i++) {
    if (!arr[i]) continue;
    count++;
  }
  CU_ASSERT_EQUAL(count, TEST_HEAP_ROUND);
  for (i = 0; i < TEST_HEAP_ROUND; i++) {
    CU_ASSERT_EQUAL(xss_count(test_heap()), TEST_HEAP_ROUND - i);
    xss_free(test_heap(), arr[i]);
    arr[i] = NULL;
  }
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
}

static void free_null() {
  void* ptr = xss_alloc(test_heap(), 42);
  int count = xss_count(test_heap());
  xss_free(NULL, (void*)42);
  CU_ASSERT_EQUAL(xss_count(test_heap()), count);
  xss_free(test_heap(), NULL);
  CU_ASSERT_EQUAL(xss_count(test_heap()), count);
  xss_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(xss_count(test_heap()), --count);
}

static void free_invalid() {
  int bullshit;
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  xss_free(test_heap(), &bullshit);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  void* ptr = xss_alloc(test_heap(), 42);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 1);
  xss_free(test_heap(), (void*)((char*)ptr - 1));
  CU_ASSERT_EQUAL(xss_count(test_heap()), 1);
  xss_free(test_heap(), (void*)((char*)ptr + 1));
  CU_ASSERT_EQUAL(xss_count(test_heap()), 1);
  xss_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
}

static void free_double() {
  cleanup_heap(); init_heap();
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  void* ptr = xss_alloc(test_heap(), 42);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 1);
  xss_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  xss_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
}

static void free_not_allocated() {
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  char buf[42];
  xss_free(test_heap(), (void*)buf);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  // Build a fake node with real data but not allocated with structure heap
  xss_node_t *test = malloc(sizeof(xss_node_t));
  void *ptr = xss_alloc(test_heap(), 42);
  xss_node_t *node = (xss_node_t*)ptr - 1;
  CU_ASSERT_EQUAL(xss_count(test_heap()), 1);
  memcpy(test, node, sizeof(xss_node_t));
  // At least heap id must be 0 in this case
  test->heap = 0;
  xss_free(test_heap(), (void*)(test+1));
  CU_ASSERT_EQUAL(xss_count(test_heap()), 1);
  xss_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  free(test);
}

static void free_sys() {
/*
  => result is (e.g. it works!):
  Test: Free and system free ...free(): invalid pointer
Aborted
*/
#if 0
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  void* ptr = xss_alloc(test_heap(), 42);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 1);
  // First node of the allocated memory
  // -- Should not be freed because of xsh_node_t header
  free(ptr);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 1);
  xss_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
#endif
}

static void free_wrong() {
  // Declare a second system heap
  xss_heap_t heap2;
  memset(&heap2, 0, sizeof(xss_heap_t));
  xss_init(&heap2, NULL);
  CU_ASSERT_EQUAL(xss_count(&heap2), 0);

  void* ptr1 = xss_alloc(test_heap(), 42);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 1);
  void* ptr2 = xss_alloc(&heap2, 51);
  CU_ASSERT_EQUAL(xss_count(&heap2), 1);

  xss_free(&heap2, ptr1);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 1);
  CU_ASSERT_EQUAL(xss_count(&heap2), 1);
  xss_free(test_heap(), ptr2);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 1);
  CU_ASSERT_EQUAL(xss_count(&heap2), 1);

  xss_free(&heap2, ptr2);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 1);
  CU_ASSERT_EQUAL(xss_count(&heap2), 0);
  xss_free(test_heap(), ptr1);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  CU_ASSERT_EQUAL(xss_count(&heap2), 0);

  memset(&heap2, 0, sizeof(xss_heap_t));
}

static void end_none() {
  xss_heap_t heap, ref0;
  size_t count;
  memset(&ref0, 0, sizeof(xss_heap_t));
  void* ptr[3];

  memset(&heap, 0, sizeof(xss_heap_t));
  CU_ASSERT_PTR_NOT_NULL(xss_init(&heap, NULL));
  ptr[0] = xss_alloc(&heap, 42);
  ptr[1] = xss_alloc(&heap, 51);
  ptr[2] = xss_alloc(&heap, 69);
  CU_ASSERT_PTR_NOT_NULL(ptr[0]);
  CU_ASSERT_PTR_NOT_NULL(ptr[1]);
  CU_ASSERT_PTR_NOT_NULL(ptr[2]);
  CU_ASSERT_EQUAL(xss_count(&heap), 3);
  xss_free(&heap, ptr[1]);
  xss_free(&heap, ptr[0]);
  xss_free(&heap, ptr[2]);
  CU_ASSERT_EQUAL(xss_count(&heap), 0);
  CU_ASSERT_PTR_EQUAL(xss_end(&heap, NULL), NULL);
  CU_ASSERT_EQUAL(0, memcmp(&heap, &ref0, sizeof(xss_heap_t)));

  CU_ASSERT_PTR_NOT_NULL(xss_init(&heap, NULL));
  ptr[0] = xss_alloc(&heap, 42);
  ptr[1] = xss_alloc(&heap, 51);
  ptr[2] = xss_alloc(&heap, 69);
  CU_ASSERT_PTR_NOT_NULL(ptr[0]);
  CU_ASSERT_PTR_NOT_NULL(ptr[1]);
  CU_ASSERT_PTR_NOT_NULL(ptr[2]);
  CU_ASSERT_EQUAL(xss_count(&heap), 3);
  xss_free(&heap, ptr[1]);
  xss_free(&heap, ptr[0]);
  xss_free(&heap, ptr[2]);
  CU_ASSERT_EQUAL(xss_count(&heap), 0);
  CU_ASSERT_PTR_EQUAL(xss_end(&heap, &count), NULL);
  CU_ASSERT_EQUAL(0, memcmp(&heap, &ref0, sizeof(xss_heap_t)));
  CU_ASSERT_EQUAL(count, 0);
}

static void end_remain() {
  xss_heap_t heap, ref0;
  size_t count;
  memset(&ref0, 0, sizeof(xss_heap_t));
  void* ptr[3];

  memset(&heap, 0, sizeof(xss_heap_t));
  CU_ASSERT_PTR_NOT_NULL(xss_init(&heap, NULL));
  ptr[0] = xss_alloc(&heap, 42);
  ptr[1] = xss_alloc(&heap, 51);
  ptr[2] = xss_alloc(&heap, 69);
  CU_ASSERT_EQUAL(xss_count(&heap), 3);
  CU_ASSERT_PTR_NOT_NULL(ptr[0]);
  CU_ASSERT_PTR_NOT_NULL(ptr[1]);
  CU_ASSERT_PTR_NOT_NULL(ptr[2]);
  CU_ASSERT_PTR_EQUAL(xss_end(&heap, NULL), NULL);
  CU_ASSERT_EQUAL(0, memcmp(&heap, &ref0, sizeof(xss_heap_t)));

  CU_ASSERT_PTR_NOT_NULL(xss_init(&heap, NULL));
  ptr[0] = xss_alloc(&heap, 42);
  ptr[1] = xss_alloc(&heap, 51);
  ptr[2] = xss_alloc(&heap, 69);
  CU_ASSERT_EQUAL(xss_count(&heap), 3);
  CU_ASSERT_PTR_NOT_NULL(ptr[0]);
  CU_ASSERT_PTR_NOT_NULL(ptr[1]);
  CU_ASSERT_PTR_NOT_NULL(ptr[2]);
  xss_free(&heap, ptr[1]);
  CU_ASSERT_EQUAL(xss_count(&heap), 2);
  CU_ASSERT_PTR_EQUAL(xss_end(&heap, &count), NULL);
  CU_ASSERT_EQUAL(0, memcmp(&heap, &ref0, sizeof(xss_heap_t)));
  CU_ASSERT_EQUAL(count, 2);
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

static int init_protected_heap() {
  xtc_protect_t protect = { .lock = lock, .unlock = unlock };
  memset(test_heap(), 0, sizeof(xss_heap_t));
  xss_init(test_heap(), &protect);
  reset_protect();
  return 0;
}

static int cleanup_protected_heap() {
  reset_protect();
  size_t count;
  void *pool = xss_end(test_heap(), &count);
  assert(NULL == pool); (void)pool;
  assert(!count);
  return 0;
}

static void simple_test_protect() {
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  void *ptr = xss_alloc(test_heap(), 42);
  CU_ASSERT_PTR_NOT_NULL(ptr);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 1);
  CU_ASSERT_EQUAL(lock_called, 2);
  CU_ASSERT_EQUAL(unlock_called, 2);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  xss_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  CU_ASSERT_EQUAL(lock_called, 2);
  CU_ASSERT_EQUAL(unlock_called, 2);
  reset_protect();
}

static void multiple_test_protect() {
  void* arr[TEST_HEAP_ROUND];
  memset(arr, 0, sizeof(arr));
  int i;
  reset_protect();
  for(i = 0; i < TEST_HEAP_ROUND; i++) {
    arr[i] = xss_alloc(test_heap(), TEST_HEAP_ROUND*(i+1));
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  CU_ASSERT_EQUAL(xss_count(test_heap()), TEST_HEAP_ROUND);
  CU_ASSERT_EQUAL(lock_called, i+1);
  CU_ASSERT_EQUAL(unlock_called, i+1);
  reset_protect();
  for(i = 0; i < TEST_HEAP_ROUND; i++) {
    xss_free(test_heap(), arr[i]);
    arr[i] = NULL;
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  CU_ASSERT_EQUAL(lock_called, i+1);
  CU_ASSERT_EQUAL(unlock_called, i+1);
  reset_protect();
}

static void error_test_protect() {
  int lock_calls = 0;
  reset_protect();

  xss_heap_t heap;
  CU_ASSERT_PTR_NULL(xss_alloc(&heap, 42));// lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  CU_ASSERT_PTR_NULL(xss_alloc((xss_heap_t*)(((char*)(&heap)) + 2), 42));// lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  CU_ASSERT_PTR_NULL(xss_alloc((xss_heap_t*)test_heap()->count, 42));// lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  CU_ASSERT_PTR_NULL(xss_alloc(test_heap(), 0));// lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  xss_free(NULL, (void*)42);// lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xss_free(test_heap(), NULL);// lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  int bullshit;
  xss_free(test_heap(), &bullshit); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  void* ptr = xss_alloc(test_heap(), 42); lock_calls++;
  xss_free(test_heap(), (void*)((char*)ptr - 1)); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xss_free(test_heap(), (void*)((char*)ptr + 1)); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xss_free(test_heap(), ptr); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  CU_ASSERT_EQUAL(lock_called, lock_calls);

  lock_calls = 0;
  cleanup_protected_heap(); init_protected_heap();
  ptr = xss_alloc(test_heap(), 42); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xss_free(test_heap(), ptr); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xss_free(test_heap(), ptr); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  char buf[42];
  xss_free(test_heap(), (void*)buf); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  // Build a fake node with real data but not allocated with structure heap
  xss_node_t *test = malloc(sizeof(xss_node_t));
  ptr = xss_alloc(test_heap(), 42); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xss_node_t *node = (xss_node_t*)ptr - 1;
  memcpy(test, node, sizeof(xss_node_t));
  test->heap = 0;
  xss_free(test_heap(), (void*)(test+1)); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xss_free(test_heap(), ptr); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  free(test);

  CU_ASSERT_EQUAL(lock_called, lock_calls);

  reset_protect();
}

static void end_test_protect() {
  xss_heap_t ref0;
  memset(&ref0, 0, sizeof(xss_heap_t));

  CU_ASSERT_PTR_EQUAL(xss_end(test_heap(), NULL), NULL);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, lock_called);
  CU_ASSERT_EQUAL(0, memcmp(test_heap(), &ref0, sizeof(xss_heap_t)));

  size_t count;
  init_protected_heap();
  CU_ASSERT_PTR_EQUAL(xss_end(test_heap(), &count), NULL);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, lock_called);
  CU_ASSERT_EQUAL(0, memcmp(test_heap(), &ref0, sizeof(xss_heap_t)));

  init_protected_heap();  // so that cleanup works
}

static void test_perf() {
  void* arr[TEST_HEAP_ROUND];
  memset(arr, 0, sizeof(arr));
  srand(time(NULL));
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  int na = 0, nf = 0, last = 0;
  clock_t sa = 0, sf = 0; 
  printf("\n ===== %03d%% : ", last);
  for (int i = 0; i < TEST_PERFORMANCE_ROUND; i++) {
    int adv = (i * 100) / TEST_PERFORMANCE_ROUND;
    if (last != adv) {
      last = adv;
      printf("\r ===== %03d%% : ", last);
    }
    int id = rand() % TEST_HEAP_ROUND;
    size_t count = 0;
    if (arr[id]) {
      count = xss_count(test_heap());
      clock_t start = clock();
      xss_free(test_heap(), arr[id]);
      sf += clock() - start;
      arr[id] = NULL;
      nf++;
      CU_ASSERT_EQUAL(xss_count(test_heap()), count - 1);
    } else {
      count = xss_count(test_heap());
      clock_t start = clock();
      arr[id] = xss_alloc(test_heap(), 42);
      sa += clock() - start;
      na++;
      CU_ASSERT_PTR_NOT_NULL(arr[id]);
      CU_ASSERT_EQUAL(xss_count(test_heap()), count + 1);
    }
  }
  printf("\r ===== 100%% : ");
  for (int i = 0; i < TEST_HEAP_ROUND; i++) {
    if (arr[i]) {
      clock_t start = clock();
      xss_free(test_heap(), arr[i]);
      sf += clock() - start;
      arr[i] = NULL;
      nf++;
    }
  }
  CU_ASSERT_EQUAL(xss_count(test_heap()), 0);
  CU_ASSERT_EQUAL(na, nf);
  double avga, avgf;
  avga = (double)sa * 1e9 / CLOCKS_PER_SEC / na;
  avgf = (double)sf * 1e9 / CLOCKS_PER_SEC / nf;
  printf("#alloc. = %.3lf ns [%d] #free. = %.3lf ns [%d] : ...", avga, na, avgf, nf);
}
