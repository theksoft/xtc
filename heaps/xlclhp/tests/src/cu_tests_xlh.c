#include "cu_tests.h"
#include "xtc_lclhp.h"
#include "cxx_tests_xlh.h"
#include <CUnit/CUnit.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#define ASSERT_EQUAL_STATS(a, b)  \
  do {  \
    CU_ASSERT_EQUAL(a.count, b.count); \
    CU_ASSERT_EQUAL(a.max_block_size, b.max_block_size); \
    CU_ASSERT_EQUAL(a.total_size, b.total_size); \
  } while(0)

#define TEST_PERFORMANCE_ROUND    10000000

static test_suite_t* get_ts_init_end_error();
static test_suite_t* get_ts_init_end();
static test_suite_t* get_ts_allocation_errors();
static test_suite_t* get_ts_operational();
static test_suite_t* get_ts_free_errors();
static test_suite_t* get_ts_fragmentation();
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
  get_ts_fragmentation,
  get_ts_end,
  get_ts_protect,
  get_ts_random,
  get_ts_cxx_allocator,
  get_ts_cxx_protected_allocator,
  TEST_SUITE_GETTER_END
};

static void init_error_heap_mem();
static void init_error_sizes();
static void init_error_protect();
static void end_error();

static test_suite_t* get_ts_init_end_error() {

  static test_t tests[] = {
    { "Init with bad heap / mem parameters", init_error_heap_mem },
    { "Init with bad pool or struct size parameters", init_error_sizes },
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

static void init_static();
static void init_dynamic();

static test_suite_t* get_ts_init_end() {

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

static void frag_head();
static void frag_tail();
static void frag_next();
static void frag_previous();
static void frag_both();
static void frag_more();
static void frag_more2();
static void frag_failing();

static test_suite_t* get_ts_fragmentation() {

  static test_t tests[] = {
    { "Fragmentation holes at block head", frag_head },
    { "Fragmentation at block tail", frag_tail },
    { "Free joining with next", frag_next },
    { "Free joining with previous", frag_previous },
    { "Free joining both next and previous", frag_both },
    { "More Free joinings", frag_more },
    { "More Free joinings", frag_more2 },
    { "Failed allocation due to fragmentation", frag_failing },
    { NULL, NULL }
  };

  static test_suite_t suite = {
    .title = "Fragmentation checks & errors",
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

static int init_perf_heap();
static int cleanup_perf_heap();

static void test_perf();

static test_suite_t* get_ts_random() {

  static test_t tests[] = {
    { "Test random && performance", test_perf },
    { NULL, NULL }
  };

  static test_suite_t suite = {
    .title = "Randomized allocation and free operations",
    .init = init_perf_heap,
    .cleanup = cleanup_perf_heap,
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
    { "[C++] Fragmentation holes at block head", cxx_frag_head },
    { "[C++] Fragmentation at block tail", cxx_frag_tail },
    { "[C++] Free joining with next", cxx_frag_next },
    { "[C++] Free joining with previous", cxx_frag_previous },
    { "[C++] Free joining both next and previous", cxx_frag_both },
    { "[C++] More Free joinings", cxx_frag_more },
    { "[C++] More Free joinings", cxx_frag_more2 },
    { "[C++] Failed allocation due to fragmentation", cxx_frag_failing },
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
  xlh_heap_t heap;
  CU_ASSERT_PTR_NULL(xlh_init(NULL, NULL, 10000, &guard));
  CU_ASSERT_PTR_NULL(xlh_init(NULL, &pool, 10000, &guard));
  CU_ASSERT_PTR_NULL(xlh_init(NULL, &pool, 10000, NULL));
  CU_ASSERT_PTR_NULL(xlh_init(&heap, NULL, 10000, &guard));
  CU_ASSERT_PTR_NULL(xlh_init(&heap, NULL, 10000, NULL));
}

static void init_error_sizes() {
  int pool;
  xtc_protect_t guard = { .lock = NULL, .unlock = NULL };
  xlh_heap_t heap;
  CU_ASSERT_PTR_NULL(xlh_init(&heap, &pool, 512, &guard));
  CU_ASSERT_PTR_NULL(xlh_init(&heap, &pool, 0, &guard));
  CU_ASSERT_PTR_NULL(xlh_init(&heap, &pool, 0, NULL));
  CU_ASSERT_PTR_NULL(xlh_init(&heap, &pool, sizeof(xlh_node_t) - 1, &guard));
  CU_ASSERT_PTR_NULL(xlh_init(&heap, &pool, sizeof(xlh_node_t) - 1, NULL));
  CU_ASSERT_PTR_NULL(xlh_init(&heap, &pool, sizeof(xlh_node_t), &guard));
  CU_ASSERT_PTR_NULL(xlh_init(&heap, &pool, sizeof(xlh_node_t), NULL));
}

static void dummy() {}

static void init_error_protect() {
  int pool;
  xlh_heap_t heap;
  xtc_protect_t guard = { .lock = NULL, .unlock = NULL };
  CU_ASSERT_PTR_NULL(xlh_init(&heap, &pool, 512, &guard));
  guard.lock = dummy;
  CU_ASSERT_PTR_NULL(xlh_init(&heap, &pool, 512, &guard));
  guard.lock = NULL;
  guard.unlock = dummy;
  CU_ASSERT_PTR_NULL(xlh_init(&heap, &pool, 512, &guard));
}

static void end_error() {
  xlh_heap_t heap;
  xlh_stats_t stats;
  CU_ASSERT_PTR_NULL(xlh_end(NULL, NULL));
  CU_ASSERT_PTR_NULL(xlh_end(NULL, &stats));
  CU_ASSERT_PTR_NULL(xlh_end(&heap, &stats));
  CU_ASSERT_PTR_NULL(xlh_end(&heap, NULL));
}

static void init_static() {
  size_t length = 1024*1024, init_length = length - sizeof(xlh_node_t);
  char pool[length];
  xlh_heap_t heap, ref0;
  xtc_protect_t guard = { .lock = dummy, .unlock = dummy };
  xlh_stats_t stats;
  memset(&ref0, 0, sizeof(xlh_heap_t));

  memset(&heap, 0, sizeof(xlh_heap_t));
  CU_ASSERT_PTR_NOT_NULL(xlh_init(&heap, &pool, length, &guard));
  CU_ASSERT_EQUAL(xlh_max_free_blk(&heap), init_length);
  xlh_free_stats(&heap, &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.total_size, init_length);
  CU_ASSERT_EQUAL(stats.max_block_size, init_length);
  memset(&stats, 0, sizeof(xlh_stats_t));
  xlh_allocated_stats(&heap, &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);

  CU_ASSERT_PTR_EQUAL(xlh_end(&heap, NULL), pool);
  CU_ASSERT_EQUAL(0, memcmp(&heap, &ref0, sizeof(xlh_heap_t)));

  CU_ASSERT_PTR_NOT_NULL(xlh_init(&heap, &pool, length, NULL));
  CU_ASSERT_EQUAL(xlh_max_free_blk(&heap), init_length);
  xlh_free_stats(&heap, &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.total_size, init_length);
  CU_ASSERT_EQUAL(stats.max_block_size, init_length);
  memset(&stats, 0, sizeof(xlh_stats_t));
  xlh_allocated_stats(&heap, &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);

  CU_ASSERT_PTR_EQUAL(xlh_end(&heap, NULL), pool);
  CU_ASSERT_EQUAL(0, memcmp(&heap, &ref0, sizeof(xlh_heap_t)));
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
}

static void init_dynamic() {
  size_t length = 512, init_length = length - sizeof(xlh_node_t);
  char *pool = (char*)malloc(length);
  xlh_heap_t heap, ref0;
  xtc_protect_t guard = { .lock = dummy, .unlock = dummy };
  xlh_stats_t stats;
  memset(&ref0, 0, sizeof(xlh_heap_t));

  memset(&heap, 0, sizeof(xlh_heap_t));
  CU_ASSERT_PTR_NOT_NULL(xlh_init(&heap, pool, length, &guard));
  CU_ASSERT_EQUAL(xlh_max_free_blk(&heap), init_length);
  xlh_free_stats(&heap, &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.total_size, init_length);
  CU_ASSERT_EQUAL(stats.max_block_size, init_length);
  memset(&stats, 0, sizeof(xlh_stats_t));
  xlh_allocated_stats(&heap, &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);

  CU_ASSERT_PTR_EQUAL(xlh_end(&heap, NULL), pool);
  CU_ASSERT_EQUAL(0, memcmp(&heap, &ref0, sizeof(xlh_heap_t)));

  CU_ASSERT_PTR_NOT_NULL(xlh_init(&heap, pool, length, NULL));
  CU_ASSERT_EQUAL(xlh_max_free_blk(&heap), init_length);
  xlh_free_stats(&heap, &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.total_size, init_length);
  CU_ASSERT_EQUAL(stats.max_block_size, init_length);
  memset(&stats, 0, sizeof(xlh_stats_t));
  xlh_allocated_stats(&heap, &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);

  CU_ASSERT_PTR_EQUAL(xlh_end(&heap, &stats), pool);
  CU_ASSERT_EQUAL(0, memcmp(&heap, &ref0, sizeof(xlh_heap_t)));
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);

  free(pool);
}

#define TEST_HEAP_OPT_COUNT     8
#define TEST_HEAP_SIZE          (TEST_HEAP_OPT_COUNT*(sizeof(xlh_node_t) + 16))

static xlh_heap_t* test_heap() {
  static xlh_heap_t heap;
  return &heap;
}

static int init_heap() {
  char* pool = malloc(TEST_HEAP_SIZE);
  memset(test_heap(), 0, sizeof(xlh_heap_t));
  xlh_init(test_heap(), pool, TEST_HEAP_SIZE, NULL);
  return 0;
}

static int cleanup_heap() {
  xlh_stats_t stats;
  void *pool = xlh_end(test_heap(), &stats);
  assert(!stats.count);
  free(pool);
  return 0;
}

static void alloc_null_heap() {
  CU_ASSERT_PTR_NULL(xlh_alloc(NULL, 42));
}

static void alloc_bad_heap() {
  xlh_heap_t heap;
  /* Uninitialized heap */
  CU_ASSERT_PTR_NULL(xlh_alloc(&heap, 42));
  /* Invalid address */
  CU_ASSERT_PTR_NULL(xlh_alloc((xlh_heap_t*)(((char*)(&heap)) + 2), 42));
  CU_ASSERT_PTR_NULL(xlh_alloc((xlh_heap_t*)test_heap()->mem_pool, 42));
}

static void alloc_0_size() {
  size_t size = xlh_max_free_blk(test_heap());
  CU_ASSERT_PTR_NULL(xlh_alloc(test_heap(), 0));
  CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), size);
}

static void alloc_bad_size() {
  size_t size = xlh_max_free_blk(test_heap());
  CU_ASSERT_PTR_NULL(xlh_alloc(test_heap(), TEST_HEAP_SIZE - 1));
  CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), size);
  CU_ASSERT_PTR_NULL(xlh_alloc(test_heap(), TEST_HEAP_SIZE + 1));
  CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), size);
}

