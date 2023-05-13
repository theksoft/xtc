#include "xmp_static_xsh.h"
#include "xmp_prealloc_xsh.h"
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

  return 0;
}
