#include <CUnit/CUnit.h>
#include <cassert>
#include "cxx_tests_xlh.h"
#include "xtc_lclhp.hxx"

#define ASSERT_EQUAL_STATS(a, b)  \
  do {  \
    CU_ASSERT_EQUAL(a.count, b.count); \
    CU_ASSERT_EQUAL(a.max_block_size, b.max_block_size); \
    CU_ASSERT_EQUAL(a.total_size, b.total_size); \
  } while(0)

#define TEST_HEAP_OPT_COUNT     8
#define TEST_HEAP_SIZE          (TEST_HEAP_OPT_COUNT*(sizeof(xlh_node_t) + 16))
static xtc::xlh::Allocator<TEST_HEAP_SIZE> *cppalloc;

extern "C" int cxx_init_allocator() {
  cppalloc = new xtc::xlh::Allocator<TEST_HEAP_SIZE>();
  return 0;
}

extern "C" int cxx_cleanup_allocator() {
  delete cppalloc;
  return 0;
}

extern "C" void cxx_alloc_0_size() {
  xlh_stats_t stats;
  cppalloc->allocatedStats(stats);
  size_t count = stats.count;
  CU_ASSERT_PTR_NULL(cppalloc->allocate(0));
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, count);
}

extern "C" void cxx_alloc_bad_size() {
  xlh_stats_t stats;
  cppalloc->allocatedStats(stats);
  size_t count = stats.count;
  cppalloc->freeStats(stats);
  CU_ASSERT_PTR_NULL(cppalloc->allocate(stats.max_block_size + 1));
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, count);
}

extern "C" void cxx_simple_alloc_free() {
  size_t lninit = TEST_HEAP_SIZE - sizeof(xlh_node_t);
  CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), lninit);
  void *ptr = cppalloc->allocate(12);
  CU_ASSERT_PTR_NOT_NULL(ptr);
  CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), lninit - XTC_ALIGNED_SIZE(12) - sizeof(xlh_node_t));
  cppalloc->free(ptr);
  CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), lninit);
  for (size_t ln = 20, lnoff = 0; lnoff < sizeof(int) + 1; lnoff++) {
    ptr = cppalloc->allocate(ln+lnoff);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), lninit - XTC_ALIGNED_SIZE(ln+lnoff) - sizeof(xlh_node_t));
    cppalloc->free(ptr);
    CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), lninit);
  }
}

extern "C" void cxx_sequential_alloc_free() {
  size_t lninit = TEST_HEAP_SIZE - sizeof(xlh_node_t);
  size_t szalloc = 14, szblk = XTC_ALIGNED_SIZE(szalloc);
  size_t n = TEST_HEAP_SIZE / (szblk + sizeof(xlh_node_t));
  assert(n == TEST_HEAP_OPT_COUNT);
  // If not the case some assert are not correct as the last block can be different
  void* arr[TEST_HEAP_OPT_COUNT];
  memset(arr, 0, sizeof(arr));
  size_t i;
  // Allocate all sequentially
  size_t expected_size = lninit;
  for (i = 0; i < n; i++) {
    CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), expected_size);
    CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), expected_size);
    arr[i] = cppalloc->allocate(szalloc);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
    expected_size -= sizeof(xlh_node_t) + szblk;
  }
  xlh_stats_t stats;
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, n);
  CU_ASSERT_EQUAL(stats.max_block_size, szblk);
  CU_ASSERT_EQUAL(stats.total_size, n*szblk);
  // Check there is really no more space
  CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), 0);
  CU_ASSERT_PTR_NULL(cppalloc->allocate(szalloc));
  CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), 0);
  // Free all blocks
  expected_size = 0;
  for (i = 0; i < n; i++) {
    CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), expected_size);
    cppalloc->free(arr[i]);
    if (expected_size) {
      expected_size += sizeof(xlh_node_t);
    }
    expected_size += szblk;
  }
  CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), lninit);
}