static void simple_alloc_free() {
  size_t lninit = TEST_HEAP_SIZE - sizeof(xlh_node_t);
  CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), lninit);
  void *ptr = xlh_alloc(test_heap(), 12);
  CU_ASSERT_PTR_NOT_NULL(ptr);
  CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), lninit - XTC_ALIGNED_SIZE(12) - sizeof(xlh_node_t));
  xlh_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), lninit);
  for (size_t ln = 20, lnoff = 0; lnoff < sizeof(int) + 1; lnoff++) {
    ptr = xlh_alloc(test_heap(), ln+lnoff);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), lninit - XTC_ALIGNED_SIZE(ln+lnoff) - sizeof(xlh_node_t));
    xlh_free(test_heap(), ptr);
    CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), lninit);
  }
}

static void sequential_alloc_free() {
  size_t lninit = TEST_HEAP_SIZE - sizeof(xlh_node_t);
  size_t szalloc = 14, szblk = XTC_ALIGNED_SIZE(szalloc);
  size_t n = TEST_HEAP_SIZE / (szblk + sizeof(xlh_node_t));
  assert(n == TEST_HEAP_OPT_COUNT);
  // If not the case some assert are not correct as the last block can be different
  void* arr[n];
  memset(arr, 0, sizeof(arr));
  size_t i;
  // Allocate all sequentially
  size_t expected_size = lninit;
  for (i = 0; i < n; i++) {
    CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), expected_size);
    arr[i] = xlh_alloc(test_heap(), szalloc);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
    expected_size -= sizeof(xlh_node_t) + szblk;
  }
  xlh_stats_t stats;
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, n);
  CU_ASSERT_EQUAL(stats.max_block_size, szblk);
  CU_ASSERT_EQUAL(stats.total_size, n*szblk);
  // Check there is really no more space
  CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), 0);
  CU_ASSERT_PTR_NULL(xlh_alloc(test_heap(), szalloc));
  CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), 0);
  // Free all blocks
  expected_size = 0;
  for (i = 0; i < n; i++) {
    CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), expected_size);
    xlh_free(test_heap(), arr[i]);
    if (expected_size) {
      expected_size += sizeof(xlh_node_t);
    }
    expected_size += szblk;
  }
  CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), lninit);
}

