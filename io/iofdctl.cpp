#include "iofdctl.h"
#include "diskhead.h"
#include <cstring>
#include <iostream>
#include <stdio.h>
using namespace std;

iofdctl::iofdctl(char * path, int storeSize, int dtype, int pagesize, int dID):bufsize(1024), LOGICSPACE(INT_MAX), disktype(dtype), PAGE_SIZE(pagesize), diskid(dID)
{
	fullSize = storeSize;
	freeSize = fullSize;
	filePath = new char[100];
	sprintf(filePath, "%s", path);
	IOFD_FREESPACE * initFree = new IOFD_FREESPACE;
	initFree->baseAddr = 0;
	initFree->numOffset = fullSize;
	freeSpaceList.insert(multimap<int, IOFD_FREESPACE*>::value_type(initFree->numOffset, initFree));
	freeAddrList.insert(map<int, IOFD_FREESPACE*>::value_type(initFree->baseAddr, initFree));
	//cout<<"Inital---> Disk-"<<dID<<" is loading......"<<endl;
	loadDisk();
	last_locate = fullSize * pagesize; //mark the last byte location of this file
	//cout<<"Inital is finish, disk-"<<dID<<" is ready to work for you"<<endl;
}

int iofdctl::loadDisk()
{
	file = open(filePath, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR);
	int cursize = lseek(file, 0, SEEK_END);   //�����µ��ļ�ƫ������ĩβ��
	if(cursize >= fullSize*PAGE_SIZE){
		close(file);
		bytePhySize = cursize;
		return 1;
	}
	close(file);
 
	BYTE *buf;
	buf = new BYTE[bufsize*PAGE_SIZE];
	memset(buf, -1, PAGE_SIZE*bufsize);
	int scale = fullSize;
	int maxpos;
	diskOpen(NORMAL_ACCESS);
	while(scale > 0){
		maxpos = (scale<bufsize) ? scale:bufsize;
		if(write(file, buf, maxpos*PAGE_SIZE) != maxpos*PAGE_SIZE){
			delete buf;
			cout<<"Exception--->write in loadisk -- iofdctl"<<endl;
			exit(0);
		}
		scale -= maxpos;
	}
	fsync(file);   //fsync����ͬ���ڴ����������޸ĵ��ļ���ݵ������豸��fsync ϵͳ���ÿ���ʹ��ȷ��ǿ��ÿ��д�붼�����µ�������
	bytePhySize = lseek(file, 0, SEEK_END);
	diskClose();
	delete []buf;
	if(bytePhySize < fullSize*PAGE_SIZE)
	{
		cout<<"Exception--->bytePhySize error -- iofdctl"<<endl;
		exit(0);
	}
	return 1;
 }

int iofdctl::diskType()
{
	return disktype;
}

int iofdctl::diskID()
{
	return diskid;
}

iofdctl::~iofdctl()                               //������
{
	IOFDfreeaddr::iterator it;
	for(it=freeAddrList.begin(); it!=freeAddrList.end(); it++)
	  delete it->second;
	freeSpaceList.clear();
	freeAddrList.clear();
	LIDmPID.clear();
	PIDmLID.clear();
}

IOFD_PID iofdctl::allocRawPage(IOFD_LID* pageid, int pagenum)     //����pagenum��ҳ�棬��Щҳ����߼���ַ�Ѿ�����pageid��
{
	pair<IOFDfreespace::iterator, IOFDfreespace::iterator> multipr;
	IOFDfreespace::iterator itp; 
	IOFD_FREESPACE * allocSpace;
	itp = freeSpaceList.lower_bound(pagenum);   //����һ���������ָ���С��pagenum�ĵ�һ��Ԫ��
	int tPid = -1;

	if(freeSize < pagenum)
	{
		allocSpace = NULL;
		tPid = last_locate/PAGE_SIZE;;
		fullSize += pagenum;
		last_locate += pagenum * PAGE_SIZE;
	}
	else
	{
		if(itp == freeSpaceList.end()){
			if(((float)freeSize)/fullSize <= 0.8)
			{
				allocSpace = NULL;
				tPid = last_locate/PAGE_SIZE;;
				fullSize += pagenum;
				last_locate += pagenum * PAGE_SIZE;
			}
			else
			{
				lazyVacuum();
				cout<<"lazyVacuum"<<endl;
				itp = freeSpaceList.lower_bound(pagenum);
				if(itp == freeSpaceList.end()){
					fullVacuum();
					cout<<"fullVacuum"<<endl;
					itp = freeSpaceList.lower_bound(pagenum);
				}
			}
		}
		if(tPid < 0)
		{
			allocSpace = itp->second;
			tPid = allocSpace->baseAddr;
			freeSpaceList.erase(itp);
			freeAddrList.erase(allocSpace->baseAddr);
	
			if(allocSpace->numOffset > pagenum){
				allocSpace->baseAddr = allocSpace->baseAddr+pagenum;
				allocSpace->numOffset = allocSpace->numOffset-pagenum;
				freeSpaceList.insert(IOFDfreespace::value_type(allocSpace->numOffset, allocSpace));
				freeAddrList.insert(IOFDfreeaddr::value_type(allocSpace->baseAddr, allocSpace));
			}
			else
				delete allocSpace;
		}
	}

	for(int i=0; i<pagenum; i++){
		LIDmPID.insert(IOFDlid_pid::value_type(pageid[i], tPid+i));
		PIDmLID.insert(IOFDlid_pid::value_type(tPid+i, pageid[i]));
	}
	freeSize -= pagenum;
	return tPid; 
}