extern "C" void cxx_unordered_alloc_free() {
  size_t lninit = TEST_HEAP_SIZE - sizeof(xlh_node_t);
  size_t szalloc = 3, szblk = XTC_ALIGNED_SIZE(szalloc);
  size_t n = TEST_HEAP_SIZE / (szblk + sizeof(xlh_node_t));
  size_t szlast = lninit - (n-1)*(szblk + sizeof(xlh_node_t));
  void* arr[16];
  assert(n <= 16);
  memset(arr, 0, sizeof(arr));
  size_t i;
  // Alloc all
  xlh_stats_t stats;
  size_t expected_size = lninit;
  for (i = 0; i < n; i++) {
    CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), expected_size);
    arr[i] = cppalloc->allocate(szalloc);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
    expected_size -= expected_size > szblk ? sizeof(xlh_node_t) + szblk : szblk;
  }
  // Free 1 over 2
  cppalloc->freeStats(stats);
  size_t count = stats.count;
  CU_ASSERT_EQUAL(count, 0);
  for (i = 0; i < n; i+=2) {
    cppalloc->freeStats(stats);
    CU_ASSERT_EQUAL(stats.count, count++);
    CU_ASSERT_EQUAL(stats.max_block_size, stats.count ? szblk : 0);
    CU_ASSERT_EQUAL(stats.total_size, stats.count*szblk);
    cppalloc->free(arr[i]);
    arr[i] = NULL;
  }
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, count);
  CU_ASSERT_EQUAL(stats.max_block_size, (n%2) ? szlast : szblk);
  CU_ASSERT_EQUAL(stats.total_size, (count-1)*szblk + ((n%2) ? szlast : szblk));
  // Alloc again half of the previously freed
  for (i = 0; i < n; i+=4) {
    assert(!arr[i]);
    cppalloc->freeStats(stats);
    CU_ASSERT_EQUAL(stats.count, count--);
    CU_ASSERT_EQUAL(stats.max_block_size, (n%2) ? szlast : szblk);
    CU_ASSERT_EQUAL(stats.total_size, (stats.count-1)*szblk + ((n%2) ? szlast : szblk));
    arr[i] = cppalloc->allocate(szalloc);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  for (i = 0; count; ) {
    while(arr[i]) i++;
    assert(i < n);
    cppalloc->freeStats(stats);
    CU_ASSERT_EQUAL(stats.count, count--);
    CU_ASSERT_EQUAL(stats.max_block_size, (n%2) ? szlast : szblk);
    CU_ASSERT_EQUAL(stats.total_size, (stats.count-1)*szblk + ((n%2) ? szlast : szblk));
    arr[i] = cppalloc->allocate(szalloc);
    CU_ASSERT_PTR_NOT_NULL(arr[i]);
  }
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, n);
  CU_ASSERT_EQUAL(stats.max_block_size, (n%2) ? szlast : szblk);
  CU_ASSERT_EQUAL(stats.total_size, (n-1)*szblk + ((n%2) ? szlast : szblk));
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
  CU_ASSERT_PTR_NULL(cppalloc->allocate(szalloc));
  for (i = 0; i < n; i++) {
    if (!arr[i]) continue;
    count++;
  }
  CU_ASSERT_EQUAL(count, n);
  for (i = 0; i < n; i++) {
    cppalloc->allocatedStats(stats);
    CU_ASSERT_EQUAL(stats.count, n-i);
    cppalloc->free(arr[i]);
    arr[i] = NULL;
  }
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, lninit);
  CU_ASSERT_EQUAL(stats.total_size, lninit);
}

extern "C" void cxx_free_null() {
  xlh_stats_t ini, ref, aref, stats;
  cppalloc->freeStats(ini);
  void* ptr = cppalloc->allocate(TEST_HEAP_SIZE / 2);
  cppalloc->freeStats(ref);
  cppalloc->allocatedStats(aref);

  cppalloc->free(NULL);
  cppalloc->freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ref);
  cppalloc->allocatedStats(stats);
  ASSERT_EQUAL_STATS(stats, aref);

  cppalloc->free(ptr);
  cppalloc->freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ini);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
}

extern "C" void cxx_free_invalid() {
  xlh_stats_t ini, ref, aref, stats;
  cppalloc->freeStats(ini);
  int bullshit;

  cppalloc->free(&bullshit);
  cppalloc->freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ini);

  void* ptr = cppalloc->allocate(TEST_HEAP_SIZE / 3);
  cppalloc->freeStats(ref);
  CU_ASSERT_EQUAL(ref.count, 1);
  CU_ASSERT_EQUAL(ref.max_block_size, ini.max_block_size - sizeof(xlh_node_t) - XTC_ALIGNED_SIZE(TEST_HEAP_SIZE/3));
  CU_ASSERT_EQUAL(ref.max_block_size, ref.total_size);
  cppalloc->allocatedStats(aref);
  CU_ASSERT_EQUAL(aref.count, 1);
  CU_ASSERT_EQUAL(aref.max_block_size, XTC_ALIGNED_SIZE(TEST_HEAP_SIZE/3));
  CU_ASSERT_EQUAL(aref.total_size, XTC_ALIGNED_SIZE(TEST_HEAP_SIZE/3));

  cppalloc->free((void*)((char*)ptr - 1));
  cppalloc->freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ref);
  cppalloc->allocatedStats(stats);
  ASSERT_EQUAL_STATS(stats, aref);

  cppalloc->free((void*)((char*)ptr + 1));
  cppalloc->freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ref);
  cppalloc->allocatedStats(stats);
  ASSERT_EQUAL_STATS(stats, aref);

  cppalloc->free(ptr);
  cppalloc->freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ini);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
}