static void unordered_alloc_free() {
  size_t lninit = TEST_HEAP_SIZE - sizeof(xlh_node_t);
  size_t szalloc = 3, szblk = XTC_ALIGNED_SIZE(szalloc);
  size_t n = TEST_HEAP_SIZE / (szblk + sizeof(xlh_node_t));
  size_t szlast = lninit - (n-1)*(szblk + sizeof(xlh_node_t));
  void* arr[n];
  memset(arr, 0, sizeof(arr));
  size_t i;
  // Alloc all
  xlh_stats_t stats;
  size_t expected_size = lninit;
  for (i = 0; i < n; i++) {
    CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), expected_size);
    arr[i] = xlh_alloc(test_heap(), szalloc);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
    expected_size -= expected_size > szblk ? sizeof(xlh_node_t) + szblk : szblk;
  }
  // Free 1 over 2
  xlh_free_stats(test_heap(), &stats);
  size_t count = stats.count;
  CU_ASSERT_EQUAL(count, 0);
  for (i = 0; i < n; i+=2) {
    xlh_free_stats(test_heap(), &stats);
    CU_ASSERT_EQUAL(stats.count, count++);
    CU_ASSERT_EQUAL(stats.max_block_size, stats.count ? szblk : 0);
    CU_ASSERT_EQUAL(stats.total_size, stats.count*szblk);
    xlh_free(test_heap(), arr[i]);
    arr[i] = NULL;
  }
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, count);
  CU_ASSERT_EQUAL(stats.max_block_size, (n%2) ? szlast : szblk);
  CU_ASSERT_EQUAL(stats.total_size, (count-1)*szblk + ((n%2) ? szlast : szblk));
  // Alloc again half of the previously freed
  for (i = 0; i < n; i+=4) {
    assert(!arr[i]);
    xlh_free_stats(test_heap(), &stats);
    CU_ASSERT_EQUAL(stats.count, count--);
    CU_ASSERT_EQUAL(stats.max_block_size, (n%2) ? szlast : szblk);
    CU_ASSERT_EQUAL(stats.total_size, (stats.count-1)*szblk + ((n%2) ? szlast : szblk));
    arr[i] = xlh_alloc(test_heap(), szalloc);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  for (i = 0; count; ) {
    while(arr[i]) i++;
    assert(i < n);
    xlh_free_stats(test_heap(), &stats);
    CU_ASSERT_EQUAL(stats.count, count--);
    CU_ASSERT_EQUAL(stats.max_block_size, (n%2) ? szlast : szblk);
    CU_ASSERT_EQUAL(stats.total_size, (stats.count-1)*szblk + ((n%2) ? szlast : szblk));
    arr[i] = xlh_alloc(test_heap(), szalloc);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, n);
  CU_ASSERT_EQUAL(stats.max_block_size, (n%2) ? szlast : szblk);
  CU_ASSERT_EQUAL(stats.total_size, (n-1)*szblk + ((n%2) ? szlast : szblk));
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
  CU_ASSERT_PTR_NULL(xlh_alloc(test_heap(), szalloc));
  for (i = 0; i < n; i++) {
    if (!arr[i]) continue;
    count++;
  }
  CU_ASSERT_EQUAL(count, n);
  for (i = 0; i < n; i++) {
    xlh_allocated_stats(test_heap(), &stats);
    CU_ASSERT_EQUAL(stats.count, n-i);
    xlh_free(test_heap(), arr[i]);
    arr[i] = NULL;
  }
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, lninit);
  CU_ASSERT_EQUAL(stats.total_size, lninit);
}

static void free_null() {
  xlh_stats_t ini, ref, aref, stats;
  xlh_free_stats(test_heap(), &ini);
  void* ptr = xlh_alloc(test_heap(), TEST_HEAP_SIZE / 2);
  xlh_free_stats(test_heap(), &ref);
  xlh_allocated_stats(test_heap(), &aref);

  xlh_free(NULL, (void*)42);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ref);
  xlh_allocated_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, aref);

  xlh_free(test_heap(), NULL);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ref);
  xlh_allocated_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, aref);

  xlh_free(test_heap(), ptr);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ini);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
}

