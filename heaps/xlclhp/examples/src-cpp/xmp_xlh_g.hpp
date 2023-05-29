#ifndef _XMP_XLH_G_HPP_
#define _XMP_XLH_G_HPP_

#include <new>
#include "xtc_lclhp.hxx"

void* operator new(std::size_t count);
void operator delete(void* ptr);
void* operator new[](std::size_t count);
void operator delete[](void* ptr);

#endif
