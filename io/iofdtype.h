#ifndef _IOFDTYPE_H
#define _IOFDTYPE_H

#include <map>
#include <vector>
#include <iterator>
#include <climits>
#include <algorithm> 
#include "../btree/global.h"
using namespace std;

typedef int IOFD_PID;
typedef int IOFD_LID;


typedef struct seq_free_space
{
	IOFD_PID baseAddr;
	int numOffset;
}IOFD_FREESPACE;

typedef multimap<int, IOFD_FREESPACE *> IOFDfreespace;
typedef map<IOFD_PID, IOFD_FREESPACE *> IOFDfreeaddr;
typedef map<int, int> IOFDlid_pid;

typedef unsigned char BYTE;
typedef int IOFD_LXFILE; 
typedef int IOFD_ACCESSTYPE;

#endif