void iofdctl::allocPage(IOFD_LID * plid, int pagenum)    //ҳ����䣬����pagenum��ҳ�棬��Щҳ���߼���ַ�����䲢�Ҵ���plid�У������plid���������ַ
{
	int temp; 
	temp = rand()%(LOGICSPACE-pagenum);
	for(int i=0; i<pagenum; i++){
		while(LIDmPID.find(temp) != LIDmPID.end())
			temp = rand()%(LOGICSPACE-pagenum);
		plid[i] = temp;
		temp++;
	}
	allocRawPage(plid, pagenum);
}

void iofdctl::lazyVacuum()      //������ղ�����freeAddrList��freeSpaceList�Ĳ���
{
	IOFDfreeaddr::iterator itpr;
	IOFDfreeaddr::iterator nextpr;
	IOFD_FREESPACE * curpos;
	IOFD_FREESPACE * nextpos;
	freeSpaceList.clear();
	for(itpr=freeAddrList.begin(); itpr!=freeAddrList.end();){
		curpos = itpr->second;
		nextpr = (itpr++);
		itpr--;
		if(nextpr == freeAddrList.end()){
			freeSpaceList.insert(IOFDfreespace::value_type(curpos->numOffset, curpos));
			itpr++;
			continue;
		}
		nextpos = nextpr->second;
		while(curpos->baseAddr+curpos->numOffset == nextpos->baseAddr){
			curpos->numOffset += nextpos->numOffset;
			freeAddrList.erase(nextpr++);
			if(nextpr == freeAddrList.end())
				break;
			nextpos = nextpr->second;
		}
		freeSpaceList.insert(IOFDfreespace::value_type(curpos->numOffset, curpos));
		itpr = nextpr;
	}
}

void iofdctl::vacuumMap(IOFD_LID id1, IOFD_PID id2)       //��ַӳ����?id1ӳ�䵽id2
{
	IOFDlid_pid::iterator lpit;
	PIDmLID.insert(IOFDlid_pid::value_type(id2, id1));
	lpit = LIDmPID.find(id1); 
	if(lpit == LIDmPID.end())
	{
		cout<<"Exception--->vacuumMap fail -- iofdctl"<<endl;
		exit(0);
	}
	lpit->second = id2;
}

void iofdctl::clearDoubleList()         //ɾ������list
{
	freeSpaceList.clear();
	IOFDfreeaddr::iterator addrit;
	for(addrit=freeAddrList.begin(); addrit!=freeAddrList.end(); addrit++)
		delete addrit->second;
	freeAddrList.clear();
}

void iofdctl::fullVacuum()     //������ղ���
{
	IOFDlid_pid tempPL;       
	tempPL.insert(PIDmLID.begin(), PIDmLID.end());     //����start��end��Ԫ��
	IOFDfreeaddr::iterator addrItp;
	IOFDlid_pid::iterator plItp;
	IOFDlid_pid::iterator plnextItp;
	int seqnum;
	IOFD_PID rpoint, wpoint;
	int validnum = fullSize-freeSize;
	if(validnum <= 0){
		cout<<"fullVacuum error---> fullsize-freesize"<<endl;
		exit(0);
	}
	addrItp = freeAddrList.begin();
	plItp = tempPL.begin();
	BYTE *buf;
	int bufpos;
	if(validnum < bufsize)
		buf = new BYTE[validnum*PAGE_SIZE];
	else
		buf = new BYTE[bufsize*PAGE_SIZE];

	wpoint = addrItp->first;
	while(plItp->first <= wpoint){
		if(plItp->first == wpoint){
			cout<<"Exception--->freeAddrList and PIDmLID"<<endl;
			exit(0);
		}
		plItp++;
	}
	PIDmLID.clear();
	diskOpen(NORMAL_ACCESS);
	while(validnum>0){
		int maxpos = (validnum<bufsize) ? validnum:bufsize;
		for(bufpos = 0; bufpos<maxpos;){
			plnextItp = (plItp++);
			plItp--;

			seqnum = 1;
			vacuumMap(plItp->second, wpoint);

			while(plnextItp->first-plItp->first == seqnum){
				vacuumMap(plnextItp->second, wpoint+seqnum);
				seqnum++;
				plnextItp++;
			}
			rawRead(plItp->first,buf+bufpos*PAGE_SIZE, PAGE_SIZE*seqnum);
			rawWrite(wpoint, buf+bufpos*PAGE_SIZE, PAGE_SIZE*seqnum);
			plItp = plnextItp;
			wpoint += seqnum;
			bufpos += seqnum;
		}
		validnum -= maxpos;
	}
	delete [] buf;
	diskClose();
	clearDoubleList();
	IOFD_FREESPACE * finalFree = new IOFD_FREESPACE;
	finalFree->baseAddr = wpoint;
	finalFree->numOffset = freeSize;
	freeAddrList.insert(IOFDfreeaddr::value_type(finalFree->baseAddr, finalFree));
	freeSpaceList.insert(IOFDfreespace::value_type(finalFree->numOffset, finalFree));
}

