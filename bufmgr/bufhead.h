#ifndef _BUF_HEAD_H
#define _BUF_HEAD_H

#define LID IOFD_LID

typedef struct nPageNode
{ 
	LID pageid;  // page id 
	int bufid;
	int isDirty;
	DISK_TYPE locate;  
	int diskId;
	struct nPageNode* next;
}npnode;

#endif