static void free_invalid() {
  xlh_stats_t ini, ref, aref, stats;
  xlh_free_stats(test_heap(), &ini);
  int bullshit;

  xlh_free(test_heap(), &bullshit);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ini);

  void* ptr = xlh_alloc(test_heap(), TEST_HEAP_SIZE / 3);
  xlh_free_stats(test_heap(), &ref);
  CU_ASSERT_EQUAL(ref.count, 1);
  CU_ASSERT_EQUAL(ref.max_block_size, ini.max_block_size - sizeof(xlh_node_t) - XTC_ALIGNED_SIZE(TEST_HEAP_SIZE/3));
  CU_ASSERT_EQUAL(ref.max_block_size, ref.total_size);
  xlh_allocated_stats(test_heap(), &aref);
  CU_ASSERT_EQUAL(aref.count, 1);
  CU_ASSERT_EQUAL(aref.max_block_size, XTC_ALIGNED_SIZE(TEST_HEAP_SIZE/3));
  CU_ASSERT_EQUAL(aref.total_size, XTC_ALIGNED_SIZE(TEST_HEAP_SIZE/3));

  xlh_free(test_heap(), (void*)((char*)ptr - 1));
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ref);
  xlh_allocated_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, aref);

  xlh_free(test_heap(), (void*)((char*)ptr + 1));
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ref);
  xlh_allocated_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, aref);

  xlh_free(test_heap(), ptr);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ini);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
}

static void free_double() {
  cleanup_heap(); init_heap();
  xlh_stats_t ini, aref, stats;
  xlh_free_stats(test_heap(), &ini);
  CU_ASSERT_EQUAL(ini.count, 1);
  xlh_allocated_stats(test_heap(), &aref);
  CU_ASSERT_EQUAL(aref.count, 0);
  void* ptr = xlh_alloc(test_heap(), TEST_HEAP_SIZE/2);
  // free
  xlh_free(test_heap(), ptr);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ini);
  xlh_allocated_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, aref);
  // Double free
  xlh_free(test_heap(), ptr);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ini);
  xlh_allocated_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, aref);
}

static void free_not_allocated() {
  xlh_stats_t ini, ref, aref, stats;
  xlh_free_stats(test_heap(), &ini);
  CU_ASSERT_EQUAL(ini.count, 1);
  xlh_allocated_stats(test_heap(), &aref);
  CU_ASSERT_EQUAL(aref.count, 0);

  char buf[42];
  xlh_free(test_heap(), (void*)buf);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ini);
  xlh_allocated_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, aref);

  // Build a fake node with real data but not allocated with structure heap
  void *ptr = xlh_alloc(test_heap(), 16);
  xlh_free_stats(test_heap(), &ref);
  xlh_allocated_stats(test_heap(), &aref);

  size_t sznode = sizeof(xlh_node_t) + 16;
  xlh_node_t *test = malloc(sznode);
  xlh_node_t *node = (xlh_node_t*)ptr - 1;
  memcpy(test, node, sznode);

  xlh_free(test_heap(), (void*)(test+1));
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ref);
  xlh_allocated_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, aref);

  xlh_free(test_heap(), ptr);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ini);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);

  free(test);
}

static void free_sys() {
/*
  => result is (e.g. it works!):
  Test: Free and system free ...free(): invalid pointer
Aborted
*/
#if 0
  xlh_stats_t ini, ref, stats;
  xlh_free_stats(test_heap(), &ini);
  void* ptr = xlh_alloc(test_heap(), TEST_HEAP_SIZE / 4);
  xlh_free_stats(test_heap(), &ref);
  // First node of the allocated memory
  // -- Should not be freed because of xlh_node_t header
  free(ptr);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ref);
  xlh_free(test_heap(), ptr);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ini);
#endif
}

static void free_wrong() {
  xlh_stats_t ini1, ref1, aref1, ini2, ref2, aref2, stats;
  xlh_free_stats(test_heap(), &ini1);
  CU_ASSERT_EQUAL(ini1.count, 1);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);

  // Declare a second heap
  size_t length = 1024;
  char* pool = malloc(length);
  xlh_heap_t heap2;
  memset(&heap2, 0, sizeof(xlh_heap_t));
  xlh_init(&heap2, pool, length, NULL);
  xlh_free_stats(&heap2, &ini2);
  CU_ASSERT_EQUAL(ini2.count, 1);
  xlh_allocated_stats(&heap2, &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  // Allocate on each heap
  void* ptr1 = xlh_alloc(test_heap(), TEST_HEAP_SIZE / 8);
  xlh_free_stats(test_heap(), &ref1);
  xlh_allocated_stats(test_heap(), &aref1);
  CU_ASSERT_EQUAL(aref1.count, 1);
  void* ptr2 = xlh_alloc(&heap2, length/3);
  xlh_free_stats(&heap2, &ref2);
  xlh_allocated_stats(&heap2, &aref2);
  CU_ASSERT_EQUAL(aref2.count, 1);

  // Free each pointer on the bad heap

  xlh_free(&heap2, ptr1);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ref1);
  xlh_allocated_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, aref1);
  xlh_free_stats(&heap2, &stats);
  ASSERT_EQUAL_STATS(stats, ref2);
  xlh_allocated_stats(&heap2, &stats);
  ASSERT_EQUAL_STATS(stats, aref2);

  xlh_free(test_heap(), ptr2);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ref1);
  xlh_allocated_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, aref1);
  xlh_free_stats(&heap2, &stats);
  ASSERT_EQUAL_STATS(stats, ref2);
  xlh_allocated_stats(&heap2, &stats);
  ASSERT_EQUAL_STATS(stats, aref2);

  // Correct free
  xlh_free(&heap2, ptr2);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ref1);
  xlh_allocated_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, aref1);
  xlh_free_stats(&heap2, &stats);
  ASSERT_EQUAL_STATS(stats, ini2);
  xlh_allocated_stats(&heap2, &stats);
  CU_ASSERT_EQUAL(stats.count, 0);

  xlh_free(test_heap(), ptr1);
  xlh_free_stats(test_heap(), &stats);
  ASSERT_EQUAL_STATS(stats, ini1);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  xlh_free_stats(&heap2, &stats);
  ASSERT_EQUAL_STATS(stats, ini2);
  xlh_allocated_stats(&heap2, &stats);
  CU_ASSERT_EQUAL(stats.count, 0);

  memset(&heap2, 0, sizeof(xlh_heap_t));
  free(pool);
}

