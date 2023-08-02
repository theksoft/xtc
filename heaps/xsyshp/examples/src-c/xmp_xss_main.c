#include "xmp_xss.h"
#include <stdio.h>
#include <assert.h>

typedef struct {
  int field1, field2, field3, field4;
} my_struct;

int main() {
  my_struct *s1, *s2;

  s1 = sys_alloc(sizeof(my_struct));
  assert(s1);

  s1->field1 = 1;
  s1->field2 = 2;
  s1->field3 = 3;
  s1->field4 = 4;

  s2 = sys_alloc(sizeof(my_struct)*2);
  assert(s2);

  *s2 = *s1;
  *(s2+1) = *s1;

  sys_free(s1);
  sys_free(s2);

  printf("\nExample with nothing to print - Check source code for implementation example\n\n");

  return 0;
}