extern "C" void cxx_free_double() {
  cxx_cleanup_allocator(); cxx_init_allocator();
  xlh_stats_t ini, aref, stats;
  cppalloc->freeStats(ini);
  CU_ASSERT_EQUAL(ini.count, 1);
  cppalloc->allocatedStats(aref);
  CU_ASSERT_EQUAL(aref.count, 0);
  void* ptr = cppalloc->allocate(TEST_HEAP_SIZE/2);
  // free
  cppalloc->free(ptr);
  cppalloc->freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ini);
  cppalloc->allocatedStats(stats);
  ASSERT_EQUAL_STATS(stats, aref);
  // Double free
  cppalloc->free(ptr);
  cppalloc->freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ini);
  cppalloc->allocatedStats(stats);
  ASSERT_EQUAL_STATS(stats, aref);
}

extern "C" void cxx_free_not_allocated() {
  xlh_stats_t ini, aref, stats;
  cppalloc->freeStats(ini);
  CU_ASSERT_EQUAL(ini.count, 1);
  cppalloc->allocatedStats(aref);
  CU_ASSERT_EQUAL(aref.count, 0);

  char buf[42];
  cppalloc->free((void*)buf);
  cppalloc->freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ini);
  cppalloc->allocatedStats(stats);
  ASSERT_EQUAL_STATS(stats, aref);

  // Fake node cannot be built - class inner
}

extern "C" void cxx_free_wrong() {
  xlh_stats_t ini1, ref1, aref1, ini2, ref2, aref2, stats;
  cppalloc->freeStats(ini1);
  CU_ASSERT_EQUAL(ini1.count, 1);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);

  // Declare a second heap - on stack
  xtc::xlh::Allocator<TEST_HEAP_SIZE> heap2;
  heap2.freeStats(ini2);
  CU_ASSERT_EQUAL(ini2.count, 1);
  heap2.allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  // Allocate on each heap
  void* ptr1 = cppalloc->allocate(TEST_HEAP_SIZE / 8);
  cppalloc->freeStats(ref1);
  cppalloc->allocatedStats(aref1);
  CU_ASSERT_EQUAL(aref1.count, 1);
  void* ptr2 = heap2.allocate(TEST_HEAP_SIZE / 3);
  heap2.freeStats(ref2);
  heap2.allocatedStats(aref2);
  CU_ASSERT_EQUAL(aref2.count, 1);

  // Free each pointer on the bad heap

  heap2.free(ptr1);
  cppalloc->freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ref1);
  cppalloc->allocatedStats(stats);
  ASSERT_EQUAL_STATS(stats, aref1);
  heap2.freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ref2);
  heap2.allocatedStats(stats);
  ASSERT_EQUAL_STATS(stats, aref2);

  cppalloc->free(ptr2);
  cppalloc->freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ref1);
  cppalloc->allocatedStats(stats);
  ASSERT_EQUAL_STATS(stats, aref1);
  heap2.freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ref2);
  heap2.allocatedStats(stats);
  ASSERT_EQUAL_STATS(stats, aref2);

  // Correct free
  heap2.free(ptr2);
  cppalloc->freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ref1);
  cppalloc->allocatedStats(stats);
  ASSERT_EQUAL_STATS(stats, aref1);
  heap2.freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ini2);
  heap2.allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);

  cppalloc->free(ptr1);
  cppalloc->freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ini1);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  heap2.freeStats(stats);
  ASSERT_EQUAL_STATS(stats, ini2);
  heap2.allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
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
  ptr[0] = cppalloc->allocate(sz[0]);
  ptr[1] = cppalloc->allocate(sz[1]);
  ptr[2] = cppalloc->allocate(sz[2]);
  ptr[3] = cppalloc->allocate(sz[3]);
  assert(ptr[0] && ptr[1] && ptr[2] && ptr[3]);

  // Check
  xlh_stats_t stats;
  cppalloc->freeStats(stats);
  assert(stats.count == 0);
  cppalloc->allocatedStats(stats);
  assert(stats.count == 4);
  assert(stats.total_size == 4 * szmeanalloc);
}

