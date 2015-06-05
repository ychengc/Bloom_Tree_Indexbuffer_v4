#ifndef _BUF_MALLOC_H
#define _BUF_MALLOC_H

#include <stdlib.h>
#include <iostream>
using namespace std;
#define iMALLOC posix_memalign   //int posix_memalign (void **memptr,size_t alignment,size_t size);调用posix_memalign( )成功时会
                               //返回size字节的动态内存，并且这块内存的地址是alignment的倍数。参数alignment必须是2的幂，
                                //还是void指针的大小的倍数。返回的内存块的地址放在了memptr里面，函数返回值是0.
const int DIRTY = 1;
const int CLEAN = 0;
const int DELETE = 2;
#endif


