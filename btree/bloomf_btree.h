#ifndef BLOOMF_BTREE_H_
#define BLOOMF_BTREE_H_
#include "nativebtree.h"
#include "../Bloomfilter/bloom.h"
#include <iostream>
#include "../bufmgr/BatchProcessBuffer.h"
using namespace std;
#define ACTIVE_NO 0xFF
#define HEAD_eBYTE_NUM 6  //the last byte of the page means its node type
#define MAX_NODE_NUM(bit_bytes) ((PAGESIZE-HEAD_eBYTE_NUM-sizeof(int))/(bit_bytes+sizeof(int)+2*sizeof(KEY_TYPE)))  //calculate the max overnode num
#define CUR_NODE_NUM(phead) (phead->nodeNum)  //current number of overflow nodes
#define ACTIVE(phead) (phead->active)  //the label number of active node
#define GET_KV_NUM(bit_bytes) ((PAGESIZE-HEAD_eBYTE_NUM-sizeof(int)-max_overflow_node_num*(bit_bytes+sizeof(int)+2*sizeof(KEY_TYPE)))/(sizeof(KEY_TYPE)+sizeof(VALUE_TYPE)))

#define BIT_ARRAY(phead, n, bit_bytes) ((int *)(phead->reserved+n*bit_bytes)) // the address of the n'th bit array


#define NODE_POINT(phead, bit_bytes) ((int *)(phead->reserved+max_overflow_node_num*bit_bytes)) // get the address of the pointers to all overflow nodes

// If there is spare space in leaf head, some key-value pairs will be stored in leaf head. The following macro definition gets the address of keys stored in leaf head
#define KEY_IN_HEAD(phead, bit_bytes) ((KEY_TYPE *)(phead->reserved+max_overflow_node_num*bit_bytes+sizeof(int)*(max_overflow_node_num+1)+2*sizeof(KEY_TYPE)*max_overflow_node_num))

// get the address of values stored in leaf head
#define VALUE_IN_HEAD(phead, bit_bytes) ((VALUE_TYPE *)(phead->reserved+max_overflow_node_num*bit_bytes+sizeof(int)*(max_overflow_node_num+1)+2*sizeof(KEY_TYPE)*max_overflow_node_num+kv_num*sizeof(KEY_TYPE)))

//get the address of the key range of all leaf pages
#define KEY_RANGE_IN_HEAD(phead, bit_bytes) ((KEY_TYPE *)(phead->reserved+max_overflow_node_num*bit_bytes+sizeof(int)*(max_overflow_node_num+1)))

template <typename KEY_TYPE>
class sortleafv;

template <typename BCELL, typename KEY_TYPE>
class BloomFilter_Btree : public nativebtree<BCELL,KEY_TYPE>{
public:
	BloomFilter_Btree(bufmgr * ctl, iofdctl * dd, int key_num_up,bloom *blm,double n_err,int ofnum_t,bpb<KEY_TYPE> *bpbuf):nativebtree<BCELL,KEY_TYPE>::nativebtree(ctl,dd,key_num_up){
		bloomfilter=blm;
		bit_bytes=bloomfilter->get_bit_bytes();
		max_overflow_node_num=MAX_NODE_NUM(bit_bytes);
		cout<<"actual calculate max over node num is "<<max_overflow_node_num<<endl;
		if(max_overflow_node_num>24)
			max_overflow_node_num=24;
		kv_num=GET_KV_NUM (bit_bytes);
		of_num_tradeoff=ofnum_t;
		//max_overflow_node_num=3;
		//kv_num=2;
		node_err=n_err;
		bpbuff=bpbuf;
		if(kv_num==0)
			max_split_count=max_overflow_node_num;
		else
			max_split_count=max_overflow_node_num+1;
	};
	void insert_key(KEY_TYPE key, VALUE_TYPE value);
	void delete_key(KEY_TYPE key);
	VALUE_TYPE search_Key(KEY_TYPE key);
	void clear_bpb( );
	void rangesearchKey(KEY_TYPE keymin,KEY_TYPE keymax);
	void printtree(LID node,int level);
	void print_max_overnode_num();
	void test();
private:
	void insertbfKey(KEY_TYPE key, VALUE_TYPE value);
	void deletebfKey(KEY_TYPE key);
	VALUE_TYPE searchbfKey(KEY_TYPE key);
	int bit_array_zero(int *bit_array);
	void set_location(LeafHeadb *head, int nodeID, int pageid);
	int get_location(LeafHeadb *head, int nodeID);
	void set_nodeflag(LeafHeadb *head, int nodeID, int zo);
	int get_nodeflag(LeafHeadb *head, int nodeID);
	int lookup(int *bit_array, char *key, int len);
	void set_active(LeafHeadb *head,int nodeID);
	void set_active_no(LeafHeadb *head);
	void set_cur_nodenum(LeafHeadb *head,int num);
	int get_new_active(LeafHeadb *head);
	LeafHeadb *get_first_node(LID cur_pid);
	LID apply_first_node();
	char * className();
	BCELL* get_bcell_key(LID curpid);
	int get_firstnode_KeyNum(KEY_TYPE *poskey);
	LID search_kloc_leafnode_h(LID curpid,KEY_TYPE key);
	LID search_kloc_leafnode_t(LID curpid,KEY_TYPE key);
	int search_kval_leafnode_h(LID curpid,KEY_TYPE key);
	int search_kval_leafnode_t(LID curpid,KEY_TYPE key);
	int search_kval_firstnode(LID curpid,KEY_TYPE key);
	int search_kval_leafpage(LID curpid,KEY_TYPE key);
	int search_kloc_leafpage(LID curpid,KEY_TYPE key);
	int insert_kval_firstnode(LID curpid,KEY_TYPE key,VALUE_TYPE value);
	void set_krange_firstnode(LeafHeadb *first_node,KEY_TYPE min_key,KEY_TYPE max_key, int nodeID);
	void get_krange_firstnode(LeafHeadb *first_node, KEY_TYPE *key_min,KEY_TYPE *key_max,int nodeID);
	void init_krange_firstnode(LeafHeadb *first_node,int nodeID);
	void update_kval_firstnode(LID curpid,KEY_TYPE key,VALUE_TYPE value);
	int splitofLeafssd(LID curnode_pid, LID lbro_pid[ ], LID rbro_pid[ ],KEY_TYPE ikey,VALUE_TYPE ivalue);
	void initnode_to_normal_leaf(LID curnode_pid);
	bool insertsplitleaf_spinner(LID curnode_pid,int freespace, KEY_TYPE & splitKey, LID & lbro_pid, LID & rbro_pid,int split_num,LID lch[],LID rch[]);
	void turn_solid(LID curnode_pid,int nodeID);
	void turn_active(LID curnode_pid,int nodeID);
	int insert_leafnode_h(LID curnode_pid,KEY_TYPE key,VALUE_TYPE value);
	int insert_leafnode_t(LID curnode_pid,KEY_TYPE key,VALUE_TYPE value);
	void get_krange_leafpage(BCELL *curcell,KEY_TYPE *key_min,KEY_TYPE *key_max);
	void turn_leafnode(int *loc);
	void init_firstnode(LID curnode_pid);
	int insertNode_bf(KEY_TYPE key, VALUE_TYPE value, LID curnode_pid, LID  lbro_pid[ ], KEY_TYPE & splitKey, LID  rbro_pid[ ], int level);
	bool is_node_active(LID firstnode_pid,LID nodepid);
	int removekeyfirstpage(KEY_TYPE key, LID cur_pid);
	void deletepage(LID first_pid,LID node_pid,int nodeid);
	bool is_leaf_empty_h(LID first_pid);
	bool is_leaf_empty_t(LID first_pid);
	void maintain_link_copynext(LID parentpid,int locate);
	void maintain_link_modify_pre_next(LID parentpid,int locate);
	void removekey_nonactivepage(KEY_TYPE key, LID cur_pid,LID keylocate);
	int removeKeyAllLeaf_h(KEY_TYPE key, LID cur_pid);
	int removeKeyAllLeaf_t(KEY_TYPE key, LID cur_pid);
	int removeKeyOFNode(KEY_TYPE key, LID cur_pid,LID pre_id);
	int bf_easeRemoveKey(KEY_TYPE key, LID node_pid,LID parent_pid, LID & target_pid,int locateson, int level,int flag);
	TREE_STATE_TYPE bf_AfterRemoveKey(LID & target_pid, LID curnode_pid, int level);
	TREE_STATE_TYPE bf_handlHelpNode(LID cur_pid, int locateson, LID parent_pid, int level);
	VALUE_TYPE searchbfKeyIn(KEY_TYPE key, LID curroot, int level);
	void rangesearchbfKeyIn(KEY_TYPE keymin,KEY_TYPE keymax,LID curroot, int level);
	int scanleafnode(LID current_pid,LID *nextpid,KEY_TYPE keymin,KEY_TYPE keymax);
	int scanleafnode_h(LID current_pid,LID *nextpid,KEY_TYPE keymin,KEY_TYPE keymax);
	int scanleafnode_t(LID current_pid,LID *nextpid,KEY_TYPE keymin,KEY_TYPE keymax);
	int judge_node_type(LID pid);
	int is_last_page_of_leaf(LID pid);
	void set_page_non_last(BCELL *curcell);
	void set_page_last(BCELL *curcell);
	int flush(taskentry<KEY_TYPE> **entrybuff,int & ecount);
	void extree_flush();
    int merge_sort(sortleafv<KEY_TYPE> *tembuf,LID curnode_pid,KEY_TYPE ikey,VALUE_TYPE ivalue);
	int max_overflow_node_num;
	int max_split_count;
	int bit_bytes;
	int kv_num;
	double node_err;
	bloom *bloomfilter;
	int of_num_tradeoff;
	bpb<KEY_TYPE> *bpbuff;
};

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::print_max_overnode_num()
{
	cout<<"max over node num"<<max_overflow_node_num<<endl;
}

//=======================================quick sort=======================================================================
template <typename KEY_TYPE>
int swap(sortleafv<KEY_TYPE> *i, sortleafv<KEY_TYPE> *j)
 {
	sortleafv<KEY_TYPE> temp;

     if(i->key == j->key)
         return 0;

     temp = *i;
     *i = *j;
     *j = temp;

     return 1;
 }

template <typename KEY_TYPE>
 void quicksort(sortleafv<KEY_TYPE> array[],int begin, int end)
 {
     int i, j;

     if(begin < end) {
         i = begin + 1;
         j = end;
         while(i < j) {
             if(array[i].key > array[begin].key) {
                 swap(&array[i], &array[j]);
                 j--;
             }
             else
                 i++;
         }
         if (array[i].key > array[begin].key)
             i--;
        swap(&array[begin], &array[i]);

         quicksort(array, begin, i);
         quicksort(array, j, end);
     }
 }
