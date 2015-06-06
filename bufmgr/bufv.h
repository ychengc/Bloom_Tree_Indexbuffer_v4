#ifndef _BUF_MALLOC_H
#define _BUF_MALLOC_H

#include <stdlib.h>
#include <iostream>
using namespace std;
#define iMALLOC posix_memalign   //int posix_memalign (void **memptr,size_t alignment,size_t size);

const int DIRTY = 1;
const int CLEAN = 0;
const int DELETE = 2;
#endif


