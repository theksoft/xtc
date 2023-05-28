#include "cu_tests.h"
#include "xtc_strhp.h"
#include "cxx_tests_xsh.h"
#include <CUnit/CUnit.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#define TEST_PERFORMANCE_ROUND    10000000

static test_suite_t* get_ts_init_error();
static test_suite_t* get_ts_init();
static test_suite_t* get_ts_allocation_errors();
static test_suite_t* get_ts_operational();
static test_suite_t* get_ts_free_errors();
static test_suite_t* get_ts_protect();
static test_suite_t* get_ts_random();

static test_suite_t* get_ts_cxx_allocator();
static test_suite_t* get_ts_cxx_protected_allocator();

const test_suite_getter_t tsg_table[] = {
  get_ts_init_error,
  get_ts_init,
  get_ts_allocation_errors,
  get_ts_operational,
  get_ts_free_errors,
  get_ts_protect,
  get_ts_random,
  get_ts_cxx_allocator,
  get_ts_cxx_protected_allocator,
  TEST_SUITE_GETTER_END
};

static void init_error_heap_mem();
static void init_error_sizes();
static void init_error_protect();

static test_suite_t* get_ts_init_error() {

  static test_t tests[] = {
    { "Bad heap / mem parameters", init_error_heap_mem },
    { "Bad pool or struct size parameters", init_error_sizes },
    { "Bad protect parameter", init_error_protect },
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

static void init_static();
static void init_dynamic();

static test_suite_t* get_ts_init() {

  static test_t tests[] = {
    { "Static initializations", init_static },
    { "Dynamic initialisations", init_dynamic },
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
static void alloc_bad_size();

static test_suite_t* get_ts_allocation_errors() {

  static test_t tests[] = {
    { "Allocate on NULL heap", alloc_null_heap },
    { "Allocate on bad heap", alloc_bad_heap },
    { "Allocate with 0 size", alloc_0_size },
    { "Allocate with bad size", alloc_bad_size },
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

static int init_protected_heap();
static int cleanup_protected_heap();

static void simple_test_protect();
static void multiple_test_protect();
static void error_test_protect();

static test_suite_t* get_ts_protect() {

  static test_t tests[] = {
    { "Simple sequence on protected structure heap", simple_test_protect },
    { "Multiple sequences on protected structure heap", multiple_test_protect },
    { "Error sequences on protected structure heap", error_test_protect },
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
    { "[C++] Allocate with bad size", cxx_alloc_bad_size },
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
    { "[C++] Simple sequence on protected structure allocator", cxx_simple_test_protect },
    { "[C++] Multiple sequences on protected structure allocator", cxx_multiple_test_protect },
    { "[C++] Error sequences on protected structure allocator", cxx_error_test_protect },
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
  int pool;
  xtc_protect_t guard;
  xsh_heap_t heap;
  CU_ASSERT_PTR_NULL(xsh_init(NULL, NULL, 512, 10, &guard));
  CU_ASSERT_PTR_NULL(xsh_init(NULL, &pool, 512, 10, &guard));
  CU_ASSERT_PTR_NULL(xsh_init(NULL, &pool, 512, 10, NULL));
  CU_ASSERT_PTR_NULL(xsh_init(&heap, NULL, 512, 10, &guard));
  CU_ASSERT_PTR_NULL(xsh_init(&heap, NULL, 512, 10, NULL));
}

static void init_error_sizes() {
  int pool;
  xtc_protect_t guard;
  xsh_heap_t heap;
  CU_ASSERT_PTR_NULL(xsh_init(&heap, &pool, 512, 0, &guard));
  CU_ASSERT_PTR_NULL(xsh_init(&heap, &pool, 512, 0, NULL));
  CU_ASSERT_PTR_NULL(xsh_init(&heap, &pool, 0, 10, &guard));
  CU_ASSERT_PTR_NULL(xsh_init(&heap, &pool, 0, 10, NULL));
  CU_ASSERT_PTR_NULL(xsh_init(&heap, &pool, 10, 10, &guard));
  CU_ASSERT_PTR_NULL(xsh_init(&heap, &pool, 10, 10, NULL));
  CU_ASSERT_PTR_NULL(xsh_init(&heap, &pool, 12, 10, &guard));
  CU_ASSERT_PTR_NULL(xsh_init(&heap, &pool, 12, 10, NULL));
}

static void dummy() {}

static void init_error_protect() {
  int pool;
  xsh_heap_t heap;
  xtc_protect_t guard = { .lock = NULL, .unlock = NULL };
  CU_ASSERT_PTR_NULL(xsh_init(&heap, &pool, 512, 10, &guard));
  guard.lock = dummy;
  CU_ASSERT_PTR_NULL(xsh_init(&heap, &pool, 512, 10, &guard));
  guard.lock = NULL;
  guard.unlock = dummy;
  CU_ASSERT_PTR_NULL(xsh_init(&heap, &pool, 512, 10, &guard));
}

static void init_static() {
  typedef struct { char buf[10]; } test_struct;
  size_t length = XSH_HEAP_LENGTH(test_struct,42);
  char pool[length];
  xsh_heap_t heap;
  xtc_protect_t guard = { .lock = dummy, .unlock = dummy };
  memset(&heap, 0, sizeof(xsh_heap_t));
  CU_ASSERT_PTR_NOT_NULL(xsh_init(&heap, &pool, length, sizeof(test_struct), &guard));
  CU_ASSERT_EQUAL(xsh_free_count(&heap), 42);
  memset(&heap, 0, sizeof(xsh_heap_t));
  CU_ASSERT_PTR_NOT_NULL(xsh_init(&heap, &pool, length, sizeof(test_struct), NULL));
  CU_ASSERT_EQUAL(xsh_free_count(&heap), 42);
}

static void init_dynamic() {
  typedef struct { char buf[10]; } test_struct;
  size_t length = XSH_HEAP_LENGTH(test_struct,51);
  char *pool = (char*)malloc(length);
  xsh_heap_t heap;
  xtc_protect_t guard = { .lock = dummy, .unlock = dummy };
  memset(&heap, 0, sizeof(xsh_heap_t));
  CU_ASSERT_PTR_NOT_NULL(xsh_init(&heap, pool, length, sizeof(test_struct), &guard));
  CU_ASSERT_EQUAL(xsh_free_count(&heap), 51);
  memset(&heap, 0, sizeof(xsh_heap_t));
  CU_ASSERT_PTR_NOT_NULL(xsh_init(&heap, pool, length, sizeof(test_struct), NULL));
  CU_ASSERT_EQUAL(xsh_free_count(&heap), 51);
  free(pool);
}

#define TEST_HEAP_STRUCT_SIZE   11
#define TEST_HEAP_SIZE          42

static xsh_heap_t* test_heap() {
  static xsh_heap_t heap;
  return &heap;
}

static int init_heap() {
  typedef struct { char buf[TEST_HEAP_STRUCT_SIZE]; } test_struct;
  size_t length = XSH_HEAP_LENGTH(test_struct,TEST_HEAP_SIZE);
  char* pool = malloc(length);
  memset(test_heap(), 0, sizeof(xsh_heap_t));
  xsh_init(test_heap(), pool, length, sizeof(test_struct), NULL);
  return 0;
}

static int cleanup_heap() {
  void *pool = test_heap()->mem_pool;
  memset(test_heap(), 0, sizeof(xsh_heap_t));
  free(pool);
  return 0;
}

static void alloc_null_heap() {
  CU_ASSERT_PTR_NULL(xsh_alloc(NULL, 42));
}

static void alloc_bad_heap() {
  xsh_heap_t heap;
  /* Uninitialized heap */
  CU_ASSERT_PTR_NULL(xsh_alloc(&heap, 42));
  /* Invalid address */
  CU_ASSERT_PTR_NULL(xsh_alloc((xsh_heap_t*)(((char*)(&heap)) + 2), 42));
  CU_ASSERT_PTR_NULL(xsh_alloc((xsh_heap_t*)test_heap()->mem_pool, 42));
}

static void alloc_0_size() {
  size_t count = xsh_free_count(test_heap());
  CU_ASSERT_PTR_NULL(xsh_alloc(test_heap(), 0));
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), count);
}

static void alloc_bad_size() {
  size_t count = xsh_free_count(test_heap());
  CU_ASSERT_PTR_NULL(xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE - 1));
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), count);
  CU_ASSERT_PTR_NULL(xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE + 1));
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), count);
}

