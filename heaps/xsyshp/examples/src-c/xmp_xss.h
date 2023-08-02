#ifndef _XMP_XSS_
#define _XMP_XSS_

#include <stdlib.h>

#ifndef __USE_MALLOC

void* sys_alloc(size_t size);
void sys_free(void *p);

#else   // __USE_MALLOC

#define sys_alloc(size)   malloc(size)
#define sys_free(p)       free((void*)p)

#endif  //  __USE_MALLOC

#endif
