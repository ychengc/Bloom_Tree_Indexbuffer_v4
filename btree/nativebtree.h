#ifndef _NATIVE_BTREE_H
#define _NATIVE_BTREE_H

#include "btree.h"
#include "btreestruct.h"
#include <iostream>
#include <cstring>
#include <iomanip>
#include <fstream>
using namespace std;
#define PAGE_SIZE 4096

template <typename BCELL, typename KEY_TYPE>
class nativebtree:public btree<KEY_TYPE>
{
	public:
		nativebtree(bufmgr * ctl, iofdctl * dd, int key_num_up);
		~nativebtree();
		
		int searchKey(KEY_TYPE key);
		void insertKey(KEY_TYPE key, VALUE_TYPE value);
		void deleteKey(KEY_TYPE key);
		void rangesearchKey(KEY_TYPE keymin,KEY_TYPE keymax);
		int updateKey(KEY_TYPE key, VALUE_TYPE value);
		LID getRoot();
		char * className();
		int getHeight();
		void printtree(LID node,int level);
	protected:
		bufmgr * bufctl;
		LID root_pid;
		int high;
		iofdctl * disk;
		const int KEY_NUM;
		
		//assist function
		int getKeyNum(LID node_pid);
		KEY_TYPE getKeyLocate(LID node_pid, int scale);
		int isNodeFull(LID node_pid);
		LID applynode();

		VALUE_TYPE searchKeyIn(KEY_TYPE key, LID curroot, int level);
		
		//insert function
		void insertNonFullLeaf(KEY_TYPE key, VALUE_TYPE value, LID node_pid);
		void insertNonFullInner(KEY_TYPE key, int locate, LID lson_pid, LID rson_pid, LID node_pid);
		bool splitLeaf(LID curnode_pid, LID & lbro_pid, LID & rbro_pid);     //split leaf node
        bool splitInner(LID curnode_pid, KEY_TYPE & splitKey, LID & lbro_pid, LID & rbro_pid);    
        bool insertNode(KEY_TYPE key, VALUE_TYPE value, LID curnode_pid, LID & lbro_pid, KEY_TYPE & splitKey, LID & rbro_pid, int level);

		//remove function
		KEY_TYPE easeRemoveKey(KEY_TYPE key, LID node_pid, LID & target_pid, int level);
		TREE_STATE_TYPE AfterRemoveKey(LID & target_pid, LID curnode_pid, int level);
		TREE_STATE_TYPE handlHelpNode(LID son_pid, int locateson, LID parent_pid, int level);
		void removeKeyWithLeftSonNonMergeInner(int locate, LID cur_pid);
		void removeKeyWithRightSonNonMergeInner(int locate, LID cur_pid);
		int removeKeyNonMergeLeaf(KEY_TYPE key, LID cur_pid);
		KEY_TYPE unitNonDelLeaf(LID leaf_pid, LID helpBro_pid);
		void unitNonDelInner(LID target_pid, int locateTarget, LID help_pid, LID parent_pid);
		KEY_TYPE mergeWithDelLeaf(LID small_pid, LID big_pid);
		void mergeWithDelInner(LID small_pid, LID big_pid, int keyLocate, LID parent_pid);
};


//------------------------------------------------------------------------------------------------------------------------
template <typename BCELL, typename KEY_TYPE>
nativebtree<BCELL, KEY_TYPE>::nativebtree(bufmgr * ctl, iofdctl * dd, int key_num_up):KEY_NUM(key_num_up)
{
	bufctl = ctl;
	high = 0;
	root_pid = -1;
	disk = dd;
}

template <typename BCELL, typename KEY_TYPE>
nativebtree<BCELL, KEY_TYPE>::~nativebtree()
{
	//delete disk;
}

template <typename BCELL, typename KEY_TYPE>
LID nativebtree<BCELL, KEY_TYPE>::getRoot()
{
	return root_pid;
}

template <typename BCELL, typename KEY_TYPE>
char * nativebtree<BCELL, KEY_TYPE>::className()
{
	return "native btree";
}

template <typename BCELL, typename KEY_TYPE>
VALUE_TYPE nativebtree<BCELL, KEY_TYPE>::searchKey(KEY_TYPE key)
{
	//int level = 0;
	if(root_pid < 0)
		return NONTHING;	
	return searchKeyIn(key, root_pid, -1);
}

template <typename BCELL, typename KEY_TYPE>
int nativebtree<BCELL, KEY_TYPE>::getKeyNum(LID node_pid)   //return key count of node
{
	BCELL * curcell = (BCELL*)(bufctl->readbuf(node_pid, disk));
	return curcell->cur_knum;
}