static void simple_alloc_free() {
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
  void *ptr = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
  CU_ASSERT_PTR_NOT_NULL(ptr);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - 1);
  xsh_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
}

static void sequential_alloc_free() {
  void* arr[TEST_HEAP_SIZE];
  memset(arr, 0, sizeof(arr));
  int i;
  // Allocate all sequentially
  for (i = 0; i < TEST_HEAP_SIZE; i++) {
    CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - i);
    arr[i] = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  // Check there is no more space
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), 0);
  CU_ASSERT_PTR_NULL(xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE));
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), 0);
  // Free all blocks
  for (i = 0; i < TEST_HEAP_SIZE; i++) {
    CU_ASSERT_EQUAL(xsh_free_count(test_heap()), i);
    xsh_free(test_heap(), arr[i]);
  }
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
}

static void unordered_alloc_free() {
  void* arr[TEST_HEAP_SIZE];
  memset(arr, 0, sizeof(arr));
  int i;
  // Alloc all
  for (i = 0; i < TEST_HEAP_SIZE; i++) {
    CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - i);
    arr[i] = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  // Free 1 over 2
  size_t count = xsh_free_count(test_heap());
  CU_ASSERT_EQUAL(count, 0);
  for (i = 0; i < TEST_HEAP_SIZE; i+=2) {
    CU_ASSERT_EQUAL(xsh_free_count(test_heap()), count++);
    xsh_free(test_heap(), arr[i]);
    arr[i] = NULL;
  }
  // Alloc again half of the previously freed
  for (i = 0; i < TEST_HEAP_SIZE; i+=4) {
    assert(!arr[i]);
    CU_ASSERT_EQUAL(xsh_free_count(test_heap()), count--);
    arr[i] = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  for (i = 0; count; ) {
    while(arr[i]) i++;
    assert(i < TEST_HEAP_SIZE);
    CU_ASSERT_EQUAL(xsh_free_count(test_heap()), count--);
    arr[i] = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), 0);
  CU_ASSERT_PTR_NULL(xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE));
  for (i = 0; i < TEST_HEAP_SIZE; i++) {
    if (!arr[i]) continue;
    count++;
  }
  CU_ASSERT_EQUAL(count, TEST_HEAP_SIZE);
  for (i = 0; i < TEST_HEAP_SIZE; i++) {
    CU_ASSERT_EQUAL(xsh_free_count(test_heap()), i);
    xsh_free(test_heap(), arr[i]);
    arr[i] = NULL;
  }
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
}