extern "C" void cxx_frag_head() {
  // Make 4 allocations so that all memory is used
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[4] = { 0, 9*base, -3*base, -6*base };
  void* ptr[4];
  frag_allocate_4(ptr, size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);

  // Free the first
  cppalloc->free(ptr[0]); ptr[0] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[0]));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[1] + size[2] + size[3]));

  // Free the second
  cppalloc->free(ptr[1]); ptr[1] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[0]+size[1]) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[2]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[2] + size[3]));

  // Free the third
  cppalloc->free(ptr[2]); ptr[2] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[0]+size[1]+size[2]) + 2*sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[3]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);

  // Free the fourth one
  cppalloc->free(ptr[3]); ptr[3] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
}

extern "C" void cxx_frag_tail() {
  // Make 4 allocations so that all memory is used
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[4] = { 0, 9*base, -3*base, -6*base };
  void* ptr[4];
  frag_allocate_4(ptr, size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);

  // Free the fourth one
  cppalloc->free(ptr[3]); ptr[3] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[3]));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[1] + size[2]));

  // Free the third one
  cppalloc->free(ptr[2]); ptr[2] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[2] + size[3]) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[1]));

  // Free the second one
  cppalloc->free(ptr[1]); ptr[1] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[1] + size[2] + size[3]) + 2*sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);

  // Free the first one
  cppalloc->free(ptr[0]); ptr[0] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
}

extern "C" void cxx_frag_next() {
  // Make 4 allocations so that all memory is used and
  // with the second being the largest one being able to hold a future node
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[4] = { 0, 16*base, -6*base, -10*base };
  assert((size_t)size[1] > sizeof(xlh_node_t) + 2*base);
  void* ptr[4];
  frag_allocate_4(ptr, size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);

  // Free the second
  cppalloc->free(ptr[1]); ptr[1] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[3]));

  // Allocate a size that will split the second block
  ptr[1] = cppalloc->allocate(sizeof(int));
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1] - sizeof(int) - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 4);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[3] + sizeof(int)));

  // Free the second so that it joined the split block
  cppalloc->free(ptr[1]); ptr[1] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[3]));

  // Free the third
  cppalloc->free(ptr[2]); ptr[2] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[1] + size[2]) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[3]));

  // Free the fourth
  cppalloc->free(ptr[3]); ptr[3] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[1] + size[2] + size[3]) + 2*sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);

  // Free the first
  cppalloc->free(ptr[0]); ptr[0] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
}

extern "C" void cxx_frag_previous() {
  // Make 4 allocations so that all memory is used and
  // with the second being the largest one being able to hold a future node
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[4] = { 0, 16*base, -6*base, -10*base };
  assert((size_t)size[1] > sizeof(xlh_node_t) + 2*base);
  void* ptr[4];
  frag_allocate_4(ptr, size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);

  // Free the second
  cppalloc->free(ptr[1]); ptr[1] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[3]));

  // Allocate a size that will split the second block
  ptr[1] = cppalloc->allocate(sizeof(int));
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1] - sizeof(int) - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 4);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[3] + sizeof(int)));

  // Free the third one so the it joined the remain of the second one
  cppalloc->free(ptr[2]); ptr[2] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[1] + size[2]) - sizeof(int));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[3] + sizeof(int)));

  // Free the fourth
  cppalloc->free(ptr[3]); ptr[3] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[1] + size[2] + size[3]) - sizeof(int) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + sizeof(int)));

  // Free the second
  cppalloc->free(ptr[1]); ptr[1] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[1] + size[2] + size[3]) + 2*sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);

  // Free the first
  cppalloc->free(ptr[0]); ptr[0] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
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
  ptr[0] = cppalloc->allocate(sz[0]);
  ptr[1] = cppalloc->allocate(sz[1]);
  ptr[2] = cppalloc->allocate(sz[2]);
  ptr[3] = cppalloc->allocate(sz[3]);
  ptr[4] = cppalloc->allocate(sz[4]);
  assert(ptr[0] && ptr[1] && ptr[2] && ptr[3] && ptr[4]);

  // Check
  xlh_stats_t stats;
  cppalloc->freeStats(stats);
  assert(stats.count == 0);
  cppalloc->allocatedStats(stats);
  assert(stats.count == n);
  assert(stats.total_size == n * szmeanalloc + remain);
}

