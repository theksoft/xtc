#ifndef _XMP_XLH_PREALLOC_H_
#define _XMP_XLH_PREALLOC_H_

#include <stdlib.h>

#ifndef __USE_MALLOC

void* xlh_alloc_p(size_t size);
void xlh_free_p(void *p);

#else   // __USE_MALLOC

#define xlh_alloc_p(size)   malloc(size)
#define xlh_free_p(p)       free((void*)p)

#endif  //  __USE_MALLOC

#endif