static void free_null() {
  void* ptr = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
  int count = xsh_free_count(test_heap());
  xsh_free(NULL, (void*)42);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), count);
  xsh_free(test_heap(), NULL);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), count);
  xsh_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), ++count);
}

static void free_invalid() {
  int bullshit;
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
  xsh_free(test_heap(), &bullshit);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
  void* ptr = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - 1);
  xsh_free(test_heap(), (void*)((char*)ptr - 1));
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - 1);
  xsh_free(test_heap(), (void*)((char*)ptr + 1));
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - 1);
  xsh_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
}

static void free_double() {
  cleanup_heap(); init_heap();
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
  void* ptr = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - 1);
  xsh_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
  xsh_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
}

static void free_not_allocated() {
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
  char buf[TEST_HEAP_STRUCT_SIZE];
  xsh_free(test_heap(), (void*)buf);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
  // Build a fake node with real data but not allocated with structure heap
  xsh_node_t *test = malloc(test_heap()->node_size);
  void *ptr = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
  xsh_node_t *node = (xsh_node_t*)ptr - 1;
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - 1);
  memcpy(test, node, test_heap()->node_size);
  xsh_free(test_heap(), (void*)(test+1));
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - 1);
  xsh_free(test_heap(), ptr);
  free(test);
}

static void free_sys() {
/*
  => result is (e.g. it works!):
  Test: Free and system free ...free(): invalid pointer
Aborted
*/
#if 0
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
  void* ptr = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - 1);
  // First node of the allocated memory
  // -- Should not be freed because of xsh_node_t header
  free(ptr);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - 1);
  xsh_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
