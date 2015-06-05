#include "lrubuf.h"
#include "bufv.h"
lrubuf::lrubuf(int bufsz, int page_size):bufmgr(bufsz, page_size)
{
	lru = -1;
	mru = -1;
	lru_array = new lpnode*[bufsz];
	mgr_array = new npnode*[bufsz];
	freebuf = bufsz;
	int k;
	for(k=0; k<bufsz; k++)
	{
		lru_array[k] = NULL;
		mgr_array[k] = NULL;
	}
}

lrubuf::~lrubuf()
{
	int k;
	npnode * itmgr;
	npnode * nextmgr;
	for(k=0; k<bufsize; k++)
	{
		if(lru_array[k] != NULL)
		  delete lru_array[k];
		if(mgr_array[k] != NULL)
		{
			itmgr = mgr_array[k];
			while(itmgr != NULL)
			{
				nextmgr = itmgr->next;
				delete itmgr;
				itmgr = nextmgr;
			}
		}
	}
	delete lru_array;
	delete mgr_array;
}

char* lrubuf::className()             //清空缓冲区
{
	return "lrubuf_c";
}

void lrubuf::clearbuf()
{
	int k;
	npnode * tarp;
	for(k=0; k<bufsize; k++)
	{
		if(lru_array[k] != NULL){
			tarp = findPage(lru_array[k]->pageid, disk_array(lru_array[k]->diskId));
			syncbuf(tarp);
			leavemgr(tarp->pageid, disk_array(tarp->diskId));
		}
	}
	lru = -1;
	mru = -1;
	freebuf = bufsize;
}

void lrubuf::entermgr(LID pageid, iofdctl * disk)             ////在内存为盘号为disk中pageid的页面分配空间
{
	int bufid = allocbufid();
	actual_entermgr(pageid, disk, bufid);
}

void lrubuf::actual_entermgr(LID pageid, iofdctl * disk, int bufid)
{
	getinlru(pageid, disk, bufid);                          //将lru_array[bufid]所指向的lpnode（盘为disk，页号为pageid）插入lru链表的表头
	int hashid = pageid % bufsize;
	npnode * newp = newPage(pageid, bufid, disk);
	npnode * itp;
	if(mgr_array[hashid] == NULL)
		mgr_array[hashid] = newp;
	else
	{
		itp = mgr_array[hashid];
		while(itp->next != NULL)
			itp = itp->next;
		itp->next = newp;
	}
	freebuf--;
}

void lrubuf::leavemgr(LID pageid, iofdctl * disk)   //在内存中删除盘号为disk中pageid的页面
{
	npnode * tarp = findPage(pageid, disk);
	npnode * itp;
	npnode * pre;
	if(tarp == NULL)
	{
		cout<<"Exception---> find exist page fail in "<<className()<<endl;
		exit(0);
	}
	rmlru(tarp->bufid);

	int hashid = pageid % bufsize;
	if(mgr_array[hashid] == NULL)
	{
		cout<<"Error---> mgr_array has problems in "<<className()<<" leavemgr()"<<endl;
		exit(0);
	}
	if(mgr_array[hashid] == tarp)
		mgr_array[hashid] = tarp->next;
	else
	{
		itp = mgr_array[hashid]->next;
		pre = mgr_array[hashid];
		while(itp != NULL)
		{
			if(itp == tarp)
				break;
			pre = itp;
			itp = itp->next;
		}
		if(itp == NULL)
		{
			cout<<"Error---> mgr_array has problems in "<<className()<<" leavemgr()"<<endl;
			exit(0);
		}
		pre->next = itp->next;
	}
	delete tarp;
	freebuf++;
}

npnode * lrubuf::findPage(LID pageid, iofdctl * disk)          //在内存中寻找盘号为disk中pageid的页面
{
	int hashid = pageid % bufsize;
	npnode * tarp;
	npnode * itp;
	if(mgr_array[hashid] == NULL)
	{
		return NULL;
		//cout<<"Error---> mgr_array has problems in "<<className()<<" findPage() with hashid=="<<hashid<<endl;
		//exit(0);
	}
	if(mgr_array[hashid]->pageid == pageid && mgr_array[hashid]->diskId == disk->diskID())
		tarp = mgr_array[hashid];
	else
	{
		itp = mgr_array[hashid]->next;
		while(itp != NULL)
		{
			if(itp->pageid == pageid && itp->diskId == disk->diskID())
				break;
			itp = itp->next;
		}
		/* 
		if(itp == NULL)
		{
			cout<<"Error---> mgr_array has problems in "<<className()<<" findPage() with itp=NULL"<<endl;
			exit(0);
		}*/
		tarp = itp;
	}
	if((tarp!=NULL) && (lru_array[tarp->bufid] == NULL))
	{
		if(lru_array[tarp->bufid]->pageid != pageid)
		{
			cout<<"Error---> lru_array has problems in "<<className()<<" findPage() with lru is not sync"<<endl;
			exit(0);
		}
	}
	return tarp;
}

