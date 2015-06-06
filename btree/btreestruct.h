#ifndef _BTREE_STRUCT_H
#define _BTREE_STRUCT_H
#include <math.h>
#include "global.h"
#include "btree.h"
#define CEILING(X,Y) ceil(((double)X)/Y)
const int NONTHING = -1;
const int NODE_SPACE_FULL = 11;
const int NODE_NOFULL = -1;
const int NODE_SPLIT_FULL = 10;

const bool LEAF=true;
const bool NODE=false;
const bool SPLIT=true;
const int SPLIT_LEAF=2;
const bool NO_SPLIT=false;
const unsigned short EASY_OK=0x0000;
const unsigned short CALL_FOR_HELP=0x0001;
const unsigned short MERGE_DONE=0x1000;
const unsigned short MERGE_NO=0x1001;

#define KEYSET_SIZE(x) (PAGESIZE-sizeof(int)*2)/(sizeof(int)*(x+1))


#define PAGESIZE 4096
const int KEY1_NUM = 510;
//const int KEY1_NUM = 7;
const int KEY2_NUM = 340;
const int KEY3_NUM = 255;
const int KEY4_NUM = 204;

int compare_key(const int *self, const int *other, int keynum);

//int compare_element(const int *self, const int *other, int keynum);

typedef struct _key4
{
	int keys[4];
	_key4(int * key)
	{
		keys[0] = key[0];
		keys[1] = key[1];
		keys[2] = key[2];
		keys[3] = key[3];
	}

	_key4()
	{
		keys[0] = NONTHING;
		keys[1] = NONTHING;
		keys[2] = NONTHING;
		keys[3] = NONTHING;
	}

	struct _key4& operator = (const struct _key4& keyset)
	{
		keys[0] = keyset.keys[0];
		keys[1] = keyset.keys[1];
		keys[2] = keyset.keys[2];
		keys[3] = keyset.keys[3];
		return *this;
	}

	struct _key4& operator = (const int& key)
	{
		keys[0] = key;
		keys[1] = key;
		keys[2] = key;
		keys[3] = key;
		return *this;
	}

	bool operator == (const struct _key4& keysetB)
	{
		int k;
		for(k=0; k<4; k++)
		{
			if(keys[k] != keysetB.keys[k])
				return false;
		}
		return true;
	}

	bool operator == (const int& kv)
	{
		int k;
		for(k=0; k<4; k++)
		{
			if(keys[k] != kv)
				return false;
		}
		return true;
	}

	bool operator > (const struct _key4& keysetB)
	{

		int sum = compare_key(keys, keysetB.keys, 4);
        if (sum>0)
        	return true;
        else
        	return false;
	}

	bool operator >= (const struct _key4& keysetB)
	{
		int sum = compare_key(keys, keysetB.keys, 4);
        if (sum>=0)
        	return true;
        else
        	return false;
	}

	bool operator < (const struct _key4& keysetB)
	{
		int sum = compare_key(keys, keysetB.keys, 4);
        if (sum<0)
        	return true;
        else
        	return false;
	}

	bool operator <= (const struct _key4& keysetB)
	{
		int sum = compare_key(keys, keysetB.keys, 4);
        if (sum<=0)
        	return true;
        else
        	return false;
	}
	
	bool operator != (const struct _key4& keysetB)
	{
		int k;
		for(k=0; k<4; k++)
		{
			if(keys[k] != keysetB.keys[k])
				return true;
		}
		return false;
	}

	bool operator != (const int& vk)
	{
		int k;
		for(k=0; k<4; k++)
		{
			if(keys[k] != vk)
				return true;
		}
		return false;
	}
}key4;

typedef struct _key3
{
	int keys[3];
	_key3(int key[3])
	{
		keys[0] = key[0];
		keys[1] = key[1];
		keys[2] = key[2];
	}

	_key3()
	{
		keys[0] = NONTHING;
		keys[1] = NONTHING;
		keys[2] = NONTHING;
	}

	struct _key3& operator = (const _key3& keyset)
	{
		keys[0] = keyset.keys[0];
		keys[1] = keyset.keys[1];
		keys[2] = keyset.keys[2];
		return *this;
	}
	
	struct _key3& operator = (const int& key)
	{
		keys[0] = key;
		keys[1] = key;
		keys[2] = key;
		return *this;
	}

	bool operator == (const _key3& keysetB)
	{
		int k;
		for(k=0; k<3; k++)
		{
			if(keys[k] != keysetB.keys[k])
				return false;
		}
		return true;
	}

	bool operator == (const int& kv)
	{
		int k;
		for(k=0; k<3; k++)
		{
			if(keys[k] != kv)
				return false;
		}
		return true;
	}

	bool operator > (const _key3& keysetB)
	{
		int sum = compare_key(keys, keysetB.keys, 3);
        if (sum>0)
        	return true;
        else
        	return false;
	}

	bool operator >= (const _key3& keysetB)
	{
		int sum = compare_key(keys, keysetB.keys, 3);
        if (sum>=0)
        	return true;
        else
        	return false;

	}

	bool operator < (const _key3& keysetB)
	{
		int sum = compare_key(keys, keysetB.keys, 3);
        if (sum<0)
        	return true;
        else
        	return false;
	}

	bool operator <= (const _key3& keysetB)
	{
		int sum = compare_key(keys, keysetB.keys, 3);
        if (sum<=0)
        	return true;
        else
        	return false;
	}
	
	bool operator != (const _key3& keysetB)
	{
		int k;
		for(k=0; k<3; k++)
		{
			if(keys[k] != keysetB.keys[k])
				return true;
		}
		return false;
	}

	bool operator != (const int& vk)
	{
		int k;
		for(k=0; k<3; k++)
		{
			if(keys[k] != vk)
				return true;
		}
		return false;
	}
}key3;