extern "C" void cxx_frag_both() {
  // Make 5 allocations so that all memory is used
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[5] = { 0, 9*base, -6*base, 2*base, -5*base };
  void* ptr[5];
  frag_allocate_5(ptr, size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);

  // Free the second and the fourth
  cppalloc->free(ptr[1]); ptr[1] = NULL;
  cppalloc->free(ptr[3]); ptr[3] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[1] + size[3]));
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[4]));

  // Free the third so that it joined the second and fourth
  cppalloc->free(ptr[2]); ptr[2] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[1] + size[2] + size[3]) + 2*sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[4]));

  // Free the first
  cppalloc->free(ptr[0]); ptr[0] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[0] + size[1] + size[2] + size[3]) + 3*sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[4]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);

  // Free the fifth
  cppalloc->free(ptr[4]); ptr[4] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
}

extern "C" void cxx_frag_more() {
  // Make 5 allocations so that all memory is used
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[5] = { 0, 9*base, -6*base, 2*base, -5*base };
  void* ptr[5];
  frag_allocate_5(ptr, size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);

  // Free the second and the fourth
  cppalloc->free(ptr[1]); ptr[1] = NULL;
  cppalloc->free(ptr[3]); ptr[3] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[1] + size[3]));
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[4]));

  // Free first
  cppalloc->free(ptr[0]); ptr[0] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[0] + size[1]) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[1] + size[3]) + sizeof(xlh_node_t));
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[4]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[2] + size[4]));

  // Free fifth
  cppalloc->free(ptr[4]); ptr[4] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[0] + size[1]) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[1] + size[3] + size[4]) + 2*sizeof(xlh_node_t));
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[2]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);

  // Free third
  cppalloc->free(ptr[2]); ptr[2] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
}

extern "C" void cxx_frag_more2() {
  // Make 5 allocations so that all memory is used
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[5] = { 0, -5*base, -6*base, 9*base, 2*base };
  void* ptr[5];
  frag_allocate_5(ptr, size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[3]);

  // Free the second and the fourth
  cppalloc->free(ptr[1]); ptr[1] = NULL;
  cppalloc->free(ptr[3]); ptr[3] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[3]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[1] + size[3]));
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[4]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[4]));

  // Free fifth
  cppalloc->free(ptr[4]); ptr[4] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[3] + size[4]) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[1] + size[3] + size[4]) + sizeof(xlh_node_t));
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2]));

  // Free first
  cppalloc->free(ptr[0]); ptr[0] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)(size[3] + size[4]) + sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[1] + size[3] + size[4]) + 2*sizeof(xlh_node_t));
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[2]);
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);

  // Free third
  cppalloc->free(ptr[2]); ptr[2] = NULL;
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
}