template <typename BCELL, typename KEY_TYPE>
KEY_TYPE nativebtree<BCELL, KEY_TYPE>::getKeyLocate(LID node_pid, int scale)
{
	BCELL * curcell = (BCELL*)(bufctl->readbuf(node_pid, disk));
	return curcell->key[scale];
}

template <typename BCELL, typename KEY_TYPE>   //decide state of node
int nativebtree<BCELL, KEY_TYPE>::isNodeFull(LID node_pid)
{
	int num = getKeyNum(node_pid);
	if(num < KEY_NUM-1)
		return NODE_NOFULL;
	else if(num == KEY_NUM-1)
		return NODE_SPLIT_FULL;
	else if(num == KEY_NUM)
		return NODE_SPACE_FULL;
	else
	{
		cout<<"Error--->node key number has problem - "<<className()<<" isNodeFull()"<<endl;
		exit(0);
	}
}

template <typename BCELL, typename KEY_TYPE>
LID nativebtree<BCELL, KEY_TYPE>::applynode()     //apply a new node,return lid of page
{
	LID new_pid = bufctl->applypage(disk); 
	BCELL * new_cell = (BCELL*)(bufctl->readbuf(new_pid, disk)); 
	memset(new_cell, NONTHING, PAGE_SIZE);
	new_cell->cur_knum=0;
	*((BYTE*)new_cell+PAGE_SIZE-1) = 0;
	*((BYTE*)new_cell+PAGE_SIZE-2) = 1;   //it means we apply a new page has no overflow
	bufctl->writebuf(new_pid, disk);
	return new_pid;
}

template <typename BCELL, typename KEY_TYPE>
VALUE_TYPE nativebtree<BCELL, KEY_TYPE>::searchKeyIn(KEY_TYPE key, LID curroot, int level)
{
	int k = 0;
	level++;
	BCELL * curcell;

	curcell = (BCELL*)(bufctl->readbuf(curroot, disk));

	if(level == (high-1))  //curroot is leaf node
	{
		while(curcell->key[k] != NONTHING && k<KEY_NUM)
		{
			if(curcell->key[k] == key)
				return curcell->next[k];
			k++;
		}
		return NONTHING;
	}
	else
	{
		while(curcell->key[k] != NONTHING)
		{
			if(curcell->key[k] > key && k<KEY_NUM)
			{
				if(curcell->next[k] == NONTHING)
				{
					cout<<"Error---> key has no next-"<<className()<<endl;
					exit(0);
				}
				return searchKeyIn(key, curcell->next[k], level);
			}
			k++;
			if(k == KEY_NUM)
				break;
		}
		return searchKeyIn(key, curcell->next[k], level);
	}
}

template <typename BCELL, typename KEY_TYPE>
void nativebtree<BCELL, KEY_TYPE>::insertKey(KEY_TYPE key, VALUE_TYPE value)
{
	LID aNewNode;
    LID nextLson;
    LID nextRson;
    LID rbro;
    LID lbro;
	int iLocate;
	LID nextNode;
	int i;
	int num;
	BCELL * rootcell;
	KEY_TYPE nextSplitK;
	KEY_TYPE splitKey;
	int level = 0;
	
	if(root_pid < 0)
	{
		aNewNode = applynode();
		insertNonFullLeaf(key, value, aNewNode);
		root_pid = aNewNode;
		high++;
	}
	else
	{
		if(high == 1)
		{
			if(getKeyNum(root_pid) < KEY_NUM-1)
				insertNonFullLeaf(key, value, root_pid);
			else
			{
				aNewNode = applynode();
				insertNonFullLeaf(key, value, root_pid);
				splitLeaf(root_pid, lbro, rbro);
				_split_leaf_num++;
				insertNonFullInner(getKeyLocate(rbro, 0), -1, lbro, rbro, aNewNode);
				root_pid = aNewNode;
				high++;
			}
		}
		else 
		{
			num = getKeyNum(root_pid);
			rootcell = (BCELL*)(bufctl->readbuf(root_pid, disk));
			
			for(i=0; i<num; i++)
			{
				if(key < rootcell->key[i])
					break;
			}
			iLocate = i;
			nextNode = rootcell->next[i];
			if(insertNode(key, value, nextNode, nextLson, nextSplitK, nextRson, level)==SPLIT)
			{
				insertNonFullInner(nextSplitK, iLocate, nextLson, nextRson, root_pid);
				if(isNodeFull(root_pid) == NODE_SPACE_FULL)
				{
					_split_intree_num++;
					aNewNode = applynode();
					splitInner(root_pid, splitKey, lbro, rbro);
                    insertNonFullInner(splitKey, -1, lbro, rbro, aNewNode);
					root_pid=aNewNode;
					high++;
				}
			}
		}
	}
}

