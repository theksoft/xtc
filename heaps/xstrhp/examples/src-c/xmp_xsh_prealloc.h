#ifndef _XMP_XSH_PREALLOC_H_
#define _XMP_XSH_PREALLOC_H_

#include "xmp_xsh_struct.h"

#ifndef __USE_MALLOC

my_struct* xsh_alloc_p();
void xsh_free_p(my_struct *p);

#else   // __USE_MALLOC

#include <stdlib.h>
#define xsh_alloc_p()   (my_struct*)malloc(sizeof(my_struct))
#define xsh_free_p(p)   free((void*)p)

#endif  //  __USE_MALLOC

#endif
