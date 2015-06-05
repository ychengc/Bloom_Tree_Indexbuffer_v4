#include "nobuf.h"
#include "bufv.h"
nobuf::nobuf(int bufsz, int page_size):bufmgr(bufsz, page_size)
{
	pagelist = NULL;
	bufarray = new int[bufsize];
	int k;
	for(k=0; k<bufsize; k++)
	  bufarray[k] = -1;
}

nobuf::~nobuf()
{
	npnode * iterp;
	npnode * nextp;
	iterp = pagelist;
	while(iterp != NULL)
	{
		nextp = iterp->next;
		delete iterp;
		iterp = nextp;
	}
	pagelist = NULL;
	delete bufarray;
}

void nobuf::clearbuf()     //��ջ�����
{
	npnode * iterp;
	npnode * nextp;
	iterp = pagelist;
	while(iterp != NULL)
	{
		nextp = iterp->next;
		syncbuf(iterp);
		leavemgr(iterp->pageid, disk_array(iterp->diskId));
		iterp = nextp;
	}
	pagelist = NULL;
	int k;
	for(k=0; k<bufsize; k++)
		bufarray[k] = -1;
}

npnode * nobuf::findPage(LID pageid, iofdctl * disk)              //���ڴ���Ѱ���̺�Ϊdisk��pageid��ҳ��
{
	npnode * iterp;
	iterp = pagelist;
	while(iterp != NULL)
	{
		if(iterp->pageid == pageid && iterp->diskId == disk->diskID())
		{
			if(bufarray[iterp->bufid] != pageid)
			{
				cout<<"Exception--->bufmagr has some wrong in "<<className()<<"-findPage()"<<endl;
				exit(0);
			}
			else
				return iterp;
		}
		iterp = iterp->next;
	}
	return NULL;
}

void nobuf::leavemgr(LID pageid, iofdctl * disk)    //���ڴ���ɾ���̺�Ϊdisk��pageid��ҳ��
{
	npnode * iterp;
	npnode * pre;
	iterp = pagelist;
	while(iterp->pageid != pageid || iterp->diskId != disk->diskID())
	{
		pre = iterp;
		iterp = iterp->next;
		if(iterp == NULL)
		{
			cout<<"Exception--->remove a page not exist in "<<className()<<"-leavemgr()"<<endl;
			exit(0);
		}
	}
	if(iterp == pagelist)
		pagelist = iterp->next;
	else
		pre->next = iterp->next;
	bufarray[iterp->bufid] = -1;
	delete iterp;
}

void nobuf::entermgr(LID pageid, iofdctl * disk)          //���ڴ�Ϊ�̺�Ϊdisk��pageid��ҳ�����ռ�
{
	int bufid;
	int k;
	for(k=0; k<bufsize; k++)
	{
		if(bufarray[k] < 0)
		{
			bufid = k;
			break;
		}
	}
	actual_entermgr(pageid, disk, bufid);
}

void nobuf::actual_entermgr(LID pageid, iofdctl * disk, int bufid)
{
	npnode * itp = newPage(pageid, bufid, disk);
	bufarray[bufid] = pageid;
	itp->next = pagelist;
	pagelist = itp; 
}

char * nobuf::className()
{
	return "nobuf_c";
}

int nobuf::requestbuf(LID pageid, iofdctl * disk)           //���ڴ���Ѱ���̺�Ϊdisk��pageid��ҳ�棬���û��������ڴ棬�������ڴ��ƫ����
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
	return (tarp->bufid*PAGE_SIZE);
}