extern "C" void cxx_frag_failing() {
  // Make 5 allocations so that all memory is used
  xlh_stats_t stats;
  int base = (int)sizeof(int);
  int size[5] = { 0, 9*base, -6*base, 2*base, -5*base };
  void* ptr[5];
  frag_allocate_5(ptr, size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);

  // Free the second and the fourth
  cppalloc->free(ptr[1]); ptr[1] = NULL;
  cppalloc->free(ptr[3]); ptr[3] = NULL;

  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 2);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[1]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[1] + size[3]));
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 3);
  CU_ASSERT_EQUAL(stats.max_block_size, (size_t)size[0]);
  CU_ASSERT_EQUAL(stats.total_size, (size_t)(size[0] + size[2] + size[4]));

  // Try allocate size greater than second and fourth but less than the sum of them
  CU_ASSERT((size_t)size[1]+1 < stats.total_size);
  CU_ASSERT_EQUAL(cppalloc->allocate(size[1]+1), NULL);

  // Free all
  cppalloc->free(ptr[0]); ptr[0] = NULL;
  cppalloc->free(ptr[4]); ptr[4] = NULL;
  cppalloc->free(ptr[2]); ptr[2] = NULL;

  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  CU_ASSERT_EQUAL(stats.max_block_size, TEST_HEAP_SIZE - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(stats.total_size, stats.max_block_size);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(stats.max_block_size, 0);
  CU_ASSERT_EQUAL(stats.total_size, 0);
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

extern "C" int cxx_init_protected_allocator() {
  xtc_protect_t protect = { lock, unlock };
  reset_protect();
  cppalloc = new xtc::xlh::Allocator<TEST_HEAP_SIZE>(&protect);
  return 0;
}

extern "C" int cxx_cleanup_protected_allocator() {
  reset_protect();
  delete cppalloc;
  return 0;
}

extern "C" void cxx_simple_test_protect() {
  xlh_stats_t stats;
  size_t lninit = TEST_HEAP_SIZE - sizeof(xlh_node_t);
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), lninit);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  void *ptr = cppalloc->allocate(42);
  CU_ASSERT_PTR_NOT_NULL(ptr);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), lninit - XTC_ALIGNED_SIZE(42) - sizeof(xlh_node_t));
  CU_ASSERT_EQUAL(lock_called, 2);
  CU_ASSERT_EQUAL(unlock_called, 2);
  reset_protect();
  CU_ASSERT_EQUAL(lock_called, 0);
  CU_ASSERT_EQUAL(unlock_called, 0);
  cppalloc->free(ptr);
  CU_ASSERT_EQUAL(lock_called, 1);
  CU_ASSERT_EQUAL(unlock_called, 1);
  CU_ASSERT_EQUAL(cppalloc->maxFreeBlk(), lninit);
  CU_ASSERT_EQUAL(lock_called, 2);
  CU_ASSERT_EQUAL(unlock_called, 2);
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(lock_called, 3);
  CU_ASSERT_EQUAL(unlock_called, 3);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(lock_called, 4);
  CU_ASSERT_EQUAL(unlock_called, 4);
  reset_protect();
}

extern "C" void cxx_multiple_test_protect() {
  size_t n = TEST_HEAP_OPT_COUNT;
  void* arr[TEST_HEAP_OPT_COUNT];
  memset(arr, 0, sizeof(arr));
  size_t i;
  xlh_stats_t stats;
  reset_protect();

  for(i = 0; i < n; i++) {
    arr[i] = cppalloc->allocate(16);
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, n);
  CU_ASSERT_EQUAL(lock_called, i+2);
  CU_ASSERT_EQUAL(unlock_called, i+2);
  reset_protect();

  for(i = 0; i < n; i++) {
    CU_ASSERT_PTR_NULL(cppalloc->allocate(16));
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, n);
  CU_ASSERT_EQUAL(lock_called, i+2);
  CU_ASSERT_EQUAL(unlock_called, i+2);
  reset_protect();

  for(i = 0; i < n; i++) {
    cppalloc->free(arr[i]);
    arr[i] = NULL;
  }
  CU_ASSERT_EQUAL(lock_called, i);
  CU_ASSERT_EQUAL(unlock_called, i);
  cppalloc->freeStats(stats);
  CU_ASSERT_EQUAL(stats.count, 1);
  cppalloc->allocatedStats(stats);
  CU_ASSERT_EQUAL(stats.count, 0);
  CU_ASSERT_EQUAL(lock_called, i+2);
  CU_ASSERT_EQUAL(unlock_called, i+2);
  reset_protect();
}

extern "C" void cxx_error_test_protect() {
  size_t lock_calls = 0;
  reset_protect();

  CU_ASSERT_PTR_NULL(cppalloc->allocate(0)); //lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  size_t max = cppalloc->maxFreeBlk(); lock_calls++;
  CU_ASSERT_PTR_NULL(cppalloc->allocate(max + 1)); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  cppalloc->free(NULL); //lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  int bullshit;
  cppalloc->free(&bullshit); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  void* ptr = cppalloc->allocate(42); lock_calls++;
  cppalloc->free((void*)((char*)ptr - 1)); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  cppalloc->free((void*)((char*)ptr + 1)); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  cppalloc->free(ptr); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  CU_ASSERT_EQUAL(lock_called, lock_calls);

  lock_calls = 0;
  cxx_cleanup_protected_allocator(); cxx_init_protected_allocator();
  ptr = cppalloc->allocate(42); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  cppalloc->free(ptr); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);
  cppalloc->free(ptr); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  char buf[42];
  cppalloc->free((void*)buf); lock_calls++;
  CU_ASSERT_EQUAL(lock_called, unlock_called);

  // Fake node not done -> cannot get allocator inner node size
  CU_ASSERT_EQUAL(lock_called, lock_calls);
  reset_protect();
}