#endif
}

static void free_wrong() {
  // Declare a second heap on same structure
  typedef struct { char buf[TEST_HEAP_STRUCT_SIZE]; } test_struct;
  size_t length = XSH_HEAP_LENGTH(test_struct,TEST_HEAP_SIZE);
  char* pool = malloc(length);
  xsh_heap_t heap2;
  memset(&heap2, 0, sizeof(xsh_heap_t));
  xsh_init(&heap2, pool, length, sizeof(test_struct), NULL);
  CU_ASSERT_EQUAL(xsh_free_count(&heap2), TEST_HEAP_SIZE);

  void* ptr1 = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - 1);
  void* ptr2 = xsh_alloc(&heap2, TEST_HEAP_STRUCT_SIZE);
  CU_ASSERT_EQUAL(xsh_free_count(&heap2), TEST_HEAP_SIZE - 1);

  xsh_free(&heap2, ptr1);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - 1);
  CU_ASSERT_EQUAL(xsh_free_count(&heap2), TEST_HEAP_SIZE - 1);
  xsh_free(test_heap(), ptr2);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - 1);
  CU_ASSERT_EQUAL(xsh_free_count(&heap2), TEST_HEAP_SIZE - 1);

  xsh_free(&heap2, ptr2);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - 1);
  CU_ASSERT_EQUAL(xsh_free_count(&heap2), TEST_HEAP_SIZE);
  xsh_free(test_heap(), ptr1);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
  CU_ASSERT_EQUAL(xsh_free_count(&heap2), TEST_HEAP_SIZE);

  memset(&heap2, 0, sizeof(xsh_heap_t));
  free(pool);
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
  typedef struct { char buf[TEST_HEAP_STRUCT_SIZE]; } test_struct;
  xtc_protect_t protect = { .lock = lock, .unlock = unlock };
  size_t length = XSH_HEAP_LENGTH(test_struct,TEST_HEAP_SIZE);
  char* pool = malloc(length);
  memset(test_heap(), 0, sizeof(xsh_heap_t));
  xsh_init(test_heap(), pool, length, sizeof(test_struct), &protect);
  reset_protect();
  return 0;
}

static int cleanup_protected_heap() {
  reset_protect();
  void *pool = test_heap()->mem_pool;
  memset(test_heap(), 0, sizeof(xsh_heap_t));
  free(pool);
  return 0;
}

static void simple_test_protect() {
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  void *ptr = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
  CU_ASSERT_PTR_NOT_NULL(ptr);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE - 1);
  CU_ASSERT_EQUAL(lock_called, 2);
  CU_ASSERT_EQUAL(unlock_called, 2);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  xsh_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
  CU_ASSERT_EQUAL(lock_called, 2);
  CU_ASSERT_EQUAL(unlock_called, 2);
  reset_protect();
}

static void multiple_test_protect() {
  void* arr[TEST_HEAP_SIZE];
  memset(arr, 0, sizeof(arr));
  int i;
  reset_protect();
  for(i = 0; i < TEST_HEAP_SIZE; i++) {
    arr[i] = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), 0);
  CU_ASSERT_EQUAL(lock_called, i+1);
  CU_ASSERT_EQUAL(unlock_called, i+1);
  reset_protect();
  for(i = 0; i < TEST_HEAP_SIZE; i++) {
    CU_ASSERT_PTR_NULL(xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE));
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), 0);
  CU_ASSERT_EQUAL(lock_called, i+1);
  CU_ASSERT_EQUAL(unlock_called, i+1);
  reset_protect();
  for(i = 0; i < TEST_HEAP_SIZE; i++) {
    xsh_free(test_heap(), arr[i]);
    arr[i] = NULL;
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), i);
  CU_ASSERT_EQUAL(lock_called, i+1);
  CU_ASSERT_EQUAL(unlock_called, i+1);
  reset_protect();
}

