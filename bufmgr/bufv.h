#ifndef _BUF_MALLOC_H
#define _BUF_MALLOC_H

#include <stdlib.h>
#include <iostream>
using namespace std;
#define iMALLOC posix_memalign   //int posix_memalign (void **memptr,size_t alignment,size_t size);����posix_memalign( )�ɹ�ʱ��
                               //����size�ֽڵĶ�̬�ڴ棬��������ڴ�ĵ�ַ��alignment�ı���������alignment������2���ݣ�
                                //����voidָ��Ĵ�С�ı��������ص��ڴ��ĵ�ַ������memptr���棬��������ֵ��0.
const int DIRTY = 1;
const int CLEAN = 0;
const int DELETE = 2;
#endif


