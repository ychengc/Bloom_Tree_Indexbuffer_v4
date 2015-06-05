#ifndef _NOBUF_H
#define _NOBUF_H

#include "bufmgr.h"

class nobuf:public bufmgr
{
	public:
		nobuf(int bufsz, int page_size);
		~nobuf();

		void clearbuf();
	protected:
		void leavemgr(LID pageid, iofdctl * disk);
		npnode * findPage(LID pageid, iofdctl * disk);
		void entermgr(LID pageid, iofdctl * disk);
		void actual_entermgr(LID pageid, iofdctl * disk, int bufid);
		int requestbuf(LID pageid, iofdctl * disk);
		char * className();
	
		int * bufarray;
		npnode * pagelist;
};

#endif
