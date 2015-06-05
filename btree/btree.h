/*
 * =====================================================================================
 *
 *       Filename:  virbtree.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  20/08/12 23:03:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  PUYUAN (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef _VIRBTREE_H
#define _VIRBTREE_H

#include "global.h"
#include "../bufmgr/lrubuf.h"

typedef unsigned short TREE_STATE_TYPE;
typedef int LEAF_STATE_TYPE;
typedef int VALUE_TYPE;

template <typename _KEY_TYPE>
class btree{
	public:
		btree(){};
		virtual ~btree(){};

		virtual int searchKey(_KEY_TYPE key)=0;
		virtual void insertKey(_KEY_TYPE key, VALUE_TYPE value)=0;
		virtual int updateKey(_KEY_TYPE key, VALUE_TYPE value)=0;
		virtual void deleteKey(_KEY_TYPE key)=0;
		virtual void rangesearchKey(_KEY_TYPE keymin,_KEY_TYPE keymax)=0;
		virtual LID getRoot()=0;
		virtual char * className()=0;
};

#endif
