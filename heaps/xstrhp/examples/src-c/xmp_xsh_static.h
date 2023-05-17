#ifndef _XMP_XSH_STATIC_H_
#define _XMP_XSH_STATIC_H_

#include "xmp_xsh_struct.h"

#ifndef __USE_MALLOC

my_struct* xsh_alloc_s();
void xsh_free_s(my_struct *p);

#else   // __USE_MALLOC

#include <stdlib.h>
#define xsh_alloc_s()   (my_struct*)malloc(sizeof(my_struct))
#define xsh_free_s(p)   free((void*)p)

#endif  //  __USE_MALLOC

#endif
