#ifndef _CXX_TESTS_XLH_H_
#define _CXX_TESTS_XLH_H_

#ifdef __cplusplus
extern "C" {
#endif

int cxx_init_allocator();
int cxx_cleanup_allocator();

void cxx_alloc_0_size();
void cxx_alloc_bad_size();

void cxx_simple_alloc_free();
void cxx_sequential_alloc_free();
void cxx_unordered_alloc_free();

void cxx_free_null();
void cxx_free_invalid();
void cxx_free_double();
void cxx_free_not_allocated();
void cxx_free_wrong();

void cxx_frag_head();
void cxx_frag_tail();
void cxx_frag_next();
void cxx_frag_previous();
void cxx_frag_both();
void cxx_frag_more();
void cxx_frag_more2();
void cxx_frag_failing();

int cxx_init_protected_allocator();
int cxx_cleanup_protected_allocator();

void cxx_simple_test_protect();
void cxx_multiple_test_protect();
void cxx_error_test_protect();

#ifdef __cplusplus
}
#endif

#endif