int lrubuf::allocbufid()               //在lru_array分配一个空间，返回bufid
{
	lpnode * itlp;
	int bufid = -1;;
	int k;
	if(freebuf > 0)
	{
		for(k=0; k<bufsize; k++)
		{
			if(lru_array[k] == NULL)
			{
				bufid = k;
				break;
			}
		}
		if(bufid < 0)
		{
			cout<<"Error---> freebuf is wrong in "<<className()<<"-allocbufid()"<<endl;
			exit(0);
		}
	}
	else if(freebuf == 0)
	{
		bufid = getevict();
		syncbuf(findPage(lru_array[bufid]->pageid, disk_array(lru_array[bufid]->diskId)));
		leavemgr(lru_array[bufid]->pageid, disk_array(lru_array[bufid]->diskId));
	}
	else
	{
		cout<<"Error---> freebuf is wrong in "<<className()<<"-allocbufid()"<<endl;
		exit(0);
	}
	return bufid;
}

int lrubuf::getevict()
{
	return mru;
}

lpnode * lrubuf::newLru(LID pageid, iofdctl * disk)         //生成一个盘为disk，页面号为pageid的lru链表的结点
{
	lpnode * nlru = new lpnode;
	nlru->pageid = pageid;
	nlru->next = -1;
	nlru->pre = -1;
	nlru->diskId = disk->diskID();
	return nlru;
}

void  lrubuf::rmlru(int bufid)                        //删除lru_array[bufid]以及调整lru链
{
	if(lru_array[bufid] == NULL)
	{
		cout<<"Error---> lru_array is wrong in "<<className()<<"-rmlru()"<<endl;
		exit(0);
	}

	int pre;
	int next;

	if(bufid == mru && bufid == lru)
	{
		mru = -1;
		lru = -1;
		delete lru_array[bufid];
		lru_array[bufid] = NULL;
	}
	else if(bufid == mru)
	{
		mru = lru_array[bufid]->pre;
		delete lru_array[bufid];
		lru_array[bufid] = NULL;
	}
	else if(bufid == lru)
	{
		lru = lru_array[bufid]->next;
		delete lru_array[bufid];
		lru_array[bufid] = NULL;
	}
	else
	{
		pre = lru_array[bufid]->pre;
		next = lru_array[bufid]->next;
		lru_array[pre]->next = next;
		lru_array[next]->pre = pre;
		delete lru_array[bufid];
		lru_array[bufid] = NULL;
	}
}

void lrubuf::getinlru(LID pageid, iofdctl * disk, int bufid)         //将lru_array[bufid]所指向的lpnode（盘为disk，页号为pageid）插入lru链表的表头
{
	lpnode * newlp = newLru(pageid, disk);
	lru_array[bufid] = newlp;
	
	if(lru == mru)
	{
		if(lru < 0)   //the lru list is empty
		{
			lru = bufid;
			mru = bufid;
		}
		else if(lru >= 0)  //the lru list has only one element
		{
			lru_array[lru]->pre = bufid;
			lru_array[bufid]->next = lru;
			lru = bufid;
		}
	}
	else
	{
		if(lru < 0)
		{
			cout<<"Error---> lru buf has an error lru id"<<endl;
			exit(0);
		}
		lru_array[lru]->pre = bufid;
		lru_array[bufid]->next = lru;
		lru = bufid;
	}
}

void lrubuf::adjustlru(int bufid)         //把lru_array[bufid]调整到lru链表的表头
{
	LID pageid = lru_array[bufid]->pageid;
	int diskid = lru_array[bufid]->diskId;
	rmlru(bufid);
	getinlru(pageid, disk_array(diskid), bufid);
}

int lrubuf::requestbuf(LID pageid, iofdctl * disk)         //在内存中寻找盘号为disk的pageid的页面，如果没有则读入内存，返回在内存的偏移量
{
	npnode * tarp;
	tarp = findPage(pageid, disk);
	if(tarp == NULL)
	{	
		entermgr(pageid, disk);
		if((tarp = findPage(pageid, disk)) == NULL)
		{
			cout<<"Exception---> findPage fail in bufmgr-requestbuf()"<<endl;
			exit(0);
		}
		disk->diskRead(&pageid, (BYTE*)(buffer+tarp->bufid*PAGE_SIZE), 1);
		io_read_num++;
		if(disk->diskType() == SSD_TYPE)
			_ssd_read_num++;
		else if(disk->diskType() == HDD_TYPE)
			_hdd_read_num++;
	}
	else
		adjustlru(tarp->bufid);
	return (tarp->bufid*PAGE_SIZE);
}