void iofdctl::diskOpen(IOFD_ACCESSTYPE access)
{
	int flag;
	if(access == NORMAL_ACCESS)
	  flag = O_CREAT|O_RDWR;
	else if(access == DIRECT_ACCESS)
	  flag = O_CREAT|O_RDWR|O_DIRECT;
	else{
		cout<<"open file fail"<<endl;
		exit(0);
	}
	if((file = open(filePath, flag, S_IRUSR | S_IWUSR)) <0){
		cout<<"Exception--->open disk fail"<<endl;
		exit(0);
	}
}

void iofdctl::diskClose()
{
	if(close(file) == -1){
		cout<<"Exception---> disk close fail"<<endl;
		exit(0);
	}
}

void iofdctl::rawRead(IOFD_PID phyid, BYTE * buf, int bytesize)   //��phyidҳ���bytesize����ݵ�buf
{
	if(bytePhySize < phyid*PAGE_SIZE){
		cout<<"Exception---> disk overflow in rawRead:bytePhySize="<<bytePhySize<<"; phyid*PAGE_SIZE="<<phyid<<"*"<<PAGE_SIZE<<endl;
		exit(0);
	}

	if(lseek(file, phyid*PAGE_SIZE, SEEK_SET) < 0){
		cout<<"Exception--->lseek fail in rawRead()"<<endl;
		exit(0);
	}
	int realsize;
	if((realsize = read(file, buf, bytesize)) != bytesize){
		cout<<"Error---> rawRead byte size is not fit---"<<realsize<<" & "<<bytesize<<endl;
		exit(0);
	}
}

void iofdctl::rawWrite(IOFD_PID phyid, BYTE * buf, int bytesize)    //��buf��bytesize�����д��phyid��ҳ��
{
	if(bytePhySize < phyid*PAGE_SIZE){
		cout<<"Exception---> disk overflow in rawWrite:bytePhySize="<<bytePhySize<<"; phyid*PAGE_SIZE="<<phyid<<"*"<<PAGE_SIZE<<endl;
		exit(0);
	}
	if(lseek(file, phyid*PAGE_SIZE, SEEK_SET) < 0){
		cout<<"Exception--->lseek fail in rawWrite()"<<endl;
		exit(0);
	}
	int realsize;
	if((realsize=write(file, (const void*)buf, bytesize)) != bytesize){
		cout<<"Error---> rawWrite byte size is not fit---"<<realsize<<" & "<<bytesize<<endl;
		exit(0);
	}
}

void iofdctl::diskRead(IOFD_LID* pageid, BYTE * buf, int pagenum)           //�Ӵ�����ֱ�ӣ�DIRECT_ACCESS����pageid��ʶ��pagenum��ҳ�����buf
{
	int pos;
	IOFD_PID *phyid = new IOFD_PID[pagenum];
	IOFD_PID tempid;
	IOFDlid_pid::iterator lpit; 
	for(int i=0; i<pagenum; i++){
		if((lpit = LIDmPID.find(pageid[i])) == LIDmPID.end()){
			cout<<"Exception--->Page LID-"<<pageid[i]<<" is not exist"<<endl;
			exit(0);
		}
		phyid[i] = lpit->second;
	}
	int sum;
	diskOpen(NORMAL_ACCESS);
	for(int i=0; i<pagenum;){
		if(i == 0){
			tempid = phyid[i];
			i++;
			pos = 0;
			sum = 1;
		}
		else{
			if(phyid[i]-tempid == sum){
				i++;
				sum++;
			}
			else{
				rawRead(tempid, buf+pos*PAGE_SIZE, sum*PAGE_SIZE);
				pos = i;
				tempid = phyid[i];
				i++;
				sum = 1;
			}
		}
		if(i == pagenum){
			rawRead(tempid, buf+pos*PAGE_SIZE, sum*PAGE_SIZE);
		}
	}
	diskClose();
	delete []phyid;
}