static void frag_allocate_4(void* ptr[], int sz[]) {
  // Compute and check sizes
  size_t n = 4;
  size_t szmeanalloc = (TEST_HEAP_SIZE - (n * sizeof(xlh_node_t))) / n;
  assert(XTC_ALIGNED_SIZE(szmeanalloc) == szmeanalloc);
  size_t remain = TEST_HEAP_SIZE - (n * (sizeof(xlh_node_t) + szmeanalloc));
  assert(szmeanalloc > 40);
  assert(!remain); (void)remain;

  // Allocate
  int mean = 0, mean2 = 0;
  for (size_t i = 0; i < n; i++) {
    mean += sz[i];
    sz[i] += szmeanalloc;
    assert(sz[i] > 0);
    mean2 += sz[i];
  }
  assert(0 == mean);
  assert((int)n*(int)szmeanalloc == mean2);
  ptr[0] = xlh_alloc(test_heap(), sz[0]);
  ptr[1] = xlh_alloc(test_heap(), sz[1]);
  ptr[2] = xlh_alloc(test_heap(), sz[2]);
  ptr[3] = xlh_alloc(test_heap(), sz[3]);
  assert(ptr[0] && ptr[1] && ptr[2] && ptr[3]);

  // Check
  xlh_stats_t stats;
  xlh_free_stats(test_heap(), &stats);
  assert(stats.count == 0);
  xlh_allocated_stats(test_heap(), &stats);
  assert(stats.count == 4);
  assert(stats.total_size == 4 * szmeanalloc);
}

static void frag_head() {
  // Make 4 allocations so that all memory is used
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[4] = { 0, 9*base, -3*base, -6*base };
  void* ptr[4];
  frag_allocate_4(ptr, size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);

  // Free the first
  xlh_free(test_heap(), ptr[0]); ptr[0] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[0]));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[1] + size[2] + size[3]));

  // Free the second
  xlh_free(test_heap(), ptr[1]); ptr[1] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[0]+size[1]) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[2]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[2] + size[3]));

  // Free the third
  xlh_free(test_heap(), ptr[2]); ptr[2] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[0]+size[1]+size[2]) + 2*sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[3]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);

  // Free the fourth one
  xlh_free(test_heap(), ptr[3]); ptr[3] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
}

static void frag_tail() {
  // Make 4 allocations so that all memory is used
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[4] = { 0, 9*base, -3*base, -6*base };
  void* ptr[4];
  frag_allocate_4(ptr, size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);

  // Free the fourth one
  xlh_free(test_heap(), ptr[3]); ptr[3] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[3]));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[1] + size[2]));

  // Free the third one
  xlh_free(test_heap(), ptr[2]); ptr[2] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[2] + size[3]) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[1]));

  // Free the second one
  xlh_free(test_heap(), ptr[1]); ptr[1] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[1] + size[2] + size[3]) + 2*sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);

  // Free the first one
  xlh_free(test_heap(), ptr[0]); ptr[0] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
}

static void frag_next() {
  // Make 4 allocations so that all memory is used and
  // with the second being the largest one being able to hold a future node
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[4] = { 0, 16*base, -6*base, -10*base };
  assert((size_t)size[1] > sizeof(xlh_node_t) + 2*base);
  void* ptr[4];
  frag_allocate_4(ptr, size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);

  // Free the second
  xlh_free(test_heap(), ptr[1]); ptr[1] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[3]));

  // Allocate a size that will split the second block
  ptr[1] = xlh_alloc(test_heap(), sizeof(int));
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1] - sizeof(int) - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 4);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[3] + sizeof(int)));

  // Free the second so that it joined the split block
  xlh_free(test_heap(), ptr[1]); ptr[1] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[3]));

  // Free the third
  xlh_free(test_heap(), ptr[2]); ptr[2] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[1] + size[2]) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[3]));

  // Free the fourth
  xlh_free(test_heap(), ptr[3]); ptr[3] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[1] + size[2] + size[3]) + 2*sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);

  // Free the first
  xlh_free(test_heap(), ptr[0]); ptr[0] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
}

static void frag_previous() {
  // Make 4 allocations so that all memory is used and
  // with the second being the largest one being able to hold a future node
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[4] = { 0, 16*base, -6*base, -10*base };
  assert((size_t)size[1] > sizeof(xlh_node_t) + 2*base);
  void* ptr[4];
  frag_allocate_4(ptr, size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);

  // Free the second
  xlh_free(test_heap(), ptr[1]); ptr[1] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[3]));

  // Allocate a size that will split the second block
  ptr[1] = xlh_alloc(test_heap(), sizeof(int));
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1] - sizeof(int) - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 4);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[3] + sizeof(int)));

  // Free the third one so the it joined the remain of the second one
  xlh_free(test_heap(), ptr[2]); ptr[2] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[1] + size[2]) - sizeof(int));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[3] + sizeof(int)));

  // Free the fourth
  xlh_free(test_heap(), ptr[3]); ptr[3] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[1] + size[2] + size[3]) - sizeof(int) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + sizeof(int)));

  // Free the second
  xlh_free(test_heap(), ptr[1]); ptr[1] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[1] + size[2] + size[3]) + 2*sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);

  // Free the first
  xlh_free(test_heap(), ptr[0]); ptr[0] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
}

