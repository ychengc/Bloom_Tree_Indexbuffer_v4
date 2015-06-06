#include "bufmgr.h"
#include "bufv.h"

diskSet * diskinfo = NULL;

bufmgr::bufmgr(int bufsz, int page_size):bufsize(bufsz), PAGE_SIZE(page_size)
{
	if(iMALLOC((void**)&buffer, PAGE_SIZE, PAGE_SIZE*bufsize)){
		cout<<"Exception--->iMALLOC fail in nonbuf()"<<endl;
		exit(0);
	}
	io_read_num = 0;
	io_write_num = 0;
}

bufmgr::~bufmgr()
{
	free(buffer);
}

npnode * bufmgr::newPage(LID pageid, int bufid, iofdctl * disk)
{
	npnode * newpage = new npnode;
	newpage->pageid = pageid;
	newpage->bufid = bufid;
	newpage->isDirty = CLEAN;
	newpage->next = NULL;
	newpage->locate = disk->diskType();
	newpage->diskId = disk->diskID();
	return newpage;
}

int bufmgr::get_bufsize()
{
	return bufsize;
}

void bufmgr::syncbuf(npnode * tarp)          //sync a dirty page to disk
{
	iofdctl * iodisk;
	iodisk = disk_array(tarp->diskId);
	if(tarp == NULL)
	{
		cout<<"Exception---> exectue fail in bufmgr-requestbuf()"<<endl;
		exit(0);
	}
	if(tarp->isDirty == DIRTY)
	{
		iodisk->diskWrite(&(tarp->pageid), (BYTE*)(buffer+tarp->bufid*PAGE_SIZE), 1);
		io_write_num++;
		if(iodisk->diskType() == SSD_TYPE)
			_ssd_write_num++;
		else if(iodisk->diskType() == HDD_TYPE)
			_hdd_write_num++;
	}
}

BYTE * bufmgr::readbuf(LID pageid, iofdctl * disk)       //read a page to buffer
{
	int offbuf = requestbuf(pageid, disk);
	return (BYTE*)(buffer+offbuf);
}

void bufmgr::writebuf(LID pageid, iofdctl * disk)           //set a page as dirty
{
	npnode * tarp;
	if((tarp=findPage(pageid, disk)) == NULL)
	{
		cout<<"Error--->exist page can not be found in bufmgr-writebuf(), pageid is "<<pageid<<endl;
		exit(0);
	}
	tarp->isDirty = DIRTY;
}

LID bufmgr::applypage(iofdctl * disk)
{
	LID pageid;
	disk->allocPage(&pageid, 1);
	entermgr(pageid, disk);
	return pageid;
}

char * bufmgr::className()
{
	return "bufmgr_c";
}

void bufmgr::deletepage(LID pageid, iofdctl * disk)     //delete a page from both buffer and file
{
	if(disk->existPage(pageid) < 0)
	{
		cout<<"Exception--->"<<className()<<" delete disk page not exist"<<endl;
		exit(0);
 	}
	npnode * itp;
	itp = findPage(pageid, disk);

	if(itp != NULL)
	  leavemgr(itp->pageid, disk);
	disk->delPage(&pageid, 1);
}

LID bufmgr::modifyDisk(LID src_pid, iofdctl * src_disk, iofdctl * disk)    //this function is used for page migration in hybrid storage system
{   
	npnode * curpage;
	if((curpage=findPage(src_pid, src_disk)) == NULL)
	{
		readbuf(src_pid, src_disk);
		curpage = findPage(src_pid, src_disk);
	}

	LID new_pid;
	int curbufid;
	
	curbufid = curpage->bufid;
	src_disk->delPage(&src_pid, 1);
	leavemgr(src_pid, src_disk);
	disk->allocPage(&new_pid, 1);
	actual_entermgr(new_pid, disk, curbufid);
	writebuf(new_pid, disk);
	return new_pid;
}