typedef struct _key2
{
	int keys[2];
	_key2(int key[2])
	{
		keys[0] = key[0];
		keys[1] = key[1];
	}

	_key2()
	{
		keys[0] = NONTHING;
		keys[1] = NONTHING;
	}

	struct _key2& operator = (const _key2& keyset)
	{
		keys[0] = keyset.keys[0];
		keys[1] = keyset.keys[1];
		return *this;
	}
	
	struct _key2& operator = (const int& key)
	{
		keys[0] = key;
		keys[1] = key;
		return *this;
	}

	bool operator == (const _key2& keysetB)
	{
		int k;
		for(k=0; k<2; k++)
		{
			if(keys[k] != keysetB.keys[k])
				return false;
		}
		return true;
	}

	bool operator == (const int& kv)
	{
		int k;
		for(k=0; k<2; k++)
		{
			if(keys[k] != kv)
				return false;
		}
		return true;
	}

	bool operator > (const _key2& keysetB)
	{
		int sum = compare_key(keys, keysetB.keys, 2);
        if (sum>0)
        	return true;
        else
        	return false;
	}

	bool operator >= (const _key2& keysetB)
	{
		int sum = compare_key(keys, keysetB.keys, 2);
        if (sum>=0)
        	return true;
        else
        	return false;
	}

	bool operator < (const _key2& keysetB)
	{
		int sum = compare_key(keys, keysetB.keys, 2);
        if (sum<0)
        	return true;
        else
        	return false;
	}

	bool operator <= (const _key2& keysetB)
	{
		int sum = compare_key(keys, keysetB.keys, 2);
        if (sum<=0)
        	return true;
        else
        	return false;
	}
	
	bool operator != (const _key2& keysetB)
	{
		int k;
		for(k=0; k<2; k++)
		{
			if(keys[k] != keysetB.keys[k])
				return true;
		}
		return false;
	}

	bool operator != (const int& vk)
	{
		int k;
		for(k=0; k<2; k++)
		{
			if(keys[k] != vk)
				return true;
		}
		return false;
	}
}key2;

typedef struct _key1
{
	int keys;
	_key1(int keyf[1])
	{
		keys = keyf[0];
	}

	_key1()
	{
		keys = NONTHING;
	}

	struct _key1& operator = (const _key1& keyset)
	{
		keys = keyset.keys;
		return *this;
	}

	struct _key1& operator = (const int& keyn)
	{
		keys = keyn;
		return *this;
	}
	
	bool operator == (const _key1& keysetB)
	{
		if(keys == keysetB.keys)
			return true;
		return false;
	}

	bool operator == (const int& vk)
	{
		if(keys == vk)
			return true;
		return false;
	}

	bool operator > (const _key1& keysetB)
	{
		if(keysetB.keys==Infinity)
			return false;
		if(keys > keysetB.keys)
			return true;
		else
			return false;
	}

	bool operator >= (const _key1& keysetB)
	{
		if(keysetB.keys==Infinity)
			return false;
		if(keys >= keysetB.keys)
			return true;
		else
			return false;
	}

	bool operator < (const _key1& keysetB)
	{
		if(keysetB.keys==Infinity)
			return true;
		if(keys < keysetB.keys)
			return true;
		else
			return false;
	}

	bool operator <= (const _key1& keysetB)
	{
		if(keysetB.keys==Infinity)
			return true;
		if(keys <= keysetB.keys)
			return true;
		else
			return false;
	}
	
	bool operator != (const _key1& keysetB)
	{
		if(keys != keysetB.keys)
			return true;
		return false;
	}
	
	bool operator != (const int& vk)
	{
		if(keys != vk)
			return true;
		return false;
	}
}key1;

typedef struct _bcell_key1
{
	key1 key[KEY1_NUM];
	int next[KEY1_NUM+1];
	int cur_knum;
}bcell_key1;

typedef struct _bcell_key2
{
	key2 key[KEY2_NUM];
	int next[KEY2_NUM+1]; 
	int cur_knum;
}bcell_key2;

typedef struct _bcell_key3
{
	key3 key[KEY3_NUM];
	int next[KEY3_NUM+1];
	int cur_knum;
}bcell_key3;

typedef struct _bcell_key4
{
	key4 key[KEY4_NUM];
	int next[KEY4_NUM+1]; 
	int cur_knum;
}bcell_key4;

typedef struct b_leafhead{
	BYTE node_flag[3];		// every bit is a flag for an overnode, at most 16
	                        // 0---free  which will be active deleted node
							// 1---active and and full node which must not be active
	BYTE nodeNum;
	BYTE active;
	//If the node flag  is 0, and the corresponding bit array of bloom filter is 0, we consider it as free. This node can be a candidate of active node
	//If the node flag  is 0, and the corresponding bit array of bloom filter is not 0, we consider it as deleted . This node can be a candidate of active node
	//If the node flag  is 1, then this node is active or solid without going through deletion. This node can't be a candidate of active node
	BYTE reserved[0];
}LeafHeadb;
#endif