template <typename BCELL, typename KEY_TYPE>
void nativebtree<BCELL, KEY_TYPE>::deleteKey(KEY_TYPE key)
{
	//int level = 0;
	if(root_pid == -1)
	{
		cout<<"B+ tree is empty, no delete-"<<className()<<endl;
		exit(0);
	}
	LID target_pid;
	easeRemoveKey(key, root_pid, target_pid, -1);
	AfterRemoveKey(target_pid, root_pid, -1);
}

template <typename BCELL, typename KEY_TYPE>
void nativebtree<BCELL, KEY_TYPE>::rangesearchKey(KEY_TYPE keymin,KEY_TYPE keymax ){
	//we will implement it in bloomtree.h
     return;
  }

template <typename BCELL, typename KEY_TYPE>
int nativebtree<BCELL, KEY_TYPE>::updateKey(KEY_TYPE key, VALUE_TYPE value)
{
	deleteKey(key);
	insertKey(key, value);
}

template <typename BCELL, typename KEY_TYPE>
bool nativebtree<BCELL, KEY_TYPE>::insertNode(KEY_TYPE key, VALUE_TYPE value, LID curnode_pid, LID & lbro_pid, KEY_TYPE & splitKey, LID & rbro_pid, int level)
{	
	level++;
	int ilocate;
	LID next_pid;
	LID nextl_pid;
	LID nextr_pid;
	KEY_TYPE nextSplitKey;
	int state;
	int k;
	BCELL * curcell;

	if(level == (high-1))    //leaf
	{
		state = isNodeFull(curnode_pid);
		if(state == NODE_NOFULL)
		{
			insertNonFullLeaf(key, value, curnode_pid);
			return NO_SPLIT;
		}
		else if(state == NODE_SPLIT_FULL)
		{
			_split_leaf_num++;
			insertNonFullLeaf(key, value, curnode_pid);
			splitLeaf(curnode_pid, lbro_pid, rbro_pid);
			splitKey = getKeyLocate(rbro_pid, 0);
			return SPLIT;
		}
		else
		{
			cout<<"Error--->insert node is wrong "<<className()<<" insertNode()"<<endl;
			exit(0);
		}
	}
	else
	{
		int num = getKeyNum(curnode_pid);
		curcell = (BCELL*)(bufctl->readbuf(curnode_pid, disk));

		for(k=0; k<num; k++)
		{
			if(key < curcell->key[k])
				break;
		}
		ilocate = k;
		next_pid = curcell->next[k];
		if(insertNode(key, value, next_pid, nextl_pid, nextSplitKey, nextr_pid, level)==NO_SPLIT)
			return NO_SPLIT;
		else
		{
			insertNonFullInner(nextSplitKey, ilocate, nextl_pid, nextr_pid, curnode_pid);
			if(isNodeFull(curnode_pid) == NODE_SPACE_FULL)
			{
				_split_intree_num++;
				splitInner(curnode_pid, splitKey, lbro_pid, rbro_pid);
				return SPLIT;
			}
			else
				return NO_SPLIT;
		}
	}
}

