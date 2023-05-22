#include "cu_tests.h"
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static int create_tests(const test_suite_getter_t getters[]);

int main() {
  int rtn;
  for(rtn = EXIT_FAILURE; rtn != EXIT_SUCCESS; rtn = EXIT_SUCCESS) {

    if (CUE_SUCCESS != CU_initialize_registry())
      break;

    if (!create_tests(tsg_table)) {
      fprintf(stderr, "ERROR: %s\n", CU_get_error_msg());
      CU_cleanup_registry();
      break;
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    if (CUE_SUCCESS != CU_get_error()) {
      fprintf(stderr, "ERROR: %s\n", CU_get_error_msg());
      CU_cleanup_registry();
      break;
    }

    CU_cleanup_registry();
  }
  return rtn;
}

static int create_tests(const test_suite_getter_t getters[]) {
  assert(getters);
  for (int i = 0; getters[i]; i++) {
    const test_suite_t *ts = (getters[i])();
    assert(ts && ts->title && ts->title);
    CU_pSuite ps;
    if(NULL == (ps = CU_add_suite(ts->title, ts->init, ts->cleanup))) {
      return 0;
    }
    for (test_t *t = ts->tests; t->title && t->test; t++) {
      if(NULL == CU_add_test(ps, t->title, t->test)) {
        return 0;
      }
    }
  }
  return 1;
}