static void frag_allocate_5(void* ptr[], int sz[]) {
  // Compute and check sizes
  size_t n = 5;
  size_t szmeanalloc = (TEST_HEAP_SIZE - (n * sizeof(xlh_node_t))) / n;
  szmeanalloc = XTC_ALIGNED_SIZE(szmeanalloc) - sizeof(int);
  assert(szmeanalloc > 40);
  int remain = TEST_HEAP_SIZE - (n * (sizeof(xlh_node_t) + szmeanalloc));
  assert(remain > 0);
  sz[1] += remain;

  // Allocate
  int mean = 0, mean2 = 0;
  for (size_t i = 0; i < n; i++) {
    mean += sz[i];
    sz[i] += szmeanalloc;
    assert(sz[i] > 0);
    mean2 += sz[i];
  }
  assert(remain == mean);
  assert((int)n*(int)szmeanalloc + remain == mean2);
  ptr[0] = xlh_alloc(test_heap(), sz[0]);
  ptr[1] = xlh_alloc(test_heap(), sz[1]);
  ptr[2] = xlh_alloc(test_heap(), sz[2]);
  ptr[3] = xlh_alloc(test_heap(), sz[3]);
  ptr[4] = xlh_alloc(test_heap(), sz[4]);
  assert(ptr[0] && ptr[1] && ptr[2] && ptr[3] && ptr[4]);

  // Check
  xlh_stats_t stats;
  xlh_free_stats(test_heap(), &stats);
  assert(stats.count == 0);
  xlh_allocated_stats(test_heap(), &stats);
  assert(stats.count == n);
  assert(stats.total_size == n * szmeanalloc + remain);
}

static void frag_both() {
  // Make 5 allocations so that all memory is used
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[5] = { 0, 9*base, -6*base, 2*base, -5*base };
  void* ptr[5];
  frag_allocate_5(ptr, size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);

  // Free the second and the fourth
  xlh_free(test_heap(), ptr[1]); ptr[1] = NULL;
  xlh_free(test_heap(), ptr[3]); ptr[3] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[1] + size[3]));
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[4]));

  // Free the third so that it joined the second and fourth
  xlh_free(test_heap(), ptr[2]); ptr[2] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[1] + size[2] + size[3]) + 2*sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[4]));

  // Free the first
  xlh_free(test_heap(), ptr[0]); ptr[0] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[0] + size[1] + size[2] + size[3]) + 3*sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[4]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);

  // Free the fifth
  xlh_free(test_heap(), ptr[4]); ptr[4] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
}

static void frag_more() {
  // Make 5 allocations so that all memory is used
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[5] = { 0, 9*base, -6*base, 2*base, -5*base };
  void* ptr[5];
  frag_allocate_5(ptr, size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);

  // Free the second and the fourth
  xlh_free(test_heap(), ptr[1]); ptr[1] = NULL;
  xlh_free(test_heap(), ptr[3]); ptr[3] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[1] + size[3]));
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[4]));

  // Free first
  xlh_free(test_heap(), ptr[0]); ptr[0] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[0] + size[1]) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[1] + size[3]) + sizeof(xlh_node_t));
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[4]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[2] + size[4]));

  // Free fifth
  xlh_free(test_heap(), ptr[4]); ptr[4] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[0] + size[1]) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[1] + size[3] + size[4]) + 2*sizeof(xlh_node_t));
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[2]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);

  // Free third
  xlh_free(test_heap(), ptr[2]); ptr[2] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
}

static void frag_more2() {
  // Make 5 allocations so that all memory is used
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[5] = { 0, -5*base, -6*base, 9*base, 2*base };
  void* ptr[5];
  frag_allocate_5(ptr, size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[3]);

  // Free the second and the fourth
  xlh_free(test_heap(), ptr[1]); ptr[1] = NULL;
  xlh_free(test_heap(), ptr[3]); ptr[3] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[3]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[1] + size[3]));
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[4]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[4]));

  // Free fifth
  xlh_free(test_heap(), ptr[4]); ptr[4] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[3] + size[4]) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[1] + size[3] + size[4]) + sizeof(xlh_node_t));
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2]));

  // Free first
  xlh_free(test_heap(), ptr[0]); ptr[0] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[3] + size[4]) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[1] + size[3] + size[4]) + 2*sizeof(xlh_node_t));
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[2]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);

  // Free third
  xlh_free(test_heap(), ptr[2]); ptr[2] = NULL;
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
}

static void frag_failing() {
  // Make 5 allocations so that all memory is used
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[5] = { 0, 9*base, -6*base, 2*base, -5*base };
  void* ptr[5];
  frag_allocate_5(ptr, size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);

  // Free the second and the fourth
  xlh_free(test_heap(), ptr[1]); ptr[1] = NULL;
  xlh_free(test_heap(), ptr[3]); ptr[3] = NULL;

  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[1] + size[3]));
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[4]));

  // Try allocate size greater than second and fourth but less than the sum of them
  CU_ASSERT((size_t)size[1]+1 < stats.total_size);
  CU_ASSERT_EQUAL(xlh_alloc(test_heap(), size[1]+1), NULL);

  // Free all
  xlh_free(test_heap(), ptr[0]); ptr[0] = NULL;
  xlh_free(test_heap(), ptr[4]); ptr[4] = NULL;
  xlh_free(test_heap(), ptr[2]); ptr[2] = NULL;

  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
}