template <typename BCELL, typename KEY_TYPE>
void nativebtree<BCELL, KEY_TYPE>::insertNonFullLeaf(KEY_TYPE key, VALUE_TYPE value, LID node_pid)
{
    int low=0;
	int num = getKeyNum(node_pid);
	BYTE *b=bufctl->readbuf(node_pid, disk);
	/*BYTE *t= (BYTE *)malloc(PAGE_SIZE);
	memcpy(t,b,PAGE_SIZE);*/
	BCELL * curcell = (BCELL*)(b);
   int high=0;
   high=num-1;
	int middle=0;
	while(low<=high){
		middle=(low+high)/2;
		if(curcell->key[middle]==key)
			return;
		else if(curcell->key[middle]>key)
			high=middle-1;
		else
			low=middle+1;
	}
	if(low==num){
		curcell->key[low] = key;
		curcell->next[low] = value;
		curcell->cur_knum++;
		bufctl->writebuf(node_pid, disk);
	}
	else{
         int move_key_num=num-low;
         BYTE *b1,*b2;
         //==============move key====================
         b1=b+low*sizeof(KEY_TYPE);
         b2=b1+sizeof(KEY_TYPE);
         memmove(b2,b1,sizeof(KEY_TYPE)*move_key_num);
         //==============move value==================
         b1=b+KEY_NUM*sizeof(KEY_TYPE)+low*sizeof(int);
         b2=b1+sizeof(int);
         memmove(b2,b1,sizeof(int)*move_key_num);
         //=============insert key value================
 		curcell->key[low] = key;
 		curcell->next[low] = value;
 		curcell->cur_knum++;
 		bufctl->writebuf(node_pid, disk);
	}
/*	//=================================================
	curcell = (BCELL*)(t);
	num=curcell->cur_knum;
	int i; int j;
	for(i=0; i<num; i++)
	{
		if(key <= curcell->key[i])
		{
			if(key == curcell->key[i])
			{
				//curcell->next[i] = value;
				//bufctl->writebuf(node_pid, disk);
				return;
			}
			for(j=num; j>i; j--)
			{
				curcell->key[j] = curcell->key[j-1];
				curcell->next[j] = curcell->next[j-1];
			}
			break;
		}
	}
	curcell->key[i] = key;
	curcell->next[i] = value;
	curcell->cur_knum++;
	BCELL * curcell2=(BCELL*)(b);
    if(curcell->cur_knum!=curcell2->cur_knum){
    	cout<<"error in curcell->cur_knum "<<endl;
    	exit(0);
    }
    for(i=0;i<curcell->cur_knum;i++){
    	if(curcell->key[i]!=curcell2->key[i]){
        	cout<<"error in curcell->cur_key"<<i<<endl;
        	exit(0);
    	}
    	if(curcell->next[i]!=curcell2->next[i]){
        	cout<<"error in curcell->next"<<i<<endl;
        	exit(0);
    	}
    }*/
	return;
}
/*
template <typename BCELL, typename KEY_TYPE>
void nativebtree<BCELL, KEY_TYPE>::insertNonFullLeaf(KEY_TYPE key, VALUE_TYPE value, LID node_pid)
{
	int i; int j;
	int num = getKeyNum(node_pid);
	BCELL * curcell = (BCELL*)(bufctl->readbuf(node_pid, disk));
	for(i=0; i<num; i++)
	{
		if(key <= curcell->key[i])
		{
			if(key == curcell->key[i])
			{
				//curcell->next[i] = value;
				//bufctl->writebuf(node_pid, disk);
				return;
			}
			for(j=num; j>i; j--)
			{
				curcell->key[j] = curcell->key[j-1];
				curcell->next[j] = curcell->next[j-1];
			}
			break;
		}
	}
	curcell->key[i] = key;
	curcell->next[i] = value;
	curcell->cur_knum++;
	bufctl->writebuf(node_pid, disk);
}
*/

template <typename BCELL, typename KEY_TYPE>
void nativebtree<BCELL, KEY_TYPE>::insertNonFullInner(KEY_TYPE key, int locate, LID lson_pid, LID rson_pid, LID node_pid)
{
	int i; int j;
	int num = getKeyNum(node_pid);
	BCELL * nodecell = (BCELL *)(bufctl->readbuf(node_pid, disk));
	if(locate == -1)
	{
		for(i=0; i<num; i++)
		{
			if(key < nodecell->key[i])
			{
				for(j=num; j>i; j--)
				{
					nodecell->key[j] = nodecell->key[j-1];
					nodecell->next[j+1] = nodecell->next[j];
				}
				break;
			}
		}
	}
	else
	{
		for(i=num; i>locate; i--)
		{
			nodecell->key[i] = nodecell->key[i-1];
			nodecell->next[i+1] = nodecell->next[i];
		}
	}
	nodecell->key[i] = key;
	nodecell->next[i+1] = rson_pid;
	nodecell->next[i] = lson_pid;
	nodecell->cur_knum++;
	bufctl->writebuf(node_pid, disk);
}

template <typename BCELL, typename KEY_TYPE>
bool nativebtree<BCELL, KEY_TYPE>::splitLeaf(LID curnode_pid, LID & lbro_pid, LID & rbro_pid)     //split leaf node
{
	int leftMax;
	int rightMin;
	BCELL * nodecell = (BCELL*)(bufctl->readbuf(curnode_pid, disk));
	_split_num++;

    leftMax=KEY_NUM/2-1;
    rightMin=leftMax+1;
	LID broR_pid = applynode();
	BCELL * rbrocell = (BCELL*)(bufctl->readbuf(broR_pid, disk));

	int numRight = KEY_NUM - KEY_NUM/2;
	int numLeft = KEY_NUM/2;
	int i;
	for(i=0; i<numRight; i++)
	{
		rbrocell->key[i] = nodecell->key[i+rightMin];
		rbrocell->next[i] = nodecell->next[i+rightMin];
		nodecell->key[i+rightMin] = NONTHING;
		nodecell->next[i+rightMin] = NONTHING;
	}
	nodecell->cur_knum=numLeft;
	rbrocell->cur_knum=numRight;
	rbro_pid = broR_pid;
	lbro_pid = curnode_pid;

	rbrocell->next[KEY_NUM] = nodecell->next[KEY_NUM];
	nodecell->next[KEY_NUM] = broR_pid;
	bufctl->writebuf(broR_pid, disk);
	bufctl->writebuf(curnode_pid, disk);
	return SPLIT;
}

