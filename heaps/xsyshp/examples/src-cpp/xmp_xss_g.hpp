#ifndef _XMP_XSS_G_HPP_
#define _XMP_XSS_G_HPP_

#include <new>
#include "xtc_syshp.hxx"

void* operator new(std::size_t count);
void operator delete(void* ptr);
void* operator new[](std::size_t count);
void operator delete[](void* ptr);

#endif
