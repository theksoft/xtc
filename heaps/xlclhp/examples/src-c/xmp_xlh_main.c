#include "xmp_xlh_static.h"
#include "xmp_xlh_prealloc.h"
#include <stdio.h>
#include <assert.h>

typedef struct {
  int field1, field2, field3, field4;
} my_struct;

int main() {
  my_struct *s1, *s2;

  s1 = xlh_alloc_s(sizeof(my_struct));
  assert(s1);

  s1->field1 = 1;
  s1->field2 = 2;
  s1->field3 = 3;
  s1->field4 = 4;

  s2 = xlh_alloc_p(sizeof(my_struct)*2);
  assert(s2);

  *s2 = *s1;
  *(s2+1) = *s1;

  xlh_free_s(s1);
  xlh_free_p(s2);

  printf("\nExample with nothing to print - Check source code for implementation example\n\n");

  return 0;
}