template <typename BCELL, typename KEY_TYPE>
bool nativebtree<BCELL, KEY_TYPE>::splitInner(LID curnode_pid, KEY_TYPE & splitKey, LID & lbro_pid, LID & rbro_pid)    
{
	int numRightKey = KEY_NUM/2;
	int numLeftKey = KEY_NUM - numRightKey - 1;
	int leftMax = numLeftKey - 1;
	int rightMin = leftMax + 2;
	BCELL * curcell = (BCELL*)(bufctl->readbuf(curnode_pid, disk));
	_split_num++;

	splitKey = curcell->key[leftMax+1];
	LID new_pid = applynode();
	BCELL * new_cell = (BCELL*)(bufctl->readbuf(new_pid, disk));
	int i;
	for(i=0; i<numRightKey; i++)
	{
		new_cell->key[i] = curcell->key[i+rightMin];
		new_cell->next[i] = curcell->next[i+rightMin];
	}
	new_cell->next[i] = curcell->next[i+rightMin];
	new_cell->cur_knum=numRightKey;
	rbro_pid = new_pid;
	for(i=KEY_NUM-1; i>leftMax; i--)
	{
		curcell->key[i] = NONTHING;
		curcell->next[i+1] = NONTHING;
	}
	curcell->cur_knum=numLeftKey;
	lbro_pid = curnode_pid;
	bufctl->writebuf(lbro_pid, disk);
	bufctl->writebuf(rbro_pid, disk);
	return SPLIT;
}

template <typename BCELL, typename KEY_TYPE>
KEY_TYPE nativebtree<BCELL, KEY_TYPE>::easeRemoveKey(KEY_TYPE key, LID node_pid, LID & target_pid, int level)
{
	level++;
	int target = -1;
	bool flagKeyInner = false;
	KEY_TYPE newMin;
	int j;
	int num;
	BCELL * cur_cell;
	if(level == high-1)    //leaf
	{
		removeKeyNonMergeLeaf(key, node_pid);
		target_pid = node_pid;
		return getKeyLocate(node_pid, 0);
	}
	else   //inner node
	{
		num = getKeyNum(node_pid);
		cur_cell = (BCELL*)(bufctl->readbuf(node_pid, disk));
		for(j=0; j<num; j++)
		{
			if(key <= cur_cell->key[j])
			{
				if(key == cur_cell->key[j])
				{
					j++;
					flagKeyInner = true;
				}
				break;
			}
		}
//		if(flagKeyInner==false&&j == num)
//		{
//			cout<<"Exception--->the key being found is not exist-"<<className()<<" easeRemoveKey()"<<endl;
//			exit(0);
//		}
		target = j;
		newMin = easeRemoveKey(key, cur_cell->next[target], target_pid, level);
		if(flagKeyInner){
			cur_cell->key[target-1] = newMin;
			bufctl->writebuf(node_pid,disk);
		}

		return newMin;
	}
}

template <typename BCELL, typename KEY_TYPE>
TREE_STATE_TYPE nativebtree<BCELL, KEY_TYPE>::AfterRemoveKey(LID & target_pid, LID curnode_pid, int level)
{
	level++;
	int locateKey = -1;
	int locateNode = -1;
	LID next_pid;
	int i;
	BCELL * curnode_cell;
	BCELL * target_cell;
	int num;
	TREE_STATE_TYPE flagHelp;
	TREE_STATE_TYPE flagMerge;

	if(target_pid == curnode_pid)
	{
		if(getKeyNum(curnode_pid) < CEILING(KEY_NUM, 2)-1)
			return CALL_FOR_HELP;
		else
			return EASY_OK;
	}
	else
	{
		num = getKeyNum(curnode_pid);
		target_cell = (BCELL*)(bufctl->readbuf(target_pid, disk));
		curnode_cell = (BCELL*)(bufctl->readbuf(curnode_pid, disk));
		for(i=0; i<num; i++)
		{
			if(target_cell->key[0] < curnode_cell->key[i])
			{
				locateKey = i;
				locateNode = i;
				next_pid = curnode_cell->next[i];
				break;
			}
		}
		if(locateKey == -1)
		{
			locateKey = num-1;
			locateNode = num;
			next_pid = curnode_cell->next[num];
		}
		flagHelp = AfterRemoveKey(target_pid, next_pid, level);
		if(flagHelp == EASY_OK)
			return EASY_OK;
		else if(flagHelp == CALL_FOR_HELP)
		{
			flagMerge = handlHelpNode(next_pid, locateNode, curnode_pid, level);
			if(curnode_pid != root_pid)
			{
				if(flagMerge == MERGE_DONE)
				{
					if(getKeyNum(curnode_pid) < CEILING(KEY_NUM, 2)-1)
						return CALL_FOR_HELP;
					else
						return EASY_OK;
				}
				else
					return EASY_OK;
			}
			else 
			{
				if(flagMerge == MERGE_DONE)
				{
					if(getKeyNum(curnode_pid) < 1)
					{
						root_pid = curnode_cell->next[0];
						bufctl->deletepage(curnode_pid, disk);
						high--;
					}
				}
				return EASY_OK;
			}
		}
	}
}

