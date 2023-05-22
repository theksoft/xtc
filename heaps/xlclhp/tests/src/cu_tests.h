#ifndef _CU_TESTS_H_
#define _CU_TESTS_H_

typedef struct {
  const char *title;
  void (*test)(void);
} test_t;

typedef struct {
  const char *title;
  int (*init)(void);
  int (*cleanup)(void);
  test_t *tests;
} test_suite_t;

typedef test_suite_t* (*test_suite_getter_t)(void);
#define TEST_SUITE_GETTER_END    ((test_suite_getter_t)0)

extern const test_suite_getter_t tsg_table[];

#endif // _CU_TESTS_H_
