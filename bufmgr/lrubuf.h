#ifndef _LRUBUF_H
#define _LRUBUF_H

#include "bufmgr.h"

typedef struct _LruNode
{ 
	LID pageid;
	int diskId;
	int next;
	int pre;
}lpnode;

class lrubuf:public bufmgr
{
	public:
		lrubuf(int bufsz, int page_size);
		~lrubuf();	
		void clearbuf();
	protected:
		int lru;
		int mru;
		lpnode ** lru_array;
		npnode ** mgr_array;
		int freebuf;

		void leavemgr(LID pageid, iofdctl * disk);
		npnode * findPage(LID pageid, iofdctl * disk);
		void entermgr(LID pageid, iofdctl * disk);
		void actual_entermgr(LID pageid, iofdctl * disk, int bufid);
		char * className();
		int requestbuf(LID pageid, iofdctl * disk);

		int allocbufid();
		void rmlru(int bufid);
		int getevict();  //return the evicted bufid
		void getinlru(LID pageid, iofdctl * disk, int bufid);
		lpnode * newLru(LID pageid, iofdctl * disk);
		void adjustlru(int bufid);
};

#endif