template <typename BCELL, typename KEY_TYPE>
TREE_STATE_TYPE nativebtree<BCELL, KEY_TYPE>::handlHelpNode(LID cur_pid, int locateson, LID parent_pid, int level)
{
	level++;
	LID help_pid;
	int delLocate = -1;
	int num_big;
	int num_tiny;
	BCELL * parent_cell;
	parent_cell = (BCELL*)(bufctl->readbuf(parent_pid, disk));
	if(locateson > 0 && locateson < getKeyNum(parent_pid))
	{
		num_big = getKeyNum(parent_cell->next[locateson+1]);
		num_tiny = getKeyNum(parent_cell->next[locateson-1]);
		parent_cell = (BCELL*)(bufctl->readbuf(parent_pid, disk));
		if(num_big >= CEILING(KEY_NUM, 2))
			help_pid = parent_cell->next[locateson+1];
		else if(num_tiny >= CEILING(KEY_NUM, 2))
			help_pid = parent_cell->next[locateson-1];
		else if(num_big == CEILING(KEY_NUM, 2)-1)
		{
			help_pid = parent_cell->next[locateson+1];
			delLocate = locateson;
		}
		else if(num_tiny == CEILING(KEY_NUM, 2)-1)
		{
			help_pid = parent_cell->next[locateson-1];
			delLocate = locateson-1;
		}
	}
	else if(locateson == 0)
	{
		num_big = getKeyNum(parent_cell->next[locateson+1]);
		parent_cell = (BCELL*)(bufctl->readbuf(parent_pid, disk));
		if(num_big >= CEILING(KEY_NUM, 2))
			help_pid = parent_cell->next[locateson+1];
		else if(num_big == CEILING(KEY_NUM, 2)-1)
		{
			help_pid = parent_cell->next[locateson+1];
			delLocate = locateson;
		}
	}
	else if(locateson == getKeyNum(parent_pid))
	{
		num_tiny = getKeyNum(parent_cell->next[locateson-1]);
		parent_cell = (BCELL*)(bufctl->readbuf(parent_pid, disk));
		if(num_tiny >= CEILING(KEY_NUM, 2))
			help_pid = parent_cell->next[locateson-1];
		else if(num_tiny == CEILING(KEY_NUM ,2)-1)
		{
			help_pid = parent_cell->next[locateson-1];
			delLocate = locateson-1;
		}
	}

	BCELL * help_cell;
	BCELL * cur_cell;
	if(level == high-1)
	{
		if(delLocate == -1)
		{
			help_cell = (BCELL*)(bufctl->readbuf(help_pid, disk));
			cur_cell = (BCELL*)(bufctl->readbuf(cur_pid, disk));
			unitNonDelLeaf(cur_pid, help_pid);
			if(help_cell->key[0] > cur_cell->key[0])
			{
				if(locateson > 0)
					parent_cell->key[locateson-1] = cur_cell->key[0];
				parent_cell->key[locateson] = help_cell->key[0];
			}
			else 
			{
				if(locateson > 1)
					parent_cell->key[locateson-2] = help_cell->key[0];
				parent_cell->key[locateson-1] = cur_cell->key[0];
			}
			bufctl->writebuf(parent_pid, disk);
			return MERGE_NO;
		}
		else
		{
			if(delLocate == locateson)
				mergeWithDelLeaf(cur_pid, help_pid);
			else if(delLocate == locateson-1)
				mergeWithDelLeaf(help_pid, cur_pid);
			removeKeyWithRightSonNonMergeInner(delLocate, parent_pid);
			return MERGE_DONE;
		}
	}
	else
	{
		if(delLocate == -1)
		{
			unitNonDelInner(cur_pid, locateson, help_pid, parent_pid);
			return MERGE_NO;
		}
		else
		{
			if(delLocate == locateson)
				mergeWithDelInner(cur_pid, help_pid, locateson, parent_pid);
			else if(delLocate == locateson-1)
				mergeWithDelInner(help_pid, cur_pid, locateson-1, parent_pid);
			removeKeyWithRightSonNonMergeInner(delLocate, parent_pid);
			return MERGE_DONE;
		}
	}
}

