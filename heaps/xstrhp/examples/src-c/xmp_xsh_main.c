#include "xmp_xsh_static.h"
#include "xmp_xsh_prealloc.h"
#include <stdio.h>
#include <assert.h>


int main() {
  my_struct *s1, *s2;

  s1 = xsh_alloc_s();
  assert(s1);

  s1->field1 = 1;
  s1->field2 = 2;
  s1->field3 = 3;
  s1->field4 = 4;

  s2 = xsh_alloc_p();
  assert(s2);

  *s2 = *s1;

  xsh_free_s(s1);
  xsh_free_p(s2);

  printf("\nExample with nothing to print - Check source code for implementation example\n\n");

  return 0;
}
