#ifndef _XMP_PREALLOC_XSH_H_
#define _XMP_PREALLOC_XSH_H_

#include "xmp_struct_xsh.h"

#ifndef __USE_MALLOC

my_struct* xsh_alloc_p();
void xsh_free_p(my_struct *p);

#else   // __USE_MALLOC

#include <stdlib.h>
#define xsh_alloc_p()   (my_struct*)malloc(sizeof(my_struct))
#define xsh_free_p(p)   free((void*)p)

#endif  //  __USE_MALLOC

#endif