template <typename BCELL, typename KEY_TYPE>
void nativebtree<BCELL, KEY_TYPE>::removeKeyWithLeftSonNonMergeInner(int locate, LID cur_pid)
{
	int i;
	int num;
	BCELL * cur_cell = (BCELL*)(bufctl->readbuf(cur_pid, disk));
	num = getKeyNum(cur_pid);
	for(i=locate; i<num-1; i++)
	{
		cur_cell->key[i] = cur_cell->key[i+1];
		cur_cell->next[i] = cur_cell->next[i+1];
	}
	cur_cell->next[i] = cur_cell->next[i+1];
	cur_cell->key[num-1] = NONTHING;
	cur_cell->next[num] = NONTHING; 	
	cur_cell->cur_knum--;
	bufctl->writebuf(cur_pid, disk);
}

template <typename BCELL, typename KEY_TYPE>
void nativebtree<BCELL, KEY_TYPE>::removeKeyWithRightSonNonMergeInner(int locate, LID cur_pid)
{
	int i;
	int num = getKeyNum(cur_pid);
	BCELL * cur_cell = (BCELL*)(bufctl->readbuf(cur_pid, disk));
	for(i=locate; i<num-1; i++)
	{
		cur_cell->key[i] = cur_cell->key[i+1];
		cur_cell->next[i+1] = cur_cell->next[i+2];
	}
	cur_cell->key[num-1] = NONTHING;
	cur_cell->next[num] = NONTHING;
	cur_cell->cur_knum--;
	bufctl->writebuf(cur_pid, disk);
}

template <typename BCELL, typename KEY_TYPE>
int nativebtree<BCELL, KEY_TYPE>::removeKeyNonMergeLeaf(KEY_TYPE key, LID cur_pid)
{
	int target = -1;
	BCELL * cur_cell = (BCELL*)(bufctl->readbuf(cur_pid, disk));
	int cur_num = getKeyNum(cur_pid);
	int i;
	for(i=0; i<cur_num; i++)
	{
		if(cur_cell->key[i] == key)
		{
			target = i;
			break;
		}
	}
	if(target == -1)
	{
		return 0;
		//cout<<"ERROR--->remove "<<key<<" from B+ tree !-"<<className()<<" removeKeyNonMergeLeaf()"<<endl;
		//exit(0);
	}

	for(i=target; i<cur_num-1; i++)
	{
		cur_cell->key[i] = cur_cell->key[i+1];
		cur_cell->next[i] = cur_cell->next[i+1];
	}
	cur_cell->key[cur_num-1] = NONTHING;
	cur_cell->next[cur_num-1] = NONTHING;
	cur_cell->cur_knum--;
	bufctl->writebuf(cur_pid, disk);
	return 1;
}

template <typename BCELL, typename KEY_TYPE>
KEY_TYPE nativebtree<BCELL, KEY_TYPE>::unitNonDelLeaf(LID leaf_pid, LID help_pid)
{
	BCELL * help_cell = (BCELL*)(bufctl->readbuf(help_pid, disk));
	BCELL * leaf_cell = (BCELL*)(bufctl->readbuf(leaf_pid, disk));
	int num_help = getKeyNum(help_pid);
	//int num_leaf = getKeyNum(leaf_pid);
	if(help_cell->key[0] < leaf_cell->key[0]){
		//helpBro is left brother of leaf
		insertNonFullLeaf(help_cell->key[num_help-1], help_cell->next[num_help-1], leaf_pid);
        removeKeyNonMergeLeaf(help_cell->key[num_help-1], help_pid);
    }
    else if(help_cell->key[0] > leaf_cell->key[0]){
                //helpBro is right brother of leaf
        insertNonFullLeaf(help_cell->key[0], help_cell->next[0], leaf_pid);
        removeKeyNonMergeLeaf(help_cell->key[0], help_pid);
    }
    else{
        cout<<"ERROR--->B+ tree has error struct found by unitNonDelLeaf-"<<className()<<" unitNonDelLeaf()"<<endl;
        exit(0);
    }
    return leaf_cell->key[0];
}