//========================================================================================================================
template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::merge_sort(sortleafv<KEY_TYPE> *tembuf,LID curnode_pid,KEY_TYPE ikey,VALUE_TYPE ivalue){
    LeafHeadb *first_node;
    BCELL *temp;
    KEY_TYPE *poskey;
    VALUE_TYPE *posvalue;
	sortleafv<KEY_TYPE> **array=new sortleafv<KEY_TYPE> *[max_overflow_node_num+2];
	int *lens=new int[max_overflow_node_num+2];
    LID location;
    int i;
    first_node=get_first_node(curnode_pid);
    poskey=KEY_IN_HEAD(first_node, bit_bytes);
    posvalue=VALUE_IN_HEAD(first_node, bit_bytes);
    for(int n=0;n<max_overflow_node_num+2;n++){
    	array[n]=new sortleafv<KEY_TYPE>[nativebtree<BCELL,KEY_TYPE>::KEY_NUM];
    	if(n==0){
    		for(i=0;i<kv_num;i++){
    			array[n][i].key=poskey[i];
    			array[n][i].next=posvalue[i];
    		}
    		lens[n]=kv_num;
    	}
    	else if(n<max_overflow_node_num+1){
    		   first_node=get_first_node(curnode_pid);
    		   location=get_location(first_node,n-1);
    		   temp=get_bcell_key(location);
    			for(i=0;i<nativebtree<BCELL,KEY_TYPE>::KEY_NUM;i++){
    				array[n][i].key=temp->key[i];
    				array[n][i].next=temp->next[i];
    			}
    			lens[n]=nativebtree<BCELL,KEY_TYPE>::KEY_NUM;
    			nativebtree<BCELL,KEY_TYPE>::bufctl->deletepage(location,nativebtree<BCELL,KEY_TYPE>::disk);
    	}
    	else{
    		array[n][0].key=ikey;
    		array[n][0].next=ivalue;
    		lens[n]=1;
    		nativebtree<BCELL,KEY_TYPE>::bufctl->deletepage(curnode_pid,nativebtree<BCELL,KEY_TYPE>::disk);
    	}
    }
    //===========================begin merge sort==============================
    int* cnts = new int[max_overflow_node_num+2];
    int* has_next =new int[max_overflow_node_num+2];
    sortleafv<KEY_TYPE> * datas = new sortleafv<KEY_TYPE> [max_overflow_node_num+2];
    int min_index = 0;
    sortleafv<KEY_TYPE>  min;
    int k=max_overflow_node_num+2;
    int j=0;
    //Read each k-way first into data
    for (i = 0; i < k; i++)
    {
        if (lens[i] >= 1)
        {
            datas[i] = array[i][0];
            cnts[i] = 1;
            has_next[i] = 1;
        }
        else
        {
            has_next[i] = 0;
        }
    }

    while (1)
    {

        //Select min in k
        min.key = INT_MAX;
        min_index = 0;
        for (i = 0; i < k; i++)
        {
            if (has_next[i])
            {
                if (datas[i].key < min.key)
                {
                    min = datas[i];
                    min_index = i;
                }
            }
        }

        if (min.key == INT_MAX)
        {
            //No way has data
            break;
        }
        else
        {
            tembuf[j]=datas[min_index];
            j++;
            //Succ get one min
            if (cnts[min_index] < lens[min_index])
            {
                datas[min_index] = array[min_index][cnts[min_index]];
                cnts[min_index]++;
            }
            else
            {
                has_next[min_index] = 0;
            }
        }
    }
    //========================free the allocate memory====================
    free(lens);
    for(int n=0;n<max_overflow_node_num+2;n++)
    	free(array[n]);
    free(array);
    free(cnts);
    free(has_next);
    free(datas);
    int total_num=kv_num+max_overflow_node_num*nativebtree<BCELL,KEY_TYPE>::KEY_NUM+1;
    if(j!=total_num){
    	cout<<"error---> in merge_sort() of total_num"<<endl;
    	exit(0);
    }
    return total_num;
}
//================end of merge sort ======================================
template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::insert_key(KEY_TYPE key, VALUE_TYPE value){
	taskentry<KEY_TYPE> t;
	t.flag='I';
	t.key=key;
	t.next=value;
	int bufstate=bpbuff->IsBPBFull();
	if(bufstate==0){
		bpbuff->insert_te(t);
	}
	else if(bufstate==1){
		extree_flush();
		bpbuff->insert_te(t);
	}
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::delete_key(KEY_TYPE key){
	taskentry<KEY_TYPE> t;
	t.flag='D';
	t.key=key;
	t.next=-1;
	int bufstate=bpbuff->IsBPBFull();
	if(bufstate==0){
		bpbuff->insert_te(t);
	}
	else if(bufstate==1){
		extree_flush();
		bpbuff->insert_te(t);
	}
}

template <typename BCELL, typename KEY_TYPE>
VALUE_TYPE BloomFilter_Btree<BCELL,KEY_TYPE>::search_Key(KEY_TYPE key){
	VALUE_TYPE  key_value=bpbuff->searchkey(key);
	if(key_value==Infinity){
		search_tree_count++;
		return searchbfKey( key);
	}
	else if(key_value!=Infinity)
		return key_value;
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::rangesearchKey(KEY_TYPE keymin,KEY_TYPE keymax){
	int count;
	int startpos=1;
	int endpos=bpbuff->curnum;
	count=bpbuff->findvalue(keymin,keymax,startpos,endpos);
	//search the tree on disk

	if(nativebtree<BCELL, KEY_TYPE>::root_pid < 0)
		return ;
	rangesearchbfKeyIn(keymin,keymax,nativebtree<BCELL, KEY_TYPE>::root_pid, -1);
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::clear_bpb( ){
	int i=0;
	taskentry<KEY_TYPE> *temp;
	for(i=0;i<bpbuff->curnum;i++){
		temp=(taskentry<KEY_TYPE> *)(bpbuff->buffer+i*sizeof(taskentry<KEY_TYPE>));
		if(temp->flag=='I')
			insertbfKey(temp->key, temp->next);
		else if(temp->flag=='D')
			deletebfKey(temp->key);
		else{
			cout<<"error-->in clear_bpb() ---flag error"<<endl;
			exit(0);
		}
	}
	flush_count=flush_count+bpbuff->curnum;
	bpbuff->clear();
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::bit_array_zero(int *bit_array){
	//Judging whether the bit array of bloom filter is 0
	int m = bit_bytes/sizeof(int);
	int k;
	for(k=0; k<m; k++)
	{
		if(bit_array[k] > 0)
			return 1;    //Not 0
	}
	return 0;       //Is 0
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::set_location(LeafHeadb *head, int nodeID, int pageid)
{
	int * node_array = NODE_POINT(head, bit_bytes);
	node_array[nodeID] = pageid;     //Allocating a new page
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::get_location(LeafHeadb *head, int nodeID)
{
	int *node_array = NODE_POINT(head, bit_bytes);
	return node_array[nodeID];
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::set_nodeflag(LeafHeadb *head, int nodeID, int zo)
{
	if(zo == 1)
		head->node_flag[nodeID>>3]|= (1<<(nodeID & 7));
	else
		head->node_flag[nodeID>>3] &= (~(1<<(nodeID & 7)));
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::get_nodeflag(LeafHeadb *head, int nodeID)
{
	return ((head->node_flag[nodeID>>3]>>(nodeID & 7)) & 1);
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::lookup(int *bit_array, char *key, int len)
{
	return bloomfilter->bloom_get(bit_array, key, len);
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::set_active(LeafHeadb *head,int nodeID){
	head->active = nodeID;
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::set_active_no(LeafHeadb *head){
	head->active = ACTIVE_NO;
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::set_cur_nodenum(LeafHeadb *head,int num){
	head->nodeNum = num;
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::get_new_active(LeafHeadb *head)
{
	//Allocating label number for the new active node
	//1. If there exists a deleted node, we set it as active node, then emptying corresponding bit array of bloom filter
	//2. If not, we need to allocate a new page as the active node
	int i = 0;
	int candidate = -1;
	//BYTE flag;
	//int * bitarray;
	while(i<max_overflow_node_num)
	{
		if(get_nodeflag(head, i) == 0)// the state flag of this node is 0, we consider it as deleted or free
		{
			if(get_location(head, i) == -1)  //free node
				candidate = candidate<0 ? i:candidate;
			else  // deleted node => actiuve
			{
				set_active(head,i);
				bloomfilter->bloom_init(BIT_ARRAY(head, i, bit_bytes)); //empty corresponding bit array of bloom filter
				set_nodeflag(head, i, 1);   //change the state
				return i;
			}
			break;
		}
		i++;
	}
	if(candidate < 0)
		return candidate;
	set_location(head, candidate,nativebtree<BCELL,KEY_TYPE>::applynode());//applyNewPage());
	set_active(head,candidate);
	head->nodeNum++;
	bloomfilter->bloom_init(BIT_ARRAY(head, candidate, bit_bytes)); //empty corresponding bit array of bloom filter
	set_nodeflag(head, candidate, 1);   //change the state

	return candidate;

}

template <typename BCELL, typename KEY_TYPE>
LeafHeadb *BloomFilter_Btree<BCELL,KEY_TYPE>::get_first_node(LID cur_pid){
	LeafHeadb *temp=(LeafHeadb *)(nativebtree<BCELL,KEY_TYPE>::bufctl->readbuf(cur_pid, nativebtree<BCELL,KEY_TYPE>::disk));
	return temp;
}


template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::judge_node_type(LID cur_pid){
	BYTE * page = nativebtree<BCELL,KEY_TYPE>::bufctl->readbuf(cur_pid, nativebtree<BCELL,KEY_TYPE>::disk);
	//return *(page+PAGESIZE-1);
	BYTE type=*(page+PAGESIZE-1);
	return type;
}

template <typename BCELL, typename KEY_TYPE>  //0 means it has next overflow page,1 means it has no next overflow page
int BloomFilter_Btree<BCELL,KEY_TYPE>::is_last_page_of_leaf(LID pid){
	BYTE * page = nativebtree<BCELL,KEY_TYPE>::bufctl->readbuf(pid, nativebtree<BCELL,KEY_TYPE>::disk);
	return *(page+PAGESIZE-2);
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::set_page_non_last(BCELL *curcell){
	*((BYTE*)curcell+PAGE_SIZE-2) = 0;
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::set_page_last(BCELL *curcell){
	*((BYTE*)curcell+PAGE_SIZE-2) = 1;
}

template <typename BCELL, typename KEY_TYPE>
LID BloomFilter_Btree<BCELL,KEY_TYPE>::apply_first_node(){
	LID first_pid=nativebtree<BCELL,KEY_TYPE>::applynode();
	BYTE *b=nativebtree<BCELL,KEY_TYPE>::bufctl->readbuf(first_pid, nativebtree<BCELL,KEY_TYPE>::disk);
	int len=HEAD_eBYTE_NUM-1+max_overflow_node_num*bit_bytes;
	memset(b, 0, len);
	memset(b+len, -1,PAGESIZE-1-len);
	*(b+PAGESIZE-1) = 1;
	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(first_pid,nativebtree<BCELL,KEY_TYPE>::disk);
	return first_pid;
}

template <typename BCELL, typename KEY_TYPE>
char * BloomFilter_Btree<BCELL,KEY_TYPE>::className(){
	return "BloomFilter btree";
}

template <typename BCELL, typename KEY_TYPE>
BCELL* BloomFilter_Btree<BCELL,KEY_TYPE>:: get_bcell_key(LID curpid){
	BCELL*temp=(BCELL*)(nativebtree<BCELL,KEY_TYPE>::bufctl->readbuf(curpid, nativebtree<BCELL,KEY_TYPE>::disk));
	return temp;
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::get_firstnode_KeyNum(KEY_TYPE *poskey){
	int k = 0;
	if(kv_num==0)
		return 0;
	while(poskey[k] != NONTHING && k<kv_num)
		k++;
	return k;

}
template <typename BCELL, typename KEY_TYPE>
LID BloomFilter_Btree<BCELL,KEY_TYPE>::search_kloc_leafnode_h(LID curpid,KEY_TYPE key){
    if(search_kval_firstnode(curpid,key)!=NONTHING)
    	return curpid;
    LeafHeadb *first_node=get_first_node(curpid);
    int i,active_node,location;
    int* bit_array;
    VALUE_TYPE value;
    char *tkey=(char *)(&key);
    int cur_nodenum=CUR_NODE_NUM(first_node);
    if(cur_nodenum==0)
    	return NONTHING;
    active_node=ACTIVE(first_node);
    for(i=0;i<max_overflow_node_num;i++){
    	first_node=get_first_node(curpid);
    	location=get_location(first_node,i);
    	if(location!=-1){

        	if(i!=active_node){
        		bit_array=BIT_ARRAY(first_node, i, bit_bytes);
        		bloom_search_count++;
        		if(lookup(bit_array,tkey,sizeof(KEY_TYPE))==1){

        			value=search_kval_leafpage(location,key);
            		if(value!=NONTHING)
            			return location;
            		else
            			bloom_error_count++;
        		}
        	}
        }
    }
    if(active_node==ACTIVE_NO)
    	return NONTHING;
    first_node=get_first_node(curpid);
    location=get_location(first_node,active_node);
	value=search_kval_leafpage(location,key);
	if(value!=NONTHING)
		return location;
	else
		return NONTHING;
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::search_kval_leafnode_h(LID curpid,KEY_TYPE key){
	VALUE_TYPE value;
	value=search_kval_firstnode(curpid,key);
	if(value!=NONTHING)
		return value;
    LeafHeadb *first_node=get_first_node(curpid);
    int i,cur_nodenum,active_node,location;
    int* bit_array;
    char *tkey=(char *)(&key);
    cur_nodenum=CUR_NODE_NUM(first_node);
    if(cur_nodenum==0)
    	return NONTHING;
    active_node=ACTIVE(first_node);
    for(i=0;i<max_overflow_node_num;i++){
    	first_node=get_first_node(curpid);
    	location=get_location(first_node,i);
    	if(location!=-1){

        	if(i!=active_node){
        		bit_array=BIT_ARRAY(first_node, i, bit_bytes);
        		bloom_search_count++;
        		if(lookup(bit_array,tkey,sizeof(KEY_TYPE))==1){

        			value=search_kval_leafpage(location,key);
            		if(value!=NONTHING)
            			return value;
            		else
            			bloom_error_count++;
        		}
        	}
        }
    }
    if(active_node==ACTIVE_NO)
    	return NONTHING;
    first_node=get_first_node(curpid);
    location=get_location(first_node,active_node);
	value=search_kval_leafpage(location,key);
	if(value!=NONTHING)
		return value;
	else
		return NONTHING;
}

template <typename BCELL, typename KEY_TYPE>
LID BloomFilter_Btree<BCELL,KEY_TYPE>::search_kloc_leafnode_t(LID curpid,KEY_TYPE key){
	int i=0;
	LID tpid=curpid;
	VALUE_TYPE value;
	BCELL *cur_cell;
	for(i=0;i<of_num_tradeoff;i++){
		value=search_kval_leafpage(tpid,key);
		if(value!=NONTHING)
			return tpid;
        if(is_last_page_of_leaf(tpid))
        	break;
		cur_cell=get_bcell_key(tpid);
		tpid=cur_cell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM];
	}
	return NONTHING;
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::search_kval_leafnode_t(LID curpid,KEY_TYPE key){
	int i=0;
	LID tpid=curpid;
	VALUE_TYPE value;
	BCELL *cur_cell;
	for(i=0;i<of_num_tradeoff;i++){
		value=search_kval_leafpage(tpid,key);
		if(value!=NONTHING)
			return value;
        if(is_last_page_of_leaf(tpid))
        	break;
		cur_cell=get_bcell_key(tpid);
		tpid=cur_cell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM];
	}
	return NONTHING;
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::search_kval_firstnode(LID curpid,KEY_TYPE key){
	LeafHeadb *first_node=get_first_node(curpid);
	KEY_TYPE *poskey=KEY_IN_HEAD(first_node, bit_bytes);
	KEY_TYPE tkey;
	int i=0;
	for(i=0;i<kv_num;i++){
        if(poskey[i]==NONTHING)
        	return NONTHING;
        else if(poskey[i]==key){
        	VALUE_TYPE *posvalue=VALUE_IN_HEAD(first_node, bit_bytes);
        	return posvalue[i];
        }
	}
	return NONTHING;
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::search_kloc_leafpage(LID curpid,KEY_TYPE key){
	BCELL * curcell;
	curcell =get_bcell_key(curpid);
    int low=0;
    int high=curcell->cur_knum-1;
	int middle=0;
	while(low<=high){
		middle=(low+high)/2;
		if(curcell->key[middle]==key)
			return middle;
		else if(curcell->key[middle]>key)
			high=middle-1;
		else
			low=middle+1;
	}
	return low;
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::search_kval_leafpage(LID curpid,KEY_TYPE key){
	BCELL * curcell;
	curcell =get_bcell_key(curpid);
    int low=0;
    int high=curcell->cur_knum-1;
	int middle=0;
	while(low<=high){
		middle=(low+high)/2;
		if(curcell->key[middle]==key)
			return curcell->next[middle];
		else if(curcell->key[middle]>key)
			high=middle-1;
		else
			low=middle+1;
	}
	return NONTHING;
}

/*
template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::search_kval_leafpage(LID curpid,KEY_TYPE key){
	BCELL * curcell;
	curcell =get_bcell_key(curpid);
	int k=0;
	while(curcell->key[k] != NONTHING && k<nativebtree<BCELL, KEY_TYPE>::KEY_NUM)
	{
		if(curcell->key[k] == key){
			return curcell->next[k];
		}
		k++;
	}
	return NONTHING;
}
*/

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::insert_kval_firstnode(LID curpid,KEY_TYPE key,VALUE_TYPE value){
	LeafHeadb *first_node=get_first_node(curpid);
	KEY_TYPE *poskey=KEY_IN_HEAD(first_node, bit_bytes);
	int i;
	int j;
	VALUE_TYPE *posvalue;
	if((kv_num==0)||(poskey[kv_num-1]!=NONTHING))
		return 0;
	else{
		posvalue=VALUE_IN_HEAD(first_node, bit_bytes);
		int cur_knum=get_firstnode_KeyNum(poskey);
		for(i=0;i<cur_knum;i++){
			if(key <= poskey[i]){
				VALUE_TYPE *posvalue=VALUE_IN_HEAD(first_node, bit_bytes);
				if(poskey[i]==key){
					posvalue[i]=value;
					nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(curpid,nativebtree<BCELL,KEY_TYPE>::disk);
					return 1;
				}
				for(j=cur_knum; j>i; j--)
				{
					poskey[j] = poskey[j-1];
					posvalue[j] = posvalue[j-1];
				}
				break;
			}
		}
		poskey[i] = key;
		posvalue[i] = value;
		nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(curpid,nativebtree<BCELL,KEY_TYPE>::disk);
		return 1;
	}

}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::set_krange_firstnode(LeafHeadb *first_node,KEY_TYPE min_key,KEY_TYPE max_key, int nodeID){
	//LeafHeadb *first_node=get_first_node(curpid);
	KEY_TYPE *poskey=KEY_RANGE_IN_HEAD(first_node, bit_bytes)+2*nodeID;
	poskey[0]=min_key;
	poskey[1]=max_key;
	//nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(curpid,nativebtree<BCELL,KEY_TYPE>::disk);
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::get_krange_firstnode(LeafHeadb *first_node, KEY_TYPE *key_min,KEY_TYPE *key_max,int nodeID){
	KEY_TYPE *poskey=KEY_RANGE_IN_HEAD(first_node, bit_bytes)+2*nodeID;
	*key_min=poskey[0];
	*key_max=poskey[1];
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::init_krange_firstnode(LeafHeadb *first_node,int nodeID){
	KEY_TYPE *poskey=KEY_RANGE_IN_HEAD(first_node, bit_bytes)+2*nodeID;
	poskey[0]=NONTHING;
	poskey[1]=NONTHING;
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::update_kval_firstnode(LID curpid,KEY_TYPE key,VALUE_TYPE value){
	LeafHeadb *first_node=get_first_node(curpid);
	KEY_TYPE *poskey=KEY_IN_HEAD(first_node, bit_bytes);
	int i;
	VALUE_TYPE *posvalue=VALUE_IN_HEAD(first_node, bit_bytes);
	for(i=0;i<kv_num;i++){
		if(poskey[i]==NONTHING)
			break;
		if(poskey[i]==key){
			posvalue[i]=value;
			nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(curpid,nativebtree<BCELL,KEY_TYPE>::disk);
			break;
		}
	}
}
template <typename KEY_TYPE>
class sortleafv{
	public:
	KEY_TYPE key;
	int next;
};
/*
template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::splitofLeafssd(LID curnode_pid, LID lbro_pid[ ], LID rbro_pid[ ],KEY_TYPE ikey,VALUE_TYPE ivalue){
	sortleafv<KEY_TYPE> *tembuf=new sortleafv<KEY_TYPE>[kv_num+max_overflow_node_num*nativebtree<BCELL,KEY_TYPE>::KEY_NUM+1];
    int i,j,k;
    _split_num++;
    BCELL *temp;
	int total_num=merge_sort(tembuf, curnode_pid, ikey,ivalue);
	j=0;
	int *loc=new int[max_overflow_node_num+1];
	for(i=0;i<max_overflow_node_num+1;i++)
		loc[i]=-1;
    for(i=0;i<max_overflow_node_num+1;i++){
		loc[i]=nativebtree<BCELL,KEY_TYPE>::applynode();
		temp=get_bcell_key(loc[i]);
    	if(i!=max_overflow_node_num){
    		for(k=0;k<nativebtree<BCELL,KEY_TYPE>::KEY_NUM;k++){
    			temp->key[k]=tembuf[j].key;
    			temp->next[k]=tembuf[j].next;
    			j++;
    		}
    		temp->cur_knum=nativebtree<BCELL,KEY_TYPE>::KEY_NUM;
    	}
    	else if(i==max_overflow_node_num){
    		int remain_knum=total_num-max_overflow_node_num*nativebtree<BCELL,KEY_TYPE>::KEY_NUM;
    		for(k=0;k<remain_knum;k++){
    			temp->key[k]=tembuf[j].key;
    			temp->next[k]=tembuf[j].next;
    			j++;
    		}
    		temp->cur_knum=remain_knum;
    	}
		nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(loc[i],nativebtree<BCELL,KEY_TYPE>::disk);
    }
    for(i=0;i<max_overflow_node_num;i++){
   //	 LID lbro_pid[], LID rbro_pid[]
   	 lbro_pid[i]=loc[i];
   	 rbro_pid[i]=loc[i+1];
    }
    delete[] loc;
    delete[] tembuf;
    return SPLIT_LEAF;
}
*/

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::initnode_to_normal_leaf(LID curnode_pid){
	BCELL * new_cell = get_bcell_key(curnode_pid);
	memset(new_cell, NONTHING, PAGE_SIZE);
	new_cell->cur_knum=0;
	*((BYTE*)new_cell+PAGE_SIZE-1) = 0;  //it means it's a traditional leaf
	*((BYTE*)new_cell+PAGE_SIZE-2) = 1;   //it means we apply a new page has no overflow
	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(curnode_pid,nativebtree<BCELL,KEY_TYPE>::disk);
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::splitofLeafssd(LID curnode_pid, LID lbro_pid[ ], LID rbro_pid[ ],KEY_TYPE ikey,VALUE_TYPE ivalue){
	sortleafv<KEY_TYPE> *tembuf=new sortleafv<KEY_TYPE>[kv_num+max_overflow_node_num*nativebtree<BCELL,KEY_TYPE>::KEY_NUM+1];
    int i,j,k;
    LeafHeadb *first_node;
    _split_num++;
    BCELL *temp;
    KEY_TYPE *poskey;
    VALUE_TYPE *posvalue;
    LID location;
    j=0;
    first_node=get_first_node(curnode_pid);
    poskey=KEY_IN_HEAD(first_node, bit_bytes);
    posvalue=VALUE_IN_HEAD(first_node, bit_bytes);
    for(i=0;i<kv_num;i++){
		tembuf[j].key=poskey[i];
		tembuf[j].next=posvalue[i];
		j++;
    }
   for(i=0;i<max_overflow_node_num;i++){
	   first_node=get_first_node(curnode_pid);
	   location=get_location(first_node,i);
	   temp=get_bcell_key(location);
		for(k=0;k<nativebtree<BCELL,KEY_TYPE>::KEY_NUM;k++){
			tembuf[j].key=temp->key[k];
			tembuf[j].next=temp->next[k];
			j++;
		}
		nativebtree<BCELL,KEY_TYPE>::bufctl->deletepage(location,nativebtree<BCELL,KEY_TYPE>::disk);
   }
   first_node=get_first_node(curnode_pid);
    LID next_neighbor=get_location(first_node,max_overflow_node_num);
	//nativebtree<BCELL,KEY_TYPE>::bufctl->deletepage(curnode_pid,nativebtree<BCELL,KEY_TYPE>::disk);
    //we can init the first node to a normal leaf
    initnode_to_normal_leaf(curnode_pid);
	tembuf[j].key=ikey;
	tembuf[j].next=ivalue;
	j++;
	quicksort(tembuf,0,j-1);
	int total_num=j;
	j=0;
	int max_node_num=0;
	if(total_num%nativebtree<BCELL,KEY_TYPE>::KEY_NUM==0)
		max_node_num=total_num/nativebtree<BCELL,KEY_TYPE>::KEY_NUM;
	else
		max_node_num=total_num/nativebtree<BCELL,KEY_TYPE>::KEY_NUM+1;
	if(max_node_num!=max_split_count){
		cout<<"error-->split leaf ssd"<<endl;
		exit(0);
	}
	int *loc=new int[max_node_num];
	loc[0]=curnode_pid;
	for(i=1;i<max_node_num;i++)
		loc[i]=nativebtree<BCELL,KEY_TYPE>::applynode();

    for(i=0;i<max_node_num;i++){
//		loc[i]=nativebtree<BCELL,KEY_TYPE>::applynode();
		temp=get_bcell_key(loc[i]);
    	if(i!=max_node_num-1){  //i<=max_node_num-2
    		for(k=0;k<nativebtree<BCELL,KEY_TYPE>::KEY_NUM;k++){
    			temp->key[k]=tembuf[j].key;
    			temp->next[k]=tembuf[j].next;
    			j++;
    		}
    		temp->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM]=loc[i+1];   //point to the right neighbor
    		temp->cur_knum=nativebtree<BCELL,KEY_TYPE>::KEY_NUM;
    	}
    	else if(i==max_node_num-1){
    		int remain_knum=total_num-(max_node_num-1)*nativebtree<BCELL,KEY_TYPE>::KEY_NUM;
    		for(k=0;k<remain_knum;k++){
    			temp->key[k]=tembuf[j].key;
    			temp->next[k]=tembuf[j].next;
    			j++;
    		}
    		temp->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM]=next_neighbor;  //point to the right neighbor
    		temp->cur_knum=remain_knum;
    	}
		nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(loc[i],nativebtree<BCELL,KEY_TYPE>::disk);
    }
    for(i=0;i<max_node_num-1;i++){
   //	 LID lbro_pid[], LID rbro_pid[]
   	 lbro_pid[i]=loc[i];
   	 rbro_pid[i]=loc[i+1];
    }
    delete[] loc;
    delete[] tembuf;
    return SPLIT_LEAF;
}


template <typename BCELL, typename KEY_TYPE>
bool BloomFilter_Btree<BCELL,KEY_TYPE>::insertsplitleaf_spinner(LID curnode_pid,int freespace, KEY_TYPE & splitKey, LID & lbro_pid, LID & rbro_pid,int split_num,LID lch[],LID rch[]){
	   int i,j;
	   KEY_TYPE tkey;
	   for(i=0;i<freespace;i++){
		   tkey=nativebtree<BCELL, KEY_TYPE>::getKeyLocate(rch[i], 0);
		   nativebtree<BCELL,KEY_TYPE>::insertNonFullInner(tkey, -1, lch[i],rch[i],curnode_pid );
	   }
	   _split_intree_num++;
	   nativebtree<BCELL,KEY_TYPE>::splitInner(curnode_pid, splitKey,lbro_pid,rbro_pid);
	   if(freespace==split_num-1)
		   return SPLIT;
	   else if(freespace<split_num-1)
	   {
	      j=split_num-1-freespace;
	      tkey=nativebtree<BCELL, KEY_TYPE>::getKeyLocate(rch[freespace], 0);
	      if(tkey>splitKey){
	          for(i=0;i<j;i++)
	          {
	        	  tkey=nativebtree<BCELL, KEY_TYPE>::getKeyLocate(rch[freespace+i], 0);
	        	  nativebtree<BCELL,KEY_TYPE>::insertNonFullInner(tkey, -1, lch[freespace+i],rch[freespace+i],rbro_pid);
	          }
	      }
	      else if(tkey<splitKey)
	      {
	          for(i=0;i<j;i++)
	          {
	        	  tkey=nativebtree<BCELL, KEY_TYPE>::getKeyLocate(rch[freespace+i], 0);
	        	  nativebtree<BCELL,KEY_TYPE>::insertNonFullInner(tkey, -1, lch[freespace+i],rch[freespace+i],lbro_pid);
	          }
	      }
	   }
	   return SPLIT;
}
template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::turn_solid(LID curnode_pid,int nodeID){
    LeafHeadb *first_node;
    BCELL *temp;
    first_node=get_first_node(curnode_pid);
    set_nodeflag(first_node, nodeID,1);
    LID tpid=get_location(first_node,nodeID);
    temp=get_bcell_key(tpid);
	int *bit_array;
	char *tkey;
    int k;
    bit_array=BIT_ARRAY(first_node, nodeID, bit_bytes);
    bloomfilter->bloom_init( bit_array);
	for(k=0;k<nativebtree<BCELL,KEY_TYPE>::KEY_NUM;k++){
        tkey=(char *)(&(temp->key[k]));
        bloomfilter->bloom_set(bit_array,tkey,sizeof(KEY_TYPE));
	}
	set_krange_firstnode(first_node,temp->key[0],temp->key[temp->cur_knum-1],nodeID);
	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(curnode_pid,nativebtree<BCELL,KEY_TYPE>::disk);
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::turn_active(LID curnode_pid,int nodeID){
    LeafHeadb *first_node;
    first_node=get_first_node(curnode_pid);
    set_nodeflag(first_node, nodeID,1);
    set_active(first_node, nodeID);
    int * bit_array=BIT_ARRAY(first_node, nodeID, bit_bytes);
    bloomfilter->bloom_init(bit_array);
    init_krange_firstnode(first_node,nodeID);
	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(curnode_pid,nativebtree<BCELL,KEY_TYPE>::disk);

}
template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::insert_leafnode_h(LID curnode_pid,KEY_TYPE key,VALUE_TYPE value){
	LID keyloc;
	keyloc=search_kloc_leafnode_h(curnode_pid,key);
	if(keyloc!=NONTHING){
/*		if(keyloc==curnode_pid)
			update_kval_firstnode(keyloc,key,value);
		else
			nativebtree<BCELL, KEY_TYPE>::insertNonFullLeaf(key,value,keyloc);*/
		return NO_SPLIT;
	}
	if(insert_kval_firstnode(curnode_pid,key,value)!=0)
		return NO_SPLIT;
	LID activepid;
	LeafHeadb *first_node=get_first_node(curnode_pid);
	int active_nodeid=ACTIVE(first_node);
	bool splitfalg=false;
	//int cur_node_num = CUR_NODE_NUM(first_node);
	if(active_nodeid==ACTIVE_NO){
		active_nodeid=get_new_active(first_node);
        if(active_nodeid==-1)
        	splitfalg=true;
        else
        	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(curnode_pid,nativebtree<BCELL,KEY_TYPE>::disk);
	}
	if(splitfalg==false){
		activepid=get_location(first_node,active_nodeid);
		nativebtree<BCELL,KEY_TYPE>::insertNonFullLeaf(key, value,activepid);
		if(nativebtree<BCELL,KEY_TYPE>::isNodeFull(activepid) == NODE_SPACE_FULL){
			turn_solid(curnode_pid,active_nodeid);
			first_node=get_first_node(curnode_pid);
			//set_active(first_node, ACTIVE_NO);
			set_active_no(first_node);
        	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(curnode_pid,nativebtree<BCELL,KEY_TYPE>::disk);
		}
		return NO_SPLIT;
	}
	else
		return SPLIT_LEAF;
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::insert_leafnode_t(LID curnode_pid,KEY_TYPE key,VALUE_TYPE value){
	int keyloc;
	keyloc=search_kloc_leafnode_t(curnode_pid,key);
	if(keyloc!=NONTHING){
		//nativebtree<BCELL,KEY_TYPE>::insertNonFullLeaf(key, value,keyloc);
		return 0;
	}
	int *loc=new int[of_num_tradeoff+1];
	int i=0;
	for(i=0;i<of_num_tradeoff;i++)
		loc[i]=-1;
	LID tpid=curnode_pid;
	LID tpid2;
	int keynum;
	BCELL *curcell;
	BCELL *nextcell;
	LID pre_next;
	for(i=0;i<of_num_tradeoff;i++){
		loc[i]=tpid;
		keynum=nativebtree<BCELL,KEY_TYPE>::getKeyNum(tpid);
		if(keynum<nativebtree<BCELL,KEY_TYPE>::KEY_NUM){
			nativebtree<BCELL,KEY_TYPE>::insertNonFullLeaf(key, value,tpid );
			delete[] loc;
			loc=NULL;
			return 0;
		}
		else{
			curcell=get_bcell_key(tpid);
			tpid2=curcell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM];
			if(is_last_page_of_leaf(tpid)&&(i<of_num_tradeoff-1)){
				tpid2=nativebtree<BCELL,KEY_TYPE>::applynode();
				LID pre_next=curcell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM];
				curcell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM]=tpid2;
                set_page_non_last(curcell);
                nextcell=get_bcell_key(tpid2);
                nextcell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM]=pre_next;
				nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(tpid,nativebtree<BCELL,KEY_TYPE>::disk);
				nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(tpid2,nativebtree<BCELL,KEY_TYPE>::disk);
			}

			tpid=tpid2;
		}
	}
	loc[of_num_tradeoff]=tpid;  //this location links to the right neighbor node
	turn_leafnode(loc);
	if(kv_num>0)
	   insert_kval_firstnode(curnode_pid,key,value);
	else{
		LeafHeadb *first_node=get_first_node(curnode_pid);
		int active_nodeid=get_new_active(first_node);
		LID activepid=get_location(first_node,active_nodeid);
		nativebtree<BCELL,KEY_TYPE>::insertNonFullLeaf(key, value,activepid);
	}
	delete[] loc;
	loc=NULL;
	return 1;
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::init_firstnode(LID first_pid){
	BYTE *b=nativebtree<BCELL,KEY_TYPE>::bufctl->readbuf(first_pid, nativebtree<BCELL,KEY_TYPE>::disk);
	int len=HEAD_eBYTE_NUM-1+max_overflow_node_num*bit_bytes;
	memset(b, 0, len);
	memset(b+len, -1,PAGESIZE-1-len);
	*(b+PAGESIZE-1) = 1;
	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(first_pid,nativebtree<BCELL,KEY_TYPE>::disk);
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::get_krange_leafpage(BCELL *curcell,KEY_TYPE *key_min,KEY_TYPE *key_max){
	int keynum=curcell->cur_knum;
	*key_min=curcell->key[0];
	*key_max=curcell->key[keynum-1];
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::turn_leafnode(int *loc){
	int i;
	LID first_pid=loc[0];
	BCELL *new_cell;
	BCELL *cell;
	char *tkey;
	int *bit_array;
	BCELL *temp;
	int k;
	KEY_TYPE keymin,keymax;
	LID newpid=nativebtree<BCELL,KEY_TYPE>::applynode();
	cell=get_bcell_key(loc[0]);
	new_cell=get_bcell_key(newpid);
	for(i=0;i<nativebtree<BCELL,KEY_TYPE>::KEY_NUM;i++){
		new_cell->key[i]=cell->key[i];
		new_cell->next[i]=cell->next[i];
	}
	new_cell->next[i]=cell->next[i];
	new_cell->cur_knum=cell->cur_knum;
	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(newpid,nativebtree<BCELL,KEY_TYPE>::disk);

	loc[0]=newpid;
	init_firstnode(first_pid);
	//int node_type=judge_node_type(first_pid);
	LeafHeadb *first_node=get_first_node(first_pid);
	set_cur_nodenum(first_node,of_num_tradeoff);
	set_active_no(first_node);
	set_location(first_node,max_overflow_node_num, loc[of_num_tradeoff]);  //set the pointer to next neighbor leaf
	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(first_pid,nativebtree<BCELL,KEY_TYPE>::disk);
    for(i=0;i<of_num_tradeoff;i++){
    	first_node=get_first_node(first_pid);
    	set_nodeflag(first_node, i, 1);
    	set_location(first_node,i, loc[i]);
        bit_array=BIT_ARRAY(first_node,i, bit_bytes);
        bloomfilter->bloom_init( bit_array);
        temp=get_bcell_key(loc[i]);
    	for(k=0;k<nativebtree<BCELL,KEY_TYPE>::KEY_NUM;k++){
            tkey=(char *)(&(temp->key[k]));
            bloomfilter->bloom_set(bit_array,tkey,sizeof(KEY_TYPE));
    	}
    	get_krange_leafpage(temp,&keymin,&keymax);
    	set_krange_firstnode(first_node,keymin,keymax,i);
    	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(first_pid,nativebtree<BCELL,KEY_TYPE>::disk);
    }
}


template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::insertNode_bf(KEY_TYPE key, VALUE_TYPE value, LID curnode_pid, LID  lbro_pid[ ], KEY_TYPE & splitKey, LID  rbro_pid[ ], int level){
	level++;
	int ilocate;
	//int keyloc;
	//LID tpid;
	//LID tpid2;
	LID next_pid;
	LID nextl_pid[max_overflow_node_num];
	LID nextr_pid[max_overflow_node_num];
	KEY_TYPE nextSplitKey,tkey;
	int state;
	int freespace;
	int i,k;
	BCELL * curcell;
	int node_type;
	if(level == (nativebtree<BCELL,KEY_TYPE>::high-1)){
		node_type=judge_node_type(curnode_pid);
		if(node_type==0){
			insert_leafnode_t(curnode_pid,key,value);
			return NO_SPLIT;
		}
		else if(node_type==1){
			state=insert_leafnode_h(curnode_pid,key,value);
			if(state==NO_SPLIT)
				return NO_SPLIT;
			else if(state==SPLIT_LEAF){
				_split_leaf_num++;
				splitofLeafssd(curnode_pid,lbro_pid,rbro_pid,key,value);
				return SPLIT_LEAF;
			}
		}
	}
	else{
		int num =nativebtree<BCELL,KEY_TYPE>:: getKeyNum(curnode_pid);
		curcell =get_bcell_key(curnode_pid);
		for(k=0; k<num; k++)
		{
			if(key < curcell->key[k])
				break;
		}
		ilocate = k;
		next_pid = curcell->next[k];
		state=insertNode_bf(key,value,next_pid,nextl_pid,nextSplitKey, nextr_pid,level);
		if(state==NO_SPLIT)
			return NO_SPLIT;
		else{
			if(state==SPLIT_LEAF)
			{
				freespace=nativebtree<BCELL,KEY_TYPE>::KEY_NUM-num;
				if(freespace>max_overflow_node_num)
				{
					   for(i=0;i<max_overflow_node_num;i++){
						   tkey=nativebtree<BCELL, KEY_TYPE>::getKeyLocate(nextr_pid[i], 0);
						   nativebtree<BCELL,KEY_TYPE>::insertNonFullInner(tkey, -1, nextl_pid[i],nextr_pid[i],curnode_pid );
					   }
					   return NO_SPLIT;
				}
				else
				{
				   insertsplitleaf_spinner(curnode_pid,freespace, splitKey,lbro_pid[0],rbro_pid[0],max_overflow_node_num+1,nextl_pid,nextr_pid);
				   return SPLIT;
				}
			}
			else if(state==SPLIT)
			{

				nativebtree<BCELL,KEY_TYPE>::insertNonFullInner(nextSplitKey, ilocate, nextl_pid[0], nextr_pid[0], curnode_pid);
				if(nativebtree<BCELL,KEY_TYPE>::isNodeFull(curnode_pid) == NODE_SPACE_FULL)
				{
					_split_intree_num++;
					nativebtree<BCELL,KEY_TYPE>::splitInner(curnode_pid, splitKey, lbro_pid[0], rbro_pid[0]);
					return SPLIT;
				}
				else
					return NO_SPLIT;
			}

		}
	}
}

template <typename BCELL, typename KEY_TYPE>
bool BloomFilter_Btree<BCELL,KEY_TYPE>::is_node_active(LID firstnode_pid,LID nodepid){
	LeafHeadb *first_node=get_first_node(firstnode_pid);
	int active_nodeid;
	LID active_nodepid;
	active_nodeid=ACTIVE(first_node);
	active_nodepid=get_location(first_node,active_nodeid);
	if(active_nodepid==nodepid)
		return true;
	else
		return false;
}
template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::removekeyfirstpage(KEY_TYPE key, LID cur_pid){
	LeafHeadb *first_node=get_first_node(cur_pid);
	KEY_TYPE *poskey=KEY_IN_HEAD(first_node, bit_bytes);
	int cur_num=get_firstnode_KeyNum(poskey);
	int i;
	int target=-1;
	for(i=0; i<cur_num; i++)
	{
		if(poskey[i] == key)
		{
			target = i;
			break;
		}
	}
	if(target == -1)
		return 0;
	VALUE_TYPE *posvalue=VALUE_IN_HEAD(first_node, bit_bytes);
	for(i=target; i<cur_num-1; i++)
	{
		poskey[i] = poskey[i+1];
		posvalue[i] =posvalue[i+1];
	}
	poskey[cur_num-1] = NONTHING;
	posvalue[cur_num-1] = NONTHING;
	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(cur_pid,nativebtree<BCELL,KEY_TYPE>::disk);
	return 1;
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::deletepage(LID first_pid,LID node_pid,int nodeid){
	LeafHeadb *first_node=get_first_node(first_pid);
	set_nodeflag(first_node,nodeid,0);
	int cur_nodenum=CUR_NODE_NUM(first_node);
	cur_nodenum--;
	set_cur_nodenum(first_node,cur_nodenum);
	set_location(first_node,nodeid,-1);
	int *bitarray=BIT_ARRAY(first_node, nodeid, bit_bytes);
	bloomfilter->bloom_init(bitarray);
	init_krange_firstnode(first_node,nodeid);
	if(nodeid==ACTIVE(first_node))
		set_active_no(first_node);
    nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(first_pid, nativebtree<BCELL,KEY_TYPE>::disk);
    nativebtree<BCELL,KEY_TYPE>::bufctl->deletepage(node_pid,nativebtree<BCELL,KEY_TYPE>::disk);

}

template <typename BCELL, typename KEY_TYPE>
bool BloomFilter_Btree<BCELL,KEY_TYPE>::is_leaf_empty_h(LID first_pid){
	LeafHeadb *first_node=get_first_node(first_pid);
	int cur_nodenum=CUR_NODE_NUM(first_node);
	if(cur_nodenum==0){
		if(kv_num==0)
			return 1;
		KEY_TYPE *poskey=KEY_IN_HEAD(first_node, bit_bytes);
		if(poskey[0]==-1)
			return 1;
		else
			return 0;
	}
	else
		return 0;

}

template <typename BCELL, typename KEY_TYPE>
bool BloomFilter_Btree<BCELL,KEY_TYPE>::is_leaf_empty_t(LID first_pid){
	int keynum= nativebtree<BCELL,KEY_TYPE>::getKeyNum(first_pid);
	if(keynum==0)
		return 1;
	else
		return 0;
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::removekey_nonactivepage(KEY_TYPE key, LID cur_pid,LID keylocate){
	nativebtree<BCELL,KEY_TYPE>::removeKeyNonMergeLeaf(key,keylocate);
	LeafHeadb *first_node=get_first_node(cur_pid);
	int i,location;
	for(i=0;i<max_overflow_node_num;i++){
		location=get_location(first_node,i);
		if(location==keylocate)
			break;
	}
	if(get_nodeflag(first_node,i)==1){
		set_nodeflag(first_node,i, 0);
	    nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(cur_pid, nativebtree<BCELL,KEY_TYPE>::disk);
	}

}
template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::removeKeyAllLeaf_h(KEY_TYPE key, LID cur_pid){
	LID keyloc=search_kloc_leafnode_h(cur_pid,key);
	int location,keynum,key_num_active,keycount;
	int borrownum;
	BCELL *cur_cell;
	BCELL *active_cell;
	double cur_error;
	int i,k;
	if(keyloc==-1)
		return 0;
    if(keyloc==cur_pid)
    	removekeyfirstpage(key,cur_pid);
    else{

    	LeafHeadb *first_node=get_first_node(cur_pid);
        int active_nodeid=ACTIVE(first_node);
        if(active_nodeid!=ACTIVE_NO){
        	location=get_location(first_node,active_nodeid);
        	if(location==keyloc){
        		nativebtree<BCELL,KEY_TYPE>::removeKeyNonMergeLeaf(key,location);
        		keynum=nativebtree<BCELL,KEY_TYPE>::getKeyNum(location);
        		if(keynum>0)
        			return 0;
        		else
        			deletepage(cur_pid,location,active_nodeid);

        	}else{
        		//nativebtree<BCELL,KEY_TYPE>::removeKeyNonMergeLeaf(key,keyloc);
        		removekey_nonactivepage(key,cur_pid,keyloc);
        		keynum=nativebtree<BCELL,KEY_TYPE>::getKeyNum(keyloc);
        		cur_error=bloomfilter->cur_error_get(keynum);
        		if(cur_error>=node_err){
        			key_num_active=nativebtree<BCELL,KEY_TYPE>::getKeyNum(location);
        			keycount=keynum+key_num_active;
        			if(keycount<=nativebtree<BCELL,KEY_TYPE>::KEY_NUM){
        				cur_cell=get_bcell_key(keyloc);
        				active_cell=get_bcell_key(location);
        				k=0;
        				for(k=0;k<active_cell->cur_knum;k++)
        					nativebtree<BCELL,KEY_TYPE>::insertNonFullLeaf(active_cell->key[k], active_cell->next[k], keyloc);
        				cur_cell->cur_knum=keycount;
        			    nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(keyloc, nativebtree<BCELL,KEY_TYPE>::disk);
        			    deletepage(cur_pid,location,active_nodeid);
        			   // nativebtree<BCELL,KEY_TYPE>::bufctl->deletepage(location,nativebtree<BCELL,KEY_TYPE>::disk);
            			first_node=get_first_node(cur_pid);
            			for(i=0;i<max_overflow_node_num;i++){
            				if(get_location(first_node, i)==keyloc)
            					break;
            			}
        			    if(keycount<nativebtree<BCELL,KEY_TYPE>::KEY_NUM)
        			    	turn_active(cur_pid,i);
        			    else if(keycount==nativebtree<BCELL,KEY_TYPE>::KEY_NUM)
        			    	turn_solid(cur_pid,i);
        			}
        			else if(keycount>nativebtree<BCELL,KEY_TYPE>::KEY_NUM){
        				borrownum=nativebtree<BCELL,KEY_TYPE>::KEY_NUM-keynum;


        				for(i=0;i<borrownum;i++){
        					//cur_cell->key[keynum+0]=active_cell->key[i];
        					//cur_cell->next[keynum+0]=active_cell->next[i];
        					active_cell=get_bcell_key(location);
        					nativebtree<BCELL,KEY_TYPE>::insertNonFullLeaf(active_cell->key[i], active_cell->next[i],keyloc);
        				}
        				cur_cell=get_bcell_key(keyloc);
        				cur_cell->cur_knum=nativebtree<BCELL,KEY_TYPE>::KEY_NUM;
        				nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(keyloc, nativebtree<BCELL,KEY_TYPE>::disk);
        				for(i=0;i<key_num_active-borrownum;i++){
        					active_cell->key[i]=active_cell->key[borrownum+i];
        					active_cell->key[borrownum+i]=-NONTHING;
        					active_cell->next[i]=active_cell->next[borrownum+i];
        					active_cell->next[borrownum+i]=NONTHING;
        				}
        				active_cell->cur_knum=key_num_active-borrownum;
        				nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(location, nativebtree<BCELL,KEY_TYPE>::disk);
            			first_node=get_first_node(cur_pid);
            			for(i=0;i<max_overflow_node_num;i++){
            				if(get_location(first_node, i)==keyloc)
            					break;
            			}
            			turn_solid(cur_pid,i);
            			return 0;
        			}
        		}
        		else
        			return 0;
        	}
        }
        else if(active_nodeid==ACTIVE_NO){
    		//nativebtree<BCELL,KEY_TYPE>::removeKeyNonMergeLeaf(key,keyloc);
        	removekey_nonactivepage(key,cur_pid,keyloc);
    		keynum=nativebtree<BCELL,KEY_TYPE>::getKeyNum(keyloc);
    		cur_error=bloomfilter->cur_error_get(keynum);
    		if(cur_error>=node_err){
    			first_node=get_first_node(cur_pid);
    			for(i=0;i<max_overflow_node_num;i++){
    				if(get_location(first_node, i)==keyloc)
    					break;
    			}
    			turn_active(cur_pid,i);
    		}

        }

    }
    if(is_leaf_empty_h(cur_pid))
    	return 1;
    else
    	return 0;
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::removeKeyAllLeaf_t(KEY_TYPE key, LID cur_pid){
	LID tpid,nextpid,prepid;
	BCELL * cur_cell;
	BCELL *next_cell;
	tpid=cur_pid;
	prepid=-1;

	while(1){
		nextpid=removeKeyOFNode(key,tpid,prepid);
		if(nextpid==-1)
			break;
		else{
			prepid=tpid;
			tpid=nextpid;
		}
	}
	int first_keynum=nativebtree<BCELL,KEY_TYPE>::getKeyNum(cur_pid);
	int next_keynum,i;
	if(first_keynum==0){
		cur_cell = get_bcell_key(cur_pid);
		nextpid=cur_cell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM];
		if(is_last_page_of_leaf(cur_pid)){
	        //nativebtree<BCELL,KEY_TYPE>::bufctl->deletepage(cur_pid,nativebtree<BCELL,KEY_TYPE>::disk);
            return 1;
		}
		else{
			next_keynum=nativebtree<BCELL,KEY_TYPE>::getKeyNum(nextpid);
			cur_cell = get_bcell_key(cur_pid);
			next_cell=get_bcell_key(nextpid);
			for(i=0;i<next_keynum;i++){
				cur_cell->key[i]=next_cell->key[i];
				cur_cell->next[i]=next_cell->next[i];
			}
			cur_cell->cur_knum=next_cell->cur_knum;
			cur_cell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM]=next_cell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM];
			if(is_last_page_of_leaf(nextpid))
				set_page_last(cur_cell);
			nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(cur_pid, nativebtree<BCELL,KEY_TYPE>::disk);
	        nativebtree<BCELL,KEY_TYPE>::bufctl->deletepage(nextpid,nativebtree<BCELL,KEY_TYPE>::disk);
	        return 0;
		}
	}
	else
		return 0;
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::removeKeyOFNode(KEY_TYPE key, LID cur_pid,LID pre_id){
	//remove key from overflownode,if success return -1,else return nextpage id
	int target = -1;
	BCELL * cur_cell = get_bcell_key(cur_pid);
	BCELL *pre_cell;
	int cur_num = nativebtree<BCELL,KEY_TYPE>::getKeyNum(cur_pid);
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
		if(is_last_page_of_leaf(cur_pid))
			return -1;
		else
		    return cur_cell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM];
	}

	for(i=target; i<cur_num-1; i++)
	{
		cur_cell->key[i] = cur_cell->key[i+1];
		cur_cell->next[i] = cur_cell->next[i+1];
	}
	cur_cell->key[cur_num-1] = NONTHING;
	cur_cell->next[cur_num-1] = NONTHING;
	cur_cell->cur_knum--;
	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(cur_pid, nativebtree<BCELL,KEY_TYPE>::disk);
   if(cur_num==1&&pre_id!=-1){
		pre_cell=get_bcell_key(pre_id);
		cur_cell = get_bcell_key(cur_pid);
		pre_cell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM]=cur_cell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM];
		if(is_last_page_of_leaf(cur_pid))
			set_page_last(pre_cell);
        nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(pre_id, nativebtree<BCELL,KEY_TYPE>::disk);
        nativebtree<BCELL,KEY_TYPE>::bufctl->deletepage(cur_pid,nativebtree<BCELL,KEY_TYPE>::disk);
   }
	return -1;
}
template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::insertbfKey(KEY_TYPE key, VALUE_TYPE value){
	LID aNewNode;
    LID nextLson[max_overflow_node_num];
    LID nextRson[max_overflow_node_num];
    LID rbro[max_overflow_node_num];
    LID lbro[max_overflow_node_num];
	int iLocate;
	LID nextNode;
	int i,j,state;
	int num;
	int freespace;
	BCELL * rootcell;
	//BCELL *curcell;
	KEY_TYPE nextSplitK;
	KEY_TYPE splitKey;
	KEY_TYPE tkey;
	int level = 0;
	int node_type;
	//LeafHeadb *first_node;
	if(nativebtree<BCELL,KEY_TYPE>::root_pid < 0)
	{
		aNewNode = nativebtree<BCELL,KEY_TYPE>::applynode();
		insert_leafnode_t(aNewNode,key,value);
		nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(aNewNode,nativebtree<BCELL,KEY_TYPE>::disk);
		nativebtree<BCELL,KEY_TYPE>::root_pid = aNewNode;
		nativebtree<BCELL,KEY_TYPE>::high++;
	}
	else
	{
		if(nativebtree<BCELL,KEY_TYPE>::high == 1)
		{
			node_type=judge_node_type(nativebtree<BCELL,KEY_TYPE>::root_pid);
			if(node_type==0){
				insert_leafnode_t(nativebtree<BCELL,KEY_TYPE>::root_pid,key,value);
				return ;
			}
			else if(node_type==1){
				state=insert_leafnode_h(nativebtree<BCELL,KEY_TYPE>::root_pid,key,value);
				if(state==NO_SPLIT)
					return ;
				else if(state==SPLIT_LEAF){
					_split_leaf_num++;
					splitofLeafssd(nativebtree<BCELL,KEY_TYPE>::root_pid,lbro,rbro,key,value);
					aNewNode =nativebtree<BCELL,KEY_TYPE>::applynode();
					  for(j=0;j<max_split_count-1;j++){
						   tkey=nativebtree<BCELL, KEY_TYPE>::getKeyLocate( rbro[j], 0);
						   nativebtree<BCELL,KEY_TYPE>::insertNonFullInner(tkey, -1, lbro[j],rbro[j],aNewNode);
					   }
					  nativebtree<BCELL,KEY_TYPE>::root_pid=aNewNode;
					  nativebtree<BCELL,KEY_TYPE>::high++;
					  return ;
				}
			}

		}
		else
		{
		num =nativebtree<BCELL,KEY_TYPE>:: getKeyNum(nativebtree<BCELL,KEY_TYPE>::root_pid);
		rootcell=get_bcell_key(nativebtree<BCELL,KEY_TYPE>::root_pid);
		for(i=0; i<num; i++)
		{
			if(key < rootcell->key[i])
				break;
		}
		iLocate = i;
		nextNode = rootcell->next[i];
		state=insertNode_bf(key,value,nextNode,nextLson,nextSplitK,nextRson,level);
		if(state==SPLIT)
		{
			nativebtree<BCELL,KEY_TYPE>::insertNonFullInner(nextSplitK, iLocate, nextLson[0], nextRson[0],nativebtree<BCELL,KEY_TYPE>::root_pid);
			if(nativebtree<BCELL,KEY_TYPE>::isNodeFull(nativebtree<BCELL,KEY_TYPE>::root_pid) == NODE_SPACE_FULL)
			{
				_split_intree_num++;
				aNewNode = nativebtree<BCELL,KEY_TYPE>::applynode();
				nativebtree<BCELL,KEY_TYPE>::splitInner(nativebtree<BCELL,KEY_TYPE>::root_pid, splitKey, lbro[0], rbro[0]);
				nativebtree<BCELL,KEY_TYPE>::insertNonFullInner(splitKey, -1, lbro[0], rbro[0], aNewNode);
				nativebtree<BCELL,KEY_TYPE>::root_pid=aNewNode;
				nativebtree<BCELL,KEY_TYPE>::high++;
			}
		}
		else if(state==SPLIT_LEAF)
		{
			freespace=nativebtree<BCELL,KEY_TYPE>::KEY_NUM-num;
			if(freespace>max_split_count-1)
			{
				   for(i=0;i<max_split_count-1;i++){
					   tkey=nativebtree<BCELL, KEY_TYPE>::getKeyLocate(nextRson[i], 0);
					   nativebtree<BCELL,KEY_TYPE>::insertNonFullInner(tkey, -1,nextLson[i], nextRson[i],nativebtree<BCELL,KEY_TYPE>::root_pid);
				   }

			}
			else
			{
			   insertsplitleaf_spinner(nativebtree<BCELL,KEY_TYPE>::root_pid,freespace, splitKey,lbro[0],rbro[0],max_split_count,nextLson,nextRson);
			   aNewNode = nativebtree<BCELL,KEY_TYPE>:: applynode();
			   nativebtree<BCELL,KEY_TYPE>::insertNonFullInner(splitKey, -1, lbro[0], rbro[0], aNewNode);
			   nativebtree<BCELL,KEY_TYPE>::root_pid=aNewNode;
			   nativebtree<BCELL,KEY_TYPE>::high++;
			}
		}
		}
	}
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::maintain_link_copynext(LID parentpid,int locate){
	BCELL * cur_cell=get_bcell_key(parentpid);
	if(locate>=cur_cell->cur_knum){
		cout<<"error-->invalid locate"<<endl;
		exit(0);
	}
	LID leftpid=cur_cell->next[locate];
	LID rightpid=cur_cell->next[locate+1];
	BCELL *left_cell=get_bcell_key(leftpid);
	BCELL *right_cell=get_bcell_key(rightpid);
	memcpy(left_cell,right_cell,PAGE_SIZE);
	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(leftpid,nativebtree<BCELL,KEY_TYPE>::disk);
	cur_cell->next[locate+1]=leftpid;
	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(parentpid,nativebtree<BCELL,KEY_TYPE>::disk);
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::maintain_link_modify_pre_next(LID parentpid,int locate){
	BCELL * cur_cell=get_bcell_key(parentpid);
	if(locate!=cur_cell->cur_knum){
		cout<<"error-->invalid locate in function maintain_link_modify_pre_next"<<endl;
		exit(0);
	}
	LID rightpid=cur_cell->next[locate];
	LID leftpid=cur_cell->next[locate-1];
	LID cur_nextpid;
	//we need to first get the nextpid of the rightpid
	int node_type=judge_node_type(rightpid);
	if(node_type==1){
		LeafHeadb *leafhead=get_first_node(rightpid);
		cur_nextpid=get_location(leafhead, max_overflow_node_num);
	}
	else if(node_type==0){
		if(!is_last_page_of_leaf(rightpid)){
			cout<<"error-->this is not the last page of leaf"<<endl;
			exit(0);
		}
		BCELL *cell=get_bcell_key(rightpid);
		cur_nextpid=cell->next[nativebtree<BCELL, KEY_TYPE>::KEY_NUM];
	}
	else{
		cout<<"error-->invalid node_type in maintain_link_modify_pre_next--1"<<endl;
		exit(0);
	}

	//now we need to modify the left node->next to right node ->next
	node_type=judge_node_type(leftpid);
	if(node_type==1){  //this page is a leaf head of BF-Leaf
		LeafHeadb *first_node=get_first_node(leftpid);
		LID right=get_location(first_node, max_overflow_node_num);
		if(right!=rightpid){
			cout<<"eror-->pre->nextpid!=curpid"<<endl;
			exit(0);
		}
		set_location(first_node, max_overflow_node_num, cur_nextpid);
		nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(leftpid, nativebtree<BCELL,KEY_TYPE>::disk);
	}
	else if(node_type==0){
		LID tpid=leftpid;
		BCELL *precell;
		for(int i=0;i<of_num_tradeoff;i++){
			precell=get_bcell_key(tpid);
	        if(is_last_page_of_leaf(tpid)){
	        	if(precell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM]!=rightpid){
	    			cout<<"eror-->pre->nextpid!=curpid--2"<<endl;
	    			exit(0);
	        	}
	        	precell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM]=cur_nextpid;
	        	nativebtree<BCELL,KEY_TYPE>::bufctl->writebuf(tpid, nativebtree<BCELL,KEY_TYPE>::disk);
	        	return;
	        }
			tpid=precell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM];
		}

	}
	else{
		cout<<"error-->invalid node_type in maintain_link_modify_pre_next--2"<<endl;
		exit(0);
	}
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::bf_easeRemoveKey(KEY_TYPE key, LID node_pid,LID parent_pid, LID & target_pid,int locateson, int level,int flag){
	level++;
	int target = -1;
	int num,node_type;
	int j,f,sonlocation,state;
	BCELL * cur_cell,*parent_cell;
	LID deletepid;
	if(level == nativebtree<BCELL,KEY_TYPE>::high-1) { //leaf
		node_type=judge_node_type(node_pid);
		if(node_type==1)
		    state=removeKeyAllLeaf_h(key,node_pid);
		else if(node_type==0)
			state=removeKeyAllLeaf_t(key,node_pid);
        if(state==0)
        	return 0;
        else if(state==1){
        	if(nativebtree<BCELL,KEY_TYPE>::high==1){
        		return 0;
        	}
        	else if(nativebtree<BCELL,KEY_TYPE>::high>1){
				//nativebtree<BCELL,KEY_TYPE>::bufctl->deletepage(node_pid,nativebtree<BCELL,KEY_TYPE>::disk);
	            if(flag==0){
	            	cur_cell=get_bcell_key(parent_pid);
	            	deletepid=cur_cell->next[locateson+1];
	            	maintain_link_copynext(parent_pid,locateson);
	            	nativebtree<BCELL, KEY_TYPE>::removeKeyWithLeftSonNonMergeInner(locateson,parent_pid);
	            	nativebtree<BCELL,KEY_TYPE>::bufctl->deletepage(deletepid,nativebtree<BCELL,KEY_TYPE>::disk);
	            	target_pid=parent_pid;
	            }
	            else if(flag==1){
	            	cur_cell=get_bcell_key(parent_pid);
	            	if(locateson<cur_cell->cur_knum-1){
		            	deletepid=cur_cell->next[locateson+2];
		            	maintain_link_copynext(parent_pid,locateson+1);
	            	}else{  //locateson==cur_cell->cur_knum-1,right pid=locateson+1==cur_cell->cur_knum
	            		maintain_link_modify_pre_next(parent_pid,locateson+1);
	            		deletepid=node_pid;
	            	}
	            	nativebtree<BCELL, KEY_TYPE>::removeKeyWithRightSonNonMergeInner(locateson,parent_pid);
	            	nativebtree<BCELL,KEY_TYPE>::bufctl->deletepage(deletepid,nativebtree<BCELL,KEY_TYPE>::disk);
	            	target_pid=parent_pid;
	            }
	            return 1;
        	}
        }
	}
	else{   //inner node
		num = nativebtree<BCELL, KEY_TYPE>::getKeyNum(node_pid);
		cur_cell=get_bcell_key(node_pid);
		f=0;
		for(j=0; j<num; j++)
		{
			if(key <= cur_cell->key[j])
			{
				sonlocation=j;
				if(key == cur_cell->key[j])
				{
					j++;
					f=1;
				}
				break;
			}
		}
		target = j;
        if(target==num){
        	f=1;
        	sonlocation=num-1;
        }
		state=bf_easeRemoveKey(key,cur_cell->next[target],node_pid,target_pid,sonlocation, level,f);
		return state;
	}
	return -1;
}

template <typename BCELL, typename KEY_TYPE>
TREE_STATE_TYPE BloomFilter_Btree<BCELL,KEY_TYPE>::bf_AfterRemoveKey(LID & target_pid, LID curnode_pid, int level){
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
		if(target_pid ==nativebtree<BCELL, KEY_TYPE>::root_pid){
			if(nativebtree<BCELL, KEY_TYPE>::getKeyNum(target_pid) < 1)
			{
				curnode_cell = get_bcell_key(target_pid);
				nativebtree<BCELL, KEY_TYPE>::root_pid = curnode_cell->next[0];
				nativebtree<BCELL, KEY_TYPE>::bufctl->deletepage(curnode_pid,nativebtree<BCELL, KEY_TYPE>::disk);
				nativebtree<BCELL, KEY_TYPE>::high--;
				 return EASY_OK;
			}
		}
		if(nativebtree<BCELL, KEY_TYPE>::getKeyNum(curnode_pid) < (nativebtree<BCELL, KEY_TYPE>::KEY_NUM-1)/2)
			return CALL_FOR_HELP;
		else
			return EASY_OK;

	}
	else
	{
		num = nativebtree<BCELL, KEY_TYPE>::getKeyNum(curnode_pid);
		target_cell = get_bcell_key(target_pid);
		curnode_cell = get_bcell_key(curnode_pid);
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
		flagHelp = bf_AfterRemoveKey(target_pid, next_pid, level);
		if(flagHelp == EASY_OK)
			return EASY_OK;
		else if(flagHelp == CALL_FOR_HELP)
		{
			flagMerge = bf_handlHelpNode(next_pid, locateNode, curnode_pid, level);
			if(curnode_pid != nativebtree<BCELL, KEY_TYPE>::root_pid)
			{
				if(flagMerge == MERGE_DONE)
				{
					if(nativebtree<BCELL, KEY_TYPE>::getKeyNum(curnode_pid) < (nativebtree<BCELL, KEY_TYPE>::KEY_NUM-1)/2)
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
					if(nativebtree<BCELL, KEY_TYPE>::getKeyNum(curnode_pid) < 1)
					{
						nativebtree<BCELL, KEY_TYPE>::root_pid = curnode_cell->next[0];
						nativebtree<BCELL, KEY_TYPE>::bufctl->deletepage(curnode_pid,nativebtree<BCELL, KEY_TYPE>::disk);
						nativebtree<BCELL, KEY_TYPE>::high--;
					}
				}
				return EASY_OK;
			}
		}
	}
}
template <typename BCELL, typename KEY_TYPE>
TREE_STATE_TYPE BloomFilter_Btree<BCELL,KEY_TYPE>::bf_handlHelpNode(LID cur_pid, int locateson, LID parent_pid, int level){
	level++;
	LID help_pid;
	int delLocate = -1;
	int num_big;
	int num_tiny;
	BCELL * parent_cell;
	parent_cell =get_bcell_key(parent_pid);
	if(locateson > 0 && locateson < nativebtree<BCELL, KEY_TYPE>::getKeyNum(parent_pid))
	{
		num_big = nativebtree<BCELL, KEY_TYPE>::getKeyNum(parent_cell->next[locateson+1]);
		num_tiny = nativebtree<BCELL, KEY_TYPE>::getKeyNum(parent_cell->next[locateson-1]);
		parent_cell = get_bcell_key(parent_pid);
		if(num_big >=(nativebtree<BCELL, KEY_TYPE>::KEY_NUM-1)/2+1 )
			help_pid = parent_cell->next[locateson+1];
		else if(num_tiny >=(nativebtree<BCELL, KEY_TYPE>::KEY_NUM-1)/2+1 )
			help_pid = parent_cell->next[locateson-1];
		else if(num_big == (nativebtree<BCELL, KEY_TYPE>::KEY_NUM-1)/2)
		{
			help_pid = parent_cell->next[locateson+1];
			delLocate = locateson;
		}
		else if(num_tiny ==(nativebtree<BCELL, KEY_TYPE>::KEY_NUM-1)/2)
		{
			help_pid = parent_cell->next[locateson-1];
			delLocate = locateson-1;
		}
	}
	else if(locateson == 0)
	{
		num_big = nativebtree<BCELL, KEY_TYPE>::getKeyNum(parent_cell->next[locateson+1]);
		parent_cell = get_bcell_key(parent_pid);
		if(num_big >= (nativebtree<BCELL, KEY_TYPE>::KEY_NUM-1)/2+1)
			help_pid = parent_cell->next[locateson+1];
		else if(num_big == (nativebtree<BCELL, KEY_TYPE>::KEY_NUM-1)/2)
		{
			help_pid = parent_cell->next[locateson+1];
			delLocate = locateson;
		}
	}
	else if(locateson == nativebtree<BCELL, KEY_TYPE>::getKeyNum(parent_pid))
	{
		num_tiny = nativebtree<BCELL, KEY_TYPE>::getKeyNum(parent_cell->next[locateson-1]);
		parent_cell = get_bcell_key(parent_pid);
		if(num_tiny >= (nativebtree<BCELL, KEY_TYPE>::KEY_NUM-1)/2+1)
			help_pid = parent_cell->next[locateson-1];
		else if(num_tiny == (nativebtree<BCELL, KEY_TYPE>::KEY_NUM-1)/2)
		{
			help_pid = parent_cell->next[locateson-1];
			delLocate = locateson-1;
		}
	}
	if(delLocate == -1)
	{
		nativebtree<BCELL, KEY_TYPE>::unitNonDelInner(cur_pid, locateson, help_pid, parent_pid);
		return MERGE_NO;
	}
	else
	{
		if(delLocate == locateson)
			nativebtree<BCELL, KEY_TYPE>::mergeWithDelInner(cur_pid, help_pid, locateson, parent_pid);
		else if(delLocate == locateson-1)
			nativebtree<BCELL, KEY_TYPE>::mergeWithDelInner(help_pid, cur_pid, locateson-1, parent_pid);
		nativebtree<BCELL, KEY_TYPE>::removeKeyWithRightSonNonMergeInner(delLocate, parent_pid);
		return MERGE_DONE;
	}
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::deletebfKey(KEY_TYPE key){
	if(nativebtree<BCELL, KEY_TYPE>::root_pid == -1)
	{
		/*cout<<"B+ tree is empty, no delete-"<<className()<<endl;
		exit(0)*/;
		return;
	}
	LID target_pid;
	int state;
	state=bf_easeRemoveKey( key,nativebtree<BCELL, KEY_TYPE>::root_pid,-1,target_pid,0, -1,0);
	if(state==1)
		bf_AfterRemoveKey(target_pid,nativebtree<BCELL, KEY_TYPE>::root_pid, -1);
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::scanleafnode_h(LID current_pid,LID *nextpid,KEY_TYPE keymin,KEY_TYPE keymax){

	int flag=0; //if flag assigns 1,it means this leafnode has the key in the range of [keymin,keymax]
    int upper_bound_flag=-1;  //if upper_bound_flag==1,it means the keymax <=the maxkey of this leaf
	//we need to first scan the leaf head to find the target keys
	LeafHeadb *first_node=get_first_node(current_pid);
	*nextpid=get_location(first_node,max_overflow_node_num);
	KEY_TYPE *poskey=KEY_IN_HEAD(first_node, bit_bytes);
	KEY_TYPE tkey;
	int i=0;
	for(i=0;i<kv_num;i++){
        if(poskey[i]==NONTHING)
        	break;
        if(poskey[i]>=keymax){
        	upper_bound_flag=1;
            break;
        }
        if((poskey[i]>=keymin)&&(poskey[i]<=keymax)){
        	flag=1;
        	break;
        }
	}

	//then we need to check the key range of the keys in overflow pages
    int cur_nodenum,active_node,location;
    KEY_TYPE min,max;
    BCELL *curcell;

    cur_nodenum=CUR_NODE_NUM(first_node);
    if(cur_nodenum==0){
    	if(flag==1)
    		return 1;
    	else
    		return 0;
    }

    active_node=ACTIVE(first_node);
    for(i=0;i<max_overflow_node_num;i++){
    	location=get_location(first_node,i);
    	if(location!=-1){
        	if(i!=active_node){
        		get_krange_firstnode(first_node, &min,&max,i);
                if((min<=keymax)||(max>=keymin)){
             	   curcell=get_bcell_key(location);
             	   get_krange_leafpage(curcell,&min,&max);
             	    if(max>=keymax)
             	           upper_bound_flag=1;
                     if((min<=keymax)||(max>=keymin)){
                    	    flag=1;
                    	    search_kloc_leafpage(location,keymin);
                    }
                }
                if(max>=keymax)
                	upper_bound_flag=1;
        	}
        }
    }

    if(active_node!=ACTIVE_NO){
       location=get_location(first_node,active_node);
	   curcell=get_bcell_key(location);
	   get_krange_leafpage(curcell,&min,&max);
	    if(max>=keymax)
	           upper_bound_flag=1;
        if((min<=keymax)||(max>=keymin)){
       	    flag=1;
       	    search_kloc_leafpage(location,keymin);
       }
    }
    if(upper_bound_flag==1)
    	return 0;
	if(flag==1)
		return 1;
	else
		return 0;
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::scanleafnode_t(LID current_pid,LID *nextpid,KEY_TYPE keymin,KEY_TYPE keymax){
	int flag=0;  //if flag assigns 1,it means this leafnode has the key in the range of [keymin,keymax]
	int upper_bound_flag=-1; //if upper_bound_flag==1,it means the keymax <=the maxkey of this leaf
	KEY_TYPE min,max;
	BCELL *curcell;
	LID tpid=current_pid;
	int i;
	for(i=0;i<of_num_tradeoff;i++){
		curcell=get_bcell_key(tpid);
		get_krange_leafpage(curcell,&min,&max);
		if(max>=keymax)
 	           upper_bound_flag=1;
        if((min<=keymax)||(max>=keymin)){
        	flag=1;
        	search_kloc_leafpage(tpid,keymin);
        }
        if(is_last_page_of_leaf(tpid)){
        	*nextpid=curcell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM];
        	break;
        }
		tpid=curcell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM];
	}
    if(upper_bound_flag==1)
    	return 0;
	if(flag==1)
		return 1;
	else
		return 0;
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::scanleafnode(LID current_pid,LID *nextpid,KEY_TYPE keymin,KEY_TYPE keymax){
	if(judge_node_type(current_pid) == 1)
    	return scanleafnode_h(current_pid,nextpid,keymin,keymax);
	else
        return scanleafnode_t(current_pid,nextpid,keymin,keymax);
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::rangesearchbfKeyIn(KEY_TYPE keymin,KEY_TYPE keymax,LID curroot, int level){
	int k = 0;
	LID nextpid=-1;
	level++;
	BCELL * curcell;
	LID curpid=curroot;
    if(level ==(nativebtree<BCELL, KEY_TYPE>:: high-1)){
         while(curpid!=-1){
        	 curcell=get_bcell_key(curroot);
        	 if(scanleafnode(curpid,&nextpid,keymin,keymax)){
        		 curpid=nextpid;
 /*           	 if(curpid==-1)
            	 cout<<"curpid==-1"<<endl;*/
        	 }

        	 else
        		 break;
         }
         return;
    }
	else{
		curcell =get_bcell_key(curroot);
		while(curcell->key[k] != NONTHING)
		{
			if(curcell->key[k] > keymin && k<nativebtree<BCELL, KEY_TYPE>::KEY_NUM)
			{
				if(curcell->next[k] == NONTHING)
				{
					cout<<"Error---> key has no next-"<<className()<<endl;
					exit(0);
				}
				return rangesearchbfKeyIn( keymin,keymax, curcell->next[k], level);
			}
			k++;
			if(k == nativebtree<BCELL, KEY_TYPE>::KEY_NUM)
				break;
		}
		return rangesearchbfKeyIn( keymin,keymax, curcell->next[k], level);
	}
}

template <typename BCELL, typename KEY_TYPE>
VALUE_TYPE BloomFilter_Btree<BCELL,KEY_TYPE>::searchbfKeyIn(KEY_TYPE key, LID curroot, int level){
	int k = 0;
	VALUE_TYPE value;
	level++;
	BCELL * curcell;
   // LID tpid;
    if(level ==(nativebtree<BCELL, KEY_TYPE>:: high-1)){
		if(judge_node_type(curroot) == 1)
	    	value=search_kval_leafnode_h(curroot,key);
		else
		{
			value=search_kval_leafnode_t(curroot,key);;
		}
    	return value;
    }
	else{
		curcell =get_bcell_key(curroot);
		while(curcell->key[k] != NONTHING)
		{
			if(curcell->key[k] > key && k<nativebtree<BCELL, KEY_TYPE>::KEY_NUM)
			{
				if(curcell->next[k] == NONTHING)
				{
					cout<<"Error---> key has no next-"<<className()<<endl;
					exit(0);
				}
				return searchbfKeyIn(key, curcell->next[k], level);
			}
			k++;
			if(k == nativebtree<BCELL, KEY_TYPE>::KEY_NUM)
				break;
		}
		return searchbfKeyIn(key, curcell->next[k], level);
	}
}

template <typename BCELL, typename KEY_TYPE>
VALUE_TYPE BloomFilter_Btree<BCELL,KEY_TYPE>::searchbfKey(KEY_TYPE key){
	if(nativebtree<BCELL, KEY_TYPE>::root_pid < 0)
		return NONTHING;
	return searchbfKeyIn(key, nativebtree<BCELL, KEY_TYPE>::root_pid, -1);

}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::extree_flush(){
	taskentry<KEY_TYPE> *entrybuff;
	int ecount=0;
	flush(&entrybuff,ecount);
	int i=0;
	taskentry<KEY_TYPE> *temp;
	for(i=0;i<ecount;i++){
		temp=(taskentry<KEY_TYPE> *)((BYTE*)entrybuff+i*sizeof(taskentry<KEY_TYPE>));
		if(temp->flag=='I')
			insertbfKey(temp->key, temp->next);
		else if(temp->flag=='D')
			deletebfKey(temp->key);
		else{
			cout<<"error-->in extree_flush() ---flag error"<<endl;
			exit(0);
		}
	}
	bpbuff->delete_te_batch(entrybuff, ecount);
	flush_count=flush_count+ecount;
}

template <typename BCELL, typename KEY_TYPE>
int BloomFilter_Btree<BCELL,KEY_TYPE>::flush(taskentry<KEY_TYPE> **entrybuff,int & ecount){
	int level=1;
	LID nextpid;
	BCELL* cur_cell;
	int knum;
	int i,num,maxnum,startpos,endpos,target_startpos,target_endpos;
	int max_startpos,max_endpos,tkeypos;
	maxnum=-1;
	KEY_TYPE min_value,max_value,first_minvalue,last_maxvalue;

	if( (nativebtree<BCELL,KEY_TYPE>::root_pid < 0)||(nativebtree<BCELL,KEY_TYPE>::high==1) ){
		*entrybuff=(taskentry<KEY_TYPE> *)(bpbuff->buffer);
		ecount=nativebtree<BCELL,KEY_TYPE>::KEY_NUM;
	}
	else{
		target_startpos=1;
		target_endpos=bpbuff->curnum;

		first_minvalue=Infinity;
		last_maxvalue=Infinity;
		nextpid=nativebtree<BCELL,KEY_TYPE>::root_pid;
		while(level<nativebtree<BCELL,KEY_TYPE>::high){
			maxnum=0;
			knum=nativebtree<BCELL,KEY_TYPE>::getKeyNum(nextpid);
            cur_cell=get_bcell_key(nextpid);
    		startpos=target_startpos;
    		endpos=target_endpos;
            for(i=0;i<=knum;i++){

            	if(i==0)
            		min_value=first_minvalue;
            	else
            		min_value=cur_cell->key[i-1];
            	if(i<knum)
            		max_value=cur_cell->key[i];
            	else if(i==knum)
            		max_value=last_maxvalue;
            	num=bpbuff->findvalue(min_value,max_value,startpos,endpos);
            	if(num>maxnum){
            		maxnum=num;
            		max_startpos=startpos;
            		max_endpos=endpos;
            		tkeypos=i;

            	}
            	startpos=endpos+1;
            	endpos=target_endpos;
            	if(startpos>endpos)
            		break;
            }
            target_startpos=max_startpos;
            target_endpos=max_endpos;
    		if(tkeypos>0)
    			first_minvalue=cur_cell->key[tkeypos-1];
    		if(tkeypos<knum)
    			last_maxvalue=cur_cell->key[tkeypos];
    		level++;
    		nextpid=cur_cell->next[tkeypos];

            }
			*entrybuff=(taskentry<KEY_TYPE> *)(bpbuff->buffer)+max_startpos-1;
			ecount=maxnum;

	}
	return 1;
}

template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::printtree(LID node,int level){
	int i,j,num;
	LID tpid;
	ofstream fout("/home/ycc/workspace/bfbtreedata/out",ios::app);
	BCELL* cell;
	LeafHeadb *first_node;
	KEY_TYPE *poskey;
	int node_type;
	if(level==1){
		node_type=judge_node_type(node);
		if(node_type==1){
			fout<<"level:======"<<level<<"=========="<<endl;
			first_node=get_first_node(node);
			fout<<"nodeflag=";
			for(i=23;i>=0;i--)
				fout<<(int)get_nodeflag(first_node,i);
			fout<<endl;
			fout<<"cur_nodenum="<<(int)CUR_NODE_NUM(first_node)<<endl;
			fout<<"active_nodenum="<<(int)ACTIVE(first_node)<<endl;
			fout<<"max_overflow_node_num="<<max_overflow_node_num<<endl;
		    for(i=0;i<max_overflow_node_num;i++)
		    	fout<<"node_array["<<i<<"]="<<get_location(first_node,i)<<endl;
		    poskey=KEY_IN_HEAD(first_node, bit_bytes);
		    for(i=0;i<kv_num;i++)
		    	fout<<"poskey["<<i<<"]="<<poskey[i].keys<<endl;
		    fout<<"node_type="<<(int)judge_node_type(node)<<endl;
		    for(i=0;i<max_overflow_node_num;i++){
		    	first_node=get_first_node(node);
		    	tpid=get_location(first_node,i);
		    	if(tpid!=-1){
		    		cell=get_bcell_key(tpid);
		    		for(j=0;j<nativebtree<BCELL,KEY_TYPE>::KEY_NUM;j++){
		    			fout<<"key["<<j<<"]"<<"="<<cell->key[j].keys<<","<<"next["<<j<<"]"<<"="<<cell->next[j]<<endl;
		    		}
	    			fout<<"cur_num="<<cell->cur_knum<<endl;
	    			fout<<"node_type="<<(int)judge_node_type(tpid)<<endl;
		    	}
		    }
		}
		else if(node_type==0){
			fout<<"level:======"<<level<<"=========="<<endl;
			tpid=node;
			while(1){
				cell=get_bcell_key(tpid);
				for(i=0;i<nativebtree<BCELL,KEY_TYPE>::KEY_NUM;i++){
					fout<<"key["<<i<<"]"<<"="<<cell->key[i].keys<<","<<"next["<<i<<"]"<<"="<<cell->next[i]<<endl;
				}
				fout<<"cur_num="<<cell->cur_knum<<endl;
				fout<<"node_type="<<(int)judge_node_type(tpid)<<endl;
				if(cell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM]==-1)
					break;
				else
					tpid=cell->next[nativebtree<BCELL,KEY_TYPE>::KEY_NUM];
			}
		}

	}
	else if(level>1){
		fout<<"level:======"<<level<<"=========="<<endl;
		cell=get_bcell_key(node);
		for(i=0;i<nativebtree<BCELL,KEY_TYPE>::KEY_NUM;i++){
			fout<<"key["<<i<<"]"<<"="<<cell->key[i].keys<<","<<"next["<<i<<"]"<<"="<<cell->next[i]<<endl;
		}
		fout<<"next["<<i<<"]:"<<cell->next[i]<<endl;
		fout<<"cur_num="<<cell->cur_knum<<endl;
		num=nativebtree<BCELL,KEY_TYPE>::getKeyNum(node);
		for(i=0;i<=num;i++){
			cell =get_bcell_key(node);
			if(cell->next[i]!=NONTHING){
				printtree(cell->next[i],level-1);
			}
		}
	}
}
template <typename BCELL, typename KEY_TYPE>
void BloomFilter_Btree<BCELL,KEY_TYPE>::test(){
	LID first_pid=apply_first_node();
	LeafHeadb *first_node=get_first_node(first_pid);
    int i;
    for(i=1;i<=max_overflow_node_num;i++)
    	set_location(first_node, i-1, 10*i);
    cout<<"max_overflow_node_num="<<max_overflow_node_num<<endl;
    cout<<"bit_bytes="<<bit_bytes<<endl;
    for(i=0;i<max_overflow_node_num;i++)
    	cout<<"node_array["<<i<<"]="<<get_location(first_node,i)<<endl;
    for(i=0;i<24;){
    	set_nodeflag(first_node,i, 1);
    	i=i+2;
    }
    for(i=0;i<24;i++)
        cout<<"nodeflag["<<i<<"]="<<get_nodeflag(first_node,i)<<endl;
    for(i=0;i<24;){
    	set_nodeflag(first_node,i, 0);
    	i=i+3;
    }

    for(i=0;i<24;i++)
        cout<<"nodeflag["<<i<<"]="<<get_nodeflag(first_node,i)<<endl;

    set_cur_nodenum(first_node,8);
    cout<<"cur_nodenum="<<(int)CUR_NODE_NUM(first_node)<<endl;

    set_cur_nodenum(first_node,5);
    cout<<"cur_nodenum="<<(int)CUR_NODE_NUM(first_node)<<endl;

    set_active(first_node,3);
    cout<<"active_nodenum="<<(int)ACTIVE(first_node)<<endl;

    set_active(first_node,4);
    cout<<"active_nodenum="<<(int)ACTIVE(first_node)<<endl;


    cout<<"key_num="<<kv_num<<endl;

    int newactive=get_new_active(first_node);
    cout<<"new active="<<newactive<<",active_nodenum="<<(int)ACTIVE(first_node)<<"location="<<get_location(first_node,newactive)<<endl;

    cout<<"cur_nodenum="<<(int)CUR_NODE_NUM(first_node)<<endl;
    for(i=0;i<24;i++)
        cout<<"nodeflag["<<i<<"]="<<get_nodeflag(first_node,i)<<endl;
}
#endif /* BLOOMF_BTREE_H_ */