static void end_none() {
  size_t length = 1024*1024, init_length = length - sizeof(xlh_node_t);
  char pool[length];
  xlh_heap_t heap, ref0;
  xlh_stats_t stats;
  memset(&ref0, 0, sizeof(xlh_heap_t));
  void* ptr[3];

  memset(&heap, 0, sizeof(xlh_heap_t));
  CU_ASSERT_PTR_NOT_NULL(xlh_init(&heap, &pool, length, NULL));
  ptr[0] = xlh_alloc(&heap, 200);
  ptr[1] = xlh_alloc(&heap, 50000);
  ptr[2] = xlh_alloc(&heap, 100);
  CU_ASSERT_PTR_NOT_NULL(ptr[0]);
  CU_ASSERT_PTR_NOT_NULL(ptr[1]);
  CU_ASSERT_PTR_NOT_NULL(ptr[2]);
  xlh_free(&heap, ptr[1]);
  xlh_free(&heap, ptr[0]);
  xlh_free(&heap, ptr[2]);
  xlh_free_stats(&heap, &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.total_size, init_length);
  CU_ASSERT_EQUAL(stats.max_block_size, stats.total_size);
  CU_ASSERT_PTR_EQUAL(xlh_end(&heap, NULL), pool);
  CU_ASSERT_EQUAL(0, memcmp(&heap, &ref0, sizeof(xlh_heap_t)));

  CU_ASSERT_PTR_NOT_NULL(xlh_init(&heap, &pool, length, NULL));
  ptr[0] = xlh_alloc(&heap, 200);
  ptr[1] = xlh_alloc(&heap, 50000);
  ptr[2] = xlh_alloc(&heap, 100);
  CU_ASSERT_PTR_NOT_NULL(ptr[0]);
  CU_ASSERT_PTR_NOT_NULL(ptr[1]);
  CU_ASSERT_PTR_NOT_NULL(ptr[2]);
  xlh_free(&heap, ptr[1]);
  xlh_free(&heap, ptr[0]);
  xlh_free(&heap, ptr[2]);
  xlh_free_stats(&heap, &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.total_size, init_length);
  CU_ASSERT_EQUAL(stats.max_block_size, stats.total_size);
  CU_ASSERT_PTR_EQUAL(xlh_end(&heap, &stats), pool);
  CU_ASSERT_EQUAL(0, memcmp(&heap, &ref0, sizeof(xlh_heap_t)));
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
}

static void end_remain() {
  size_t length = 1024*1024;
  char pool[length];
  xlh_heap_t heap, ref0;
  xlh_stats_t stats;
  memset(&ref0, 0, sizeof(xlh_heap_t));
  void* ptr[3];

  memset(&heap, 0, sizeof(xlh_heap_t));
  CU_ASSERT_PTR_NOT_NULL(xlh_init(&heap, &pool, length, NULL));
  ptr[0] = xlh_alloc(&heap, 200);
  ptr[1] = xlh_alloc(&heap, 50000);
  ptr[2] = xlh_alloc(&heap, 100);
  CU_ASSERT_PTR_NOT_NULL(ptr[0]);
  CU_ASSERT_PTR_NOT_NULL(ptr[1]);
  CU_ASSERT_PTR_NOT_NULL(ptr[2]);
  CU_ASSERT_PTR_EQUAL(xlh_end(&heap, NULL), pool);
  CU_ASSERT_EQUAL(0, memcmp(&heap, &ref0, sizeof(xlh_heap_t)));

  CU_ASSERT_PTR_NOT_NULL(xlh_init(&heap, &pool, length, NULL));
  ptr[0] = xlh_alloc(&heap, 200);
  ptr[1] = xlh_alloc(&heap, 50000);
  ptr[2] = xlh_alloc(&heap, 100);
  CU_ASSERT_PTR_NOT_NULL(ptr[0]);
  CU_ASSERT_PTR_NOT_NULL(ptr[1]);
  CU_ASSERT_PTR_NOT_NULL(ptr[2]);
  xlh_free(&heap, ptr[1]);
  xlh_free_stats(&heap, &stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_PTR_EQUAL(xlh_end(&heap, &stats), pool);
  CU_ASSERT_EQUAL(0, memcmp(&heap, &ref0, sizeof(xlh_heap_t)));
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.total_size, 300);
  CU_ASSERT_EQUAL(stats.max_block_size, 200);
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
  char* pool = malloc(TEST_HEAP_SIZE);
  memset(test_heap(), 0, sizeof(xlh_heap_t));
  xlh_init(test_heap(), pool, TEST_HEAP_SIZE, &protect);
  reset_protect();
  return 0;
}

static int cleanup_protected_heap() {
  reset_protect();
  xlh_stats_t stats;
  void *pool = xlh_end(test_heap(), &stats);
  assert(!stats.count);
  free(pool);
  return 0;
}

static void simple_test_protect() {
  xlh_stats_t stats;
  size_t lninit = TEST_HEAP_SIZE - sizeof(xlh_node_t);
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), lninit);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  void *ptr = xlh_alloc(test_heap(), 42);
  CU_ASSERT_PTR_NOT_NULL(ptr);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), lninit - XTC_ALIGNED_SIZE(42) - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(lock_called, 2);
  CU_ASSERT_EQUAL(unlock_called, 2);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  xlh_free(test_heap(), ptr);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  CU_ASSERT_EQUAL(xlh_max_free_blk(test_heap()), lninit);
  CU_ASSERT_EQUAL(lock_called, 2);
  CU_ASSERT_EQUAL(unlock_called, 2);
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(lock_called, 3);
  CU_ASSERT_EQUAL(unlock_called, 3);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(lock_called, 4);
  CU_ASSERT_EQUAL(unlock_called, 4);
  reset_protect();
}