template <typename BCELL, typename KEY_TYPE>
void nativebtree<BCELL, KEY_TYPE>::unitNonDelInner(LID target_pid, int locateTarget, LID help_pid, LID parent_pid)
{
	LID lson_pid;
	LID rson_pid;
	BCELL * parent_cell = (BCELL*)(bufctl->readbuf(parent_pid, disk));
	BCELL * target_cell = (BCELL*)(bufctl->readbuf(target_pid, disk));
	BCELL * help_cell = (BCELL*)(bufctl->readbuf(help_pid, disk));
	int num_target = getKeyNum(target_pid);
	int num_help = getKeyNum(help_pid);
	if(target_cell->key[0] < help_cell->key[0])
	{   //helpnode is rightbrother
		lson_pid = target_cell->next[num_target];
		rson_pid = help_cell->next[0];
		insertNonFullInner(parent_cell->key[locateTarget], num_target, lson_pid, rson_pid, target_pid);
		parent_cell->key[locateTarget] = help_cell->key[0];
        removeKeyWithLeftSonNonMergeInner(0, help_pid);
		bufctl->writebuf(parent_pid, disk);
	}
	else if(target_cell->key[0] > help_cell->key[0])
	{
		lson_pid = help_cell->next[num_help];
		rson_pid = target_cell->next[0];
		insertNonFullInner(parent_cell->key[locateTarget-1], 0, lson_pid, rson_pid, target_pid);
		parent_cell->key[locateTarget-1] = help_cell->key[num_help-1];
        removeKeyWithRightSonNonMergeInner(num_help-1, help_pid);
		bufctl->writebuf(parent_pid, disk);
	}
    else{
        cout<<"ERROR--->unitNonDelInner take place -!"<<className()<<" unitNonDelInner()"<<endl;
        exit(0);
    }
}

template <typename BCELL, typename KEY_TYPE>
KEY_TYPE nativebtree<BCELL, KEY_TYPE>::mergeWithDelLeaf(LID small_pid, LID big_pid)
{
	int num_small = getKeyNum(small_pid);
	int num_big = getKeyNum(big_pid);
	BCELL * small_cell = (BCELL*)(bufctl->readbuf(small_pid, disk));
	BCELL * big_cell = (BCELL*)(bufctl->readbuf(big_pid, disk));
	int i;
	for(i=0; i<num_big; i++)
	{
		small_cell->key[i+num_small] = big_cell->key[i];
		small_cell->next[i+num_small] = big_cell->next[i];
	}
	small_cell->next[KEY_NUM] = big_cell->next[KEY_NUM];
	small_cell->cur_knum=num_small+num_big;
	bufctl->deletepage(big_pid, disk);
	bufctl->writebuf(small_pid, disk);
	return small_cell->key[0];
}

template <typename BCELL, typename KEY_TYPE>
void nativebtree<BCELL, KEY_TYPE>::mergeWithDelInner(LID small_pid, LID big_pid, int keyLocate, LID parent_pid)
{
	int j;
	int num_small = getKeyNum(small_pid);
	int num_big = getKeyNum(big_pid);
	int i = num_small + 1;
	BCELL * small_cell = (BCELL*)(bufctl->readbuf(small_pid, disk));
	BCELL * big_cell = (BCELL*)(bufctl->readbuf(big_pid, disk));
	BCELL * parent_cell=(BCELL*)(bufctl->readbuf(parent_pid, disk));
	small_cell->key[num_small]=parent_cell->key[keyLocate];
	for(j=0; j<num_big; j++)
	{
		small_cell->key[i] = big_cell->key[j];
		small_cell->next[i] = big_cell->next[j];
		i++;
	}
	small_cell->next[i] = big_cell->next[j];
	small_cell->cur_knum=num_small+num_big+1;
	bufctl->writebuf(small_pid, disk);
	bufctl->deletepage(big_pid, disk);

}

template <typename BCELL, typename KEY_TYPE>
int nativebtree<BCELL, KEY_TYPE>::getHeight()
{
	return high;
}

template <typename BCELL, typename KEY_TYPE>
void nativebtree<BCELL, KEY_TYPE>::printtree(LID node,int level){
	int i,num;
	ofstream fout("/home/ycc/workspace/btreedata/out",ios::app);
	BCELL * node_cell = (BCELL*)(bufctl->readbuf(node, disk));
	fout<<"level:======"<<level<<"=========="<<endl;
	for(i=0;i<KEY_NUM;i++){
		fout<<"key["<<i<<"]:"<<node_cell->key[i].keys<<"next["<<i<<"]:"<<node_cell->next[i]<<endl;

	}
	fout<<"next["<<i<<"]:"<<node_cell->next[i]<<endl;
	fout<<"cur_knum="<<node_cell->cur_knum<<endl;
	num=getKeyNum(node);
	if(level>1)
	{
		for(i=0;i<=num;i++){
			node_cell = (BCELL*)(bufctl->readbuf(node, disk));
			if(node_cell->next[i]!=NONTHING){
				printtree(node_cell->next[i],level-1);
			}
		}
	}
}


#endif
