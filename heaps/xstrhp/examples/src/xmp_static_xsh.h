#ifndef _XMP_STATIC_XSH_H_
#define _XMP_STATIC_XSH_H_

#include "xmp_struct_xsh.h"

#ifndef __USE_MALLOC

my_struct* xsh_alloc_s();
void xsh_free_s(my_struct *p);

#else   // __USE_MALLOC

#include <stdlib.h>
#define xsh_alloc_s()   (my_struct*)malloc(sizeof(my_struct))
#define xsh_free_s(p)   free((void*)p)

#endif  //  __USE_MALLOC

#endif
