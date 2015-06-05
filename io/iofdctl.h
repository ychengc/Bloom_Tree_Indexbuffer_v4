#ifndef _IOCTL_H_
#define _IOCTL_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE  
#endif

#include "iofdtype.h"

const IOFD_ACCESSTYPE NORMAL_ACCESS = 0;
const IOFD_ACCESSTYPE DIRECT_ACCESS = 1;

class iofdctl{
	private:
		char * filePath;
		IOFD_LXFILE file;
		const int LOGICSPACE;
		const int disktype;
		const int diskid;
		const int PAGE_SIZE;

		//size都是以页面数目计算
		int bytePhySize;  //变量以字节衡量
		int fullSize;
		int freeSize;
		const int bufsize;

		IOFDfreespace freeSpaceList;  //空闲表管理--连续空闲空间排序
		IOFDfreeaddr freeAddrList;   //为垃圾回收做空闲表排序
		IOFDlid_pid LIDmPID;
		IOFDlid_pid PIDmLID;
		
		int loadDisk();
		void fullVacuum();
		void lazyVacuum();
		void clearDoubleList();
		void vacuumMap(IOFD_LID id1, IOFD_PID id2);
	public:
		iofdctl(char * path, int storeSize, int dtype, int pagesize, int dID);
		~iofdctl();
		void diskOpen(IOFD_ACCESSTYPE access);
		void diskClose();

		int diskType();
		int diskID();

		int last_locate;
		
		void allocPage(IOFD_LID *plid, int pagenum);
		IOFD_PID allocRawPage(IOFD_LID* pageid, int pagenum);

		void rawRead(IOFD_PID phyid, BYTE * buf, int bytesize);
		void rawWrite(IOFD_PID phyid, BYTE * buf, int bytesize);
		void diskRead(IOFD_LID *pageid, BYTE * buf, int pagenum);
		void diskWrite(IOFD_LID* pageid, BYTE * buf, int pagenum);	
		void delPage(IOFD_LID * pageid, int pagenum);
		int existPage(IOFD_LID pageid);
		int isFreeSpace(int pagenum);
		void clearAll();
};

#endif
