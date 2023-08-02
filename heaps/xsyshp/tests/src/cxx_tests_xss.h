#ifndef _CXX_TESTS_H_
#define _CXX_TESTS_H_

// No tests associated to xsh_end() as it is wrapped in the allocator

#ifdef __cplusplus
extern "C" {
#endif

int cxx_init_allocator();
int cxx_cleanup_allocator();

void cxx_alloc_0_size();

void cxx_simple_alloc_free();
void cxx_sequential_alloc_free();
void cxx_unordered_alloc_free();

void cxx_free_null();
void cxx_free_invalid();
void cxx_free_double();
void cxx_free_not_allocated();
void cxx_free_wrong();

int cxx_init_protected_allocator();
int cxx_cleanup_protected_allocator();

void cxx_simple_test_protect();
void cxx_multiple_test_protect();
void cxx_error_test_protect();

#ifdef __cplusplus
}
#endif

#endif
