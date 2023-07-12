#ifndef _XMP_XLH_STATIC_H_
#define _XMP_XLH_STATIC_H_

#include <stdlib.h>

#ifndef __USE_MALLOC

void* xlh_alloc_s(size_t size);
void xlh_free_s(void *p);

#else   // __USE_MALLOC

#define xlh_alloc_s(size)   malloc(size)
#define xlh_free_s(p)       free((void*)p)

#endif  //  __USE_MALLOC

#endif