void iofdctl::diskWrite(IOFD_LID* pageid, BYTE * buf, int pagenum)   //�Ӵ�����ֱ�ӣ�DIRECT_ACCESS����buf�����д��pageid��ʶ��pagenum��ҳ��
{
	IOFD_PID * phyid = new IOFD_PID[pagenum];
	IOFDlid_pid::iterator lpitr;
	IOFD_PID tempid;
	int pos;
	for(int i=0; i<pagenum; i++){
		if((lpitr = LIDmPID.find(pageid[i])) == LIDmPID.end()){
			cout<<"Exception--->Page LID-"<<pageid[i]<<" is not alloc"<<endl;
			exit(0);
		}
		phyid[i] = lpitr->second;
	}

	int sum;
	diskOpen(NORMAL_ACCESS);
	for(int i=0; i<pagenum;){
		if(i == 0){
			tempid = phyid[i];
			i++;
			pos = 0;
			sum = 1;
		}
		else{
			if(phyid[i]-tempid == sum){
				i++;
				sum++;
			}
			else{
				rawWrite(tempid, buf+pos*PAGE_SIZE, sum*PAGE_SIZE);
				pos = i;
				tempid = phyid[i];
				sum = 1;
				i++;
			}
		}
		if(i == pagenum){
			rawWrite(tempid, buf+pos*PAGE_SIZE, sum*PAGE_SIZE);
		}
	}
	diskClose();
	delete []phyid;
}

int iofdctl::existPage(IOFD_LID pageid)           //pageid�Ƿ����
{
	if(LIDmPID.find(pageid) == LIDmPID.end())
		return -1;
	return 1;
}

int iofdctl::isFreeSpace(int pagenum)           //�Ƿ���pagenum�����пռ�
{
	if(pagenum > freeSize)
		return -1;
	return 1;
}

void iofdctl::delPage(IOFD_LID * pageid, int pagenum)     //ɾ��pageid�е�pagenum��ҳ��
{
	IOFDlid_pid::iterator lpit;
	vector<IOFD_PID> pidv;
	for(int i=0; i<pagenum; i++){
		if((lpit=LIDmPID.find(pageid[i])) == LIDmPID.end()){
			cout<<"Exception---> delete a page not existing"<<endl;
			exit(0);
		}
		pidv.push_back(lpit->second);
		PIDmLID.erase(lpit->second);
		LIDmPID.erase(lpit);
	}
	sort(pidv.begin(), pidv.end());
	vector<IOFD_PID>::iterator pidit;
	int tempid;
	int sum;
	for(pidit=pidv.begin(); pidit!=pidv.end();){
		if(pidit == pidv.begin()){
			sum = 1;
			tempid = *pidit;
			pidit++;
		}
		else{
			if(*pidit-tempid == sum){
				sum++;
				pidit++;
			}
			else{
				IOFD_FREESPACE * alloc = new IOFD_FREESPACE;
				alloc->baseAddr = tempid;
				alloc->numOffset = sum;
				freeSpaceList.insert(IOFDfreespace::value_type(sum, alloc));
				freeAddrList.insert(IOFDfreeaddr::value_type(tempid, alloc));

				sum = 1;
				tempid = *pidit;
				pidit++;
			}
			if(pidit == pidv.end()){
				IOFD_FREESPACE * alloc = new IOFD_FREESPACE;
				alloc->baseAddr = tempid;
				alloc->numOffset = sum;
				freeSpaceList.insert(IOFDfreespace::value_type(sum, alloc));
				freeAddrList.insert(IOFDfreeaddr::value_type(tempid, alloc));
			}
		}
	}
}

void iofdctl::clearAll ()  //ɾ�����е�ҳ��
{
	freeSize = fullSize;
	IOFDfreeaddr::iterator it;
	for(it=freeAddrList.begin(); it!=freeAddrList.end(); it++)
	  delete it->second;
	freeSpaceList.clear();
	freeAddrList.clear();
	LIDmPID.clear();
	PIDmLID.clear();

	IOFD_FREESPACE * initFree = new IOFD_FREESPACE;
	initFree->baseAddr = 0;
	initFree->numOffset = fullSize;
	freeSpaceList.insert(multimap<int, IOFD_FREESPACE*>::value_type(initFree->numOffset, initFree));
	freeAddrList.insert(map<int, IOFD_FREESPACE*>::value_type(initFree->baseAddr, initFree));

}		/* -----  end of method iofdctl::clearAll  ----- */