static void error_test_protect() {
  reset_protect();

  xsh_heap_t heap;
  CU_ASSERT_PTR_NULL(xsh_alloc(&heap, 42));
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  CU_ASSERT_PTR_NULL(xsh_alloc((xsh_heap_t*)(((char*)(&heap)) + 2), 42));
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  CU_ASSERT_PTR_NULL(xsh_alloc((xsh_heap_t*)test_heap()->mem_pool, 42));
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  CU_ASSERT_PTR_NULL(xsh_alloc(test_heap(), 0));
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  CU_ASSERT_PTR_NULL(xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE - 1));
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  CU_ASSERT_PTR_NULL(xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE + 1));
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  xsh_free(NULL, (void*)42);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xsh_free(test_heap(), NULL);
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  int bullshit;
  xsh_free(test_heap(), &bullshit);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  void* ptr = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
  xsh_free(test_heap(), (void*)((char*)ptr - 1));
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xsh_free(test_heap(), (void*)((char*)ptr + 1));
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xsh_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  cleanup_heap(); init_heap();
  ptr = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xsh_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xsh_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  char buf[TEST_HEAP_STRUCT_SIZE];
  xsh_free(test_heap(), (void*)buf);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  // Build a fake node with real data but not allocated with structure heap
  xsh_node_t *test = malloc(test_heap()->node_size);
  ptr = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xsh_node_t *node = (xsh_node_t*)ptr - 1;
  memcpy(test, node, test_heap()->node_size);
  xsh_free(test_heap(), (void*)(test+1));
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xsh_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  free(test);

  reset_protect();
}

static void test_perf() {
  void* arr[TEST_HEAP_SIZE];
  memset(arr, 0, sizeof(arr));
  srand(time(NULL));
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
  int na = 0, nf = 0, last = 0;
  clock_t sa = 0, sf = 0; 
  printf("\n ===== %03d%% : ", last);
  for (int i = 0; i < TEST_PERFORMANCE_ROUND; i++) {
    int adv = (i * 100) / TEST_PERFORMANCE_ROUND;
    if (last != adv) {
      last = adv;
      printf("\r ===== %03d%% : ", last);
    }
    int id = rand() % TEST_HEAP_SIZE;
    if (arr[id]) {
      size_t count = xsh_free_count(test_heap());
      clock_t start = clock();
      xsh_free(test_heap(), arr[id]);
      sf += clock() - start;
      arr[id] = NULL;
      nf++;
      CU_ASSERT_EQUAL(xsh_free_count(test_heap()), count + 1);
    } else {
      clock_t start = clock();
      arr[id] = xsh_alloc(test_heap(), TEST_HEAP_STRUCT_SIZE);
      sa += clock() - start;
      na++;
      CU_ASSERT_PTR_NOT_NULL(arr[id]);
    }
  }
  printf("\r ===== 100%% : ");
  for (int i = 0; i < TEST_HEAP_SIZE; i++) {
    if (arr[i]) {
      clock_t start = clock();
      xsh_free(test_heap(), arr[i]);
      sf += clock() - start;
      arr[i] = NULL;
      nf++;
    }
  }
  CU_ASSERT_EQUAL(xsh_free_count(test_heap()), TEST_HEAP_SIZE);
  CU_ASSERT_EQUAL(na, nf);
  double avga, avgf;
  avga = (double)sa * 1e9 / CLOCKS_PER_SEC / na;
  avgf = (double)sf * 1e9 / CLOCKS_PER_SEC / nf;
  printf("#alloc. = %.3lf ns [%d] #free. = %.3lf ns [%d] : ...", avga, na, avgf, nf);
}
