#ifndef _BUFMGR_H
#define _BUFMGR_H

#include "../io/iofdctl.h"
#include "bufhead.h"
#include "../btree/global.h"
/*
const int BUF_EXIST = 1;
const int BUF_NOEXIST = -1;
const int PAGEFULL = -1;
*/

extern int _ssd_read_num;
extern int _ssd_write_num;
extern int _hdd_read_num;
extern int _hdd_write_num;
typedef struct _diskSet
{
	int disk_num;
	iofdctl * disk_array[50];  //the iofdctl disks used are all in this array
}diskSet;

extern diskSet * diskinfo; // = NULL;

#define disk_num   (diskinfo->disk_num)
#define disk_array(x) (diskinfo->disk_array[x])
/* 
typedef struct buffer_page
{
	BYTE value[PAGESIZE];
}PAGE; 
*/


class bufmgr
{
	public:
		int io_read_num;
		int io_write_num;

		bufmgr(int bufsz, int page_size);
		virtual ~bufmgr();
		
		virtual BYTE * readbuf(LID pageid, iofdctl * disk);
		virtual void writebuf(LID pageid, iofdctl * disk);
		virtual LID applypage(iofdctl * disk); //apply 'needsize' buf and return offset in buffer
		virtual void deletepage(LID pageid, iofdctl * disk);
		virtual LID modifyDisk(LID src_pid, iofdctl * src_disk, iofdctl * disk);
		
		virtual void clearbuf()=0;
		virtual int get_bufsize();
	protected:
		const int bufsize;
		BYTE * buffer;
//		iofdctl* iSSD;
//		iofdctl* iHDD;
		const int PAGE_SIZE;
		
		virtual void leavemgr(LID pageid, iofdctl * disk)=0;
		virtual npnode * findPage(LID pageid, iofdctl * disk)=0;
		virtual void entermgr(LID pageid, iofdctl * disk)=0;
		virtual void actual_entermgr(LID pageid, iofdctl * disk, int bufid)=0;
		virtual int requestbuf(LID pageid, iofdctl * disk)=0;  //locate the page in buffer and return its offset in buffer
		
		virtual npnode * newPage(LID pageid, int bufid, iofdctl * disk);
		virtual void syncbuf(npnode * tarp);
		virtual char * className();
};

#endif