static void multiple_test_protect() {
  size_t n = TEST_HEAP_OPT_COUNT;
  void* arr[n];
  memset(arr, 0, sizeof(arr));
  size_t i;
  xlh_stats_t stats;
  reset_protect();

  for(i = 0; i < n; i++) {
    arr[i] = xlh_alloc(test_heap(), 16);
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, n);
  CU_ASSERT_EQUAL(lock_called, i+2);
  CU_ASSERT_EQUAL(unlock_called, i+2);
  reset_protect();

  for(i = 0; i < n; i++) {
    CU_ASSERT_PTR_NULL(xlh_alloc(test_heap(), 16));
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, n);
  CU_ASSERT_EQUAL(lock_called, i+2);
  CU_ASSERT_EQUAL(unlock_called, i+2);
  reset_protect();

  for(i = 0; i < n; i++) {
    xlh_free(test_heap(), arr[i]);
    arr[i] = NULL;
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(lock_called, i+2);
  CU_ASSERT_EQUAL(unlock_called, i+2);
  reset_protect();
}

static void error_test_protect() {
  size_t lock_calls = 0;
  reset_protect();

  xlh_heap_t heap;
  CU_ASSERT_PTR_NULL(xlh_alloc(&heap, 42)); //lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  CU_ASSERT_PTR_NULL(xlh_alloc((xlh_heap_t*)(((char*)(&heap)) + 2), 42)); //lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  CU_ASSERT_PTR_NULL(xlh_alloc((xlh_heap_t*)test_heap()->mem_pool, 42)); //lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  CU_ASSERT_PTR_NULL(xlh_alloc(test_heap(), 0)); //lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  size_t max = xlh_max_free_blk(test_heap()); lock_calls++;
  CU_ASSERT_PTR_NULL(xlh_alloc(test_heap(), max + 1)); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  xlh_free(NULL, (void*)42); //lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xlh_free(test_heap(), NULL); //lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  int bullshit;
  xlh_free(test_heap(), &bullshit); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  void* ptr = xlh_alloc(test_heap(), 42); lock_calls++;
  xlh_free(test_heap(), (void*)((char*)ptr - 1)); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xlh_free(test_heap(), (void*)((char*)ptr + 1)); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xlh_free(test_heap(), ptr); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  CU_ASSERT_EQUAL(lock_called, lock_calls);

  lock_calls = 0;
  cleanup_protected_heap(); init_protected_heap();
  ptr = xlh_alloc(test_heap(), 42); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xlh_free(test_heap(), ptr); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xlh_free(test_heap(), ptr); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  char buf[42];
  xlh_free(test_heap(), (void*)buf); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  // Build a fake node with real data but not allocated with structure heap
  ptr = xlh_alloc(test_heap(), 42); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  size_t sznode = sizeof(xlh_node_t) + XTC_ALIGNED_SIZE(42);
  xlh_node_t *test = malloc(sznode);
  xlh_node_t *node = (xlh_node_t*)ptr - 1;
  memcpy(test, node, sznode);
  xlh_free(test_heap(), (void*)(test+1)); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  xlh_free(test_heap(), ptr); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  free(test);

  CU_ASSERT_EQUAL(lock_called, lock_calls);
  reset_protect();
}

static void end_test_protect() {
  xlh_heap_t ref0;
  memset(&ref0, 0, sizeof(xlh_heap_t));

  void* pool = test_heap()->mem_pool;
  CU_ASSERT_PTR_EQUAL(xlh_end(test_heap(), NULL), pool);
  free(pool);
  CU_ASSERT_EQUAL(lock_called, 2);
  CU_ASSERT_EQUAL(unlock_called, lock_called);
  CU_ASSERT_EQUAL(0, memcmp(test_heap(), &ref0, sizeof(xlh_heap_t)));

  xlh_stats_t stats;
  init_protected_heap();
  pool = test_heap()->mem_pool;
  CU_ASSERT_PTR_EQUAL(xlh_end(test_heap(), &stats), pool);
  free(pool);
  CU_ASSERT_EQUAL(lock_called, 2);
  CU_ASSERT_EQUAL(unlock_called, lock_called);
  CU_ASSERT_EQUAL(0, memcmp(test_heap(), &ref0, sizeof(xlh_heap_t)));

  init_protected_heap();  // so that cleanup works
}

#define PERF_HEAP_SIZE    (1024*1024)

static int init_perf_heap() {
  size_t length = PERF_HEAP_SIZE;
  char* pool = malloc(length);
  memset(test_heap(), 0, sizeof(xlh_heap_t));
  xlh_init(test_heap(), pool, length, NULL);
  return 0;
}

static int cleanup_perf_heap() {
  xlh_stats_t stats;
  void *pool = xlh_end(test_heap(), &stats);
  assert(!stats.count);
  free(pool);
  return 0;
}

static void test_perf() {
  int n = 64;
  void* arr[n];
  memset(arr, 0, sizeof(arr));
  srand(time(NULL));
  int na = 0, nf = 0, last = 0;
  clock_t sa = 0, sf = 0; 
  xlh_stats_t stats;
  printf("\n ===== %03d%% : ", last);
  for (int i = 0; i < TEST_PERFORMANCE_ROUND; i++) {
    int adv = (i * 100) / TEST_PERFORMANCE_ROUND;
    if (last != adv) {
      last = adv;
      printf("\r ===== %03d%% : ", last);
    }
    int id = rand() % n;
    size_t count = 0;
    if (arr[id]) {
      count = xlh_count(test_heap());
      xlh_free_stats(test_heap(), &stats);
      size_t total = stats.total_size;
      clock_t start = clock();
      xlh_free(test_heap(), arr[id]);
      sf += clock() - start;
      arr[id] = NULL;
      nf++;
      CU_ASSERT_EQUAL(xlh_count(test_heap()), count - 1);
      xlh_free_stats(test_heap(), &stats);
      CU_ASSERT(stats.total_size > total);
    } else {
      count = xlh_count(test_heap());
      double max = xlh_max_free_blk(test_heap());
      size_t size_to_alloc = (size_t)(max * ((double)rand() / (double)RAND_MAX));
      if (size_to_alloc > max) {
        size_to_alloc = max;  // to be sure
      }
      if (size_to_alloc) {
        clock_t start = clock();
        arr[id] = xlh_alloc(test_heap(), size_to_alloc);
        sa += clock() - start;
        na++;
        CU_ASSERT_PTR_NOT_NULL(arr[id]);
        CU_ASSERT_EQUAL(xlh_count(test_heap()), count + 1);
      }
    }
  }
  printf("\r ===== 100%% : ");
  for (int i = 0; i < n; i++) {
    if (arr[i]) {
      clock_t start = clock();
      xlh_free(test_heap(), arr[i]);
      sf += clock() - start;
      arr[i] = NULL;
      nf++;
    }
  }
  xlh_free_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.total_size, PERF_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.max_block_size, stats.total_size);
  xlh_allocated_stats(test_heap(), &stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(na, nf);
  double avga, avgf;
  avga = (double)sa * 1e9 / CLOCKS_PER_SEC / na;
  avgf = (double)sf * 1e9 / CLOCKS_PER_SEC / nf;
  printf("#alloc. = %.3lf ns [%d] #free. = %.3lf ns [%d] : ...", avga, na, avgf, nf);
}
