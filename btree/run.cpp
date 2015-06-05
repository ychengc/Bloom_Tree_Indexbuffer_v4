#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <time.h>
#include <string>

#include "../io/iofdctl.h"
#include "../bufmgr/lrubuf.h"
#include "../bufmgr/nobuf.h"
#include "bloomf_btree.h"
#include "nativebtree.h"

const int Disk_Storage_Size =500000;
int _count = 0;
iofdctl * ssd = NULL;
typedef BloomFilter_Btree<bcell_key1, key1> bftree_1;
typedef BloomFilter_Btree<bcell_key2, key2> bftree_2;
typedef BloomFilter_Btree<bcell_key3, key3> bftree_3;
typedef BloomFilter_Btree<bcell_key4, key4> bftree_4;

typedef struct _bloomset
{
	bloom *bloomf1;
	bloom *bloomf2;
	bloom *bloomf3;
	bloom *bloomf4;
}bloomSet;

bloomSet * blooms = NULL;
#define bloomf_1 (blooms->bloomf1)
#define bloomf_2 (blooms->bloomf2)
#define bloomf_3 (blooms->bloomf3)
#define bloomf_4 (blooms->bloomf4)

typedef bpb<key1> bpb_1;
typedef bpb<key2> bpb_2;
typedef bpb<key3> bpb_3;
typedef bpb<key4> bpb_4;

const double bpb_rate = 0.1;
//const double rate=(double)((bpb_rate)/(1-bpb_rate));
double rate=(double)((bpb_rate)/((1-bpb_rate)*(double)11));

typedef struct _bpbset
{
	bpb_1 *bpb1;
	bpb_2 *bpb2;
	bpb_3 *bpb3;
	bpb_4 *bpb4;
}bpbSet;

bpbSet * bpbs = NULL;
#define bpb1 (bpbs->bpb1)
#define bpb2 (bpbs->bpb2)
#define bpb3 (bpbs->bpb3)
#define bpb4 (bpbs->bpb4)

typedef struct _indexSet
{  //索引的管理器
	int index_num;
	void * index_array[11];
	_indexSet()
	{
		int k;
		index_num = 0;
		for(k=0; k<11; k++)
			index_array[k] = NULL;
	}
}indexSet;

indexSet * indexinfo = NULL;
#define index_num (indexinfo->index_num)
#define index_array(x) (indexinfo->index_array[x])

//char ssd_path[]= "/home/ycc/data_testfile/";
//const char ssd_path[] = "/media/ssd/";
//const char ssd_name[] = "ssd_";

void newIndex(int indexid, bufmgr * buf, int pagesz, int key_size, double nerr, int ofnum)
{
/*	char * ssdpath = new char[sizeof(ssd_path)+sizeof(ssd_name)+4];
	sprintf(ssdpath, "%s%s%d", ssd_path, ssd_name, indexid);
	
	iofdctl * ssd = NULL;
	
	ssd = new iofdctl(ssdpath, Disk_Storage_Size, SSD_TYPE, pagesz, disk_num);
	disk_array(disk_num) = ssd;
	disk_num++;*/

	switch(indexid)
	{
		case 2:
		case 9:
		case 10:
			bpb1 = new bpb_1((int)(buf->get_bufsize()*rate), pagesz);
			break;
		case 6:
		case 3:
			bpb2 = new bpb_2((int)(buf->get_bufsize()*rate), pagesz);	break;
		case 7:
		case 4:
		case 0:
			bpb3 = new bpb_3((int)(buf->get_bufsize()*rate), pagesz);	break;
		case 5:
		case 8:
		case 1:
			bpb4 = new bpb_4((int)(buf->get_bufsize()*rate), pagesz);	break;
	}

	switch(key_size)
	{
		case 1:
			index_array(indexid) = (void *)(new bftree_1(buf, ssd, KEY1_NUM, bloomf_1, nerr, ofnum,bpb1));
			break;
		case 2:
			index_array(indexid) = (void *)(new bftree_2(buf, ssd, KEY2_NUM, bloomf_2, nerr, ofnum,bpb2));
			break;
		case 3:
			index_array(indexid) = (void *)(new bftree_3(buf, ssd, KEY3_NUM, bloomf_3, nerr, ofnum,bpb3));
			break;
		case 4:
			index_array(indexid) = (void *)(new bftree_4(buf, ssd, KEY4_NUM, bloomf_4, nerr, ofnum,bpb4));
			break;
		default:
			cout<<"The key size if outside of 1-4, it is error"<<endl;
			exit(0);
			break;
	}

	index_num++;
}

template<typename treeType, typename keyType>
void exeTree(treeType * tree, keyType key, keyType key2,int operate, int & value)
{
	time_t start; time_t finish;
	time(&start);
	switch(operate)
	{
		case 0:  //search
			tree->search_Key(key);
			break;
		case 1:  //insert
			tree->insert_key(key, value);
			break;
		case 2:  //update
			tree->updateKey(key, value);
			break;
		case 3:  //delete
			tree->delete_key(key);
			break;
		case 4: //range search
			tree->rangesearchKey(key,key2);
			break;
		default:
			break;
	}
	time(&finish);
	runTime += finish-start;
}
/*
void exeHybrid(int c_index, int operate, int & value, int * key, int key_size)
{
	hybtree_1 * hyb1 = NULL; hybtree_2 * hyb2 = NULL; hybtree_3 * hyb3 = NULL; hybtree_4 * hyb4 = NULL;
	key1 * ikey_1;	key2 * ikey_2;	key3 * ikey_3;	key4 * ikey_4;

	switch(key_size)
	{
		case 1:
			hyb1 = (hybtree_1*)index_array(c_index);
			ikey_1 = new key1(key); 
			exeTree(hyb1, *ikey_1, operate, value);
			delete ikey_1;
			break;
		case 2:
			hyb2 = (hybtree_2*)index_array(c_index);
			ikey_2 = new key2(key);
			exeTree(hyb2, *ikey_2, operate, value);
			delete ikey_2;
			break;
		case 3:
			hyb3 = (hybtree_3*)index_array(c_index);
			ikey_3 = new key3(key); 
			exeTree(hyb3, *ikey_3, operate, value);
			delete ikey_3;
			break;
		case 4:
			hyb4 = (hybtree_4*)index_array(c_index);
			ikey_4 = new key4(key);
			exeTree(hyb4, *ikey_4, operate, value);
			delete ikey_4;
			break;
		default:
			break;
	}
}
*/
void exeSingle(int c_index, int operate, int & value, int * key, int *keymax,int key_size)
{
	bftree_1 * bf1 = NULL; bftree_2 * bf2 = NULL; bftree_3 * bf3 = NULL; bftree_4 * bf4 = NULL; 
	key1 * ikey_1;	key2 * ikey_2;	key3 * ikey_3;	key4 * ikey_4;
	key1 * ikey_1_1;	key2 * ikey_2_2;	key3 * ikey_3_3;	key4 * ikey_4_4;
	switch(key_size)
	{
		case 1:
			bf1 = (bftree_1*)index_array(c_index);
			ikey_1 = new key1(key);   //RAM overflow without free
			ikey_1_1=new key1(keymax);
			exeTree(bf1, *ikey_1, *ikey_1_1,operate, value);
			delete ikey_1;
			delete ikey_1_1;
			break;
		case 2:
			bf2 = (bftree_2*)index_array(c_index);
			ikey_2 = new key2(key);
			ikey_2_2=new key2(keymax);
			exeTree(bf2, *ikey_2,*ikey_2_2, operate, value);
			delete ikey_2;
			delete ikey_2_2;
			break;
		case 3:
			bf3 = (bftree_3*)index_array(c_index);
			ikey_3 = new key3(key);
			ikey_3_3=new key3(keymax);
			exeTree(bf3, *ikey_3, *ikey_3_3,operate, value);
			delete ikey_3;
			delete ikey_3_3;
			break;
		case 4:
			bf4 = (bftree_4*)index_array(c_index);
			ikey_4 = new key4(key);
			ikey_4_4=new key4(keymax);
			exeTree(bf4, *ikey_4, *ikey_4_4,operate, value);
			delete ikey_4;
			delete ikey_4_4;
			break;
		default:
			break;
	}
}

void clear_index_buffer(){
	int i;
	bftree_1 * bf1 = NULL; bftree_2 * bf2 = NULL; bftree_3 * bf3 = NULL; bftree_4 * bf4 = NULL;
	for(i=0;i<11;i++){
		switch(i)
		{
			case 2:
			case 9:
			case 10:
				bf1 = (bftree_1*)index_array(i);
				if(bf1!=NULL)
				   bf1->clear_bpb();
				break;
			case 6:
			case 3:
				bf2 = (bftree_2*)index_array(i);
				if(bf2!=NULL)
				   bf2->clear_bpb();
				break;
			case 7:
			case 4:
			case 0:
				bf3 = (bftree_3*)index_array(i);
				if(bf3!=NULL)
				    bf3->clear_bpb();
				break;
			case 5:
			case 8:
			case 1:
				bf4 = (bftree_4*)index_array(i);
				if(bf4!=NULL)
				    bf4->clear_bpb();
				break;
		}
	}
}
/*
void blackFun(int yn)
{
	char * ssdpath = new char[sizeof(ssd_path)+sizeof(ssd_name)+4];
	int k;
	int page_size;
	
	if(yn==121 || yn==89)
	{
		cout<<"enter the page size(KB) ? --- ";
		cin>>page_size;
		page_size = page_size*1024;
		for(k=0; k<11; k++)
		{
			sprintf(ssdpath, "%s%s%d", ssd_path, ssd_name, k);
			//sprintf(hddpath, "%s%s%d", hdd_path, hdd_name, k);
			//new iofdctl(hddpath, Disk_Storage_Size, HDD_TYPE, page_size, k);
			//cout<<"Inital is finish, disk--->"<<"hdd-"<<k<<" is ready to work for you"<<endl;
			new iofdctl(ssdpath, Disk_Storage_Size, SSD_TYPE, page_size, k+1);
			cout<<"Inital is finish, disk--->"<<"ssd-"<<k<<" is ready to work for you"<<endl;
		}
	}
}*/
/*
void displayPagesOndisk(int flag)
{
	int k;
	cout<<"* There index num is"<<index_num<<endl;
	if(flag == 2)
	{
		for(k=0; k<disk_num; k++)
		{
			if(disk_array(k)->diskType() == SSD_TYPE)
				cout<<"SSD-ID"<<disk_array(k)->diskID()<<" page num is "<<disk_array(k)->getPageNum()<<endl;
			else
				cout<<"HDD-ID"<<disk_array(k)->diskID()<<" page num is "<<disk_array(k)->getPageNum()<<endl;
		}
	}
	else if(flag == 1)
	{
		for(k=0; k<disk_num; k++)
			cout<<"SSD-ID"<<disk_array(k)->diskID()<<" page num is "<<disk_array(k)->getPageNum()<<endl;
	}
	else
	{
		for(k=0; k<disk_num; k++)
			cout<<"HDD-ID"<<disk_array(k)->diskID()<<" page num is "<<disk_array(k)->getPageNum()<<endl;
	}
}
*/

void initCounter(bufmgr * buffer)
{

	runTime = 0;
	io_time = 0;
	_split_num = 0;
	_split_intree_num = 0;
	_split_leaf_num = 0;
	_merge_num = 0;
	_merge_intree_num = 0;
	_merge_leaf_num = 0;
	buffer->io_read_num = 0;
	buffer->io_write_num = 0;
	_ssd_read_num = 0;
	_ssd_write_num = 0;
	_hdd_read_num = 0;
	_hdd_write_num = 0;
	_insert_num = 0;
	_search_num = 0;
	_range_search_num=0;
	_update_num = 0;
	_delete_num = 0;
	_page_number = 0;
	bloom_search_count=0;
	bloom_error_count=0;
	flush_count=0;
	search_tree_count=0;
	hit_write_in_bpb=0;
}

char * _operate[5] = {"search", "insert", "update", "delete","range"};
char * _index[11] = {"pk_customer", "ndx_customer_name", "pk_warehouse", "pk_district", "pk_new_order", "pk_order_line", "pk_stock", "pk_oorder", "ndx_oorder_carrier", "pk_item", "history_pkey"};

int get_operate_id(char * oper)
{
	const int num = 5;
	int k;
	for(k=0; k<num; k++)
	{
		if(strcmp(_operate[k], oper) == 0)
			return k;
	}
	printf("Error--->get_operate_id\n");
	exit(0);
}

void exp_print(bufmgr* buffer)
{
	cout<<"* Run Time ---> "<<runTime<<" seconds"<<endl;
	cout<<"* I/O Time ---> "<<io_time<<" seconds"<<endl;
	cout<<"bloom_search_count----->"<<bloom_search_count<<endl;
	cout<<"bloom_error_count----->"<<bloom_error_count<<endl;
	cout<<"--------------Split---------------"<<endl;
	cout<<"* Split Num ---> "<<_split_num<<endl;
	cout<<"* Split Num Inside Tree ---> "<<_split_intree_num<<endl;
	cout<<"* Split Num on Leaf ---> "<<_split_leaf_num<<endl;
	cout<<"--------------Merge---------------"<<endl;
	cout<<"* Merge Num ---> "<<_merge_num<<endl;
	cout<<"* Merge Num Inside Tree ---> "<<_merge_intree_num<<endl;
	cout<<"* Merge Num on Leaf ---> "<<_merge_leaf_num<<endl;
	cout<<"--------------Read & Write----------------"<<endl;
	cout<<"* All Read  Num ---> "<<buffer->io_read_num<<endl;
	cout<<"* All Write Num ---> "<<buffer->io_write_num<<endl;
	cout<<"* SSD Read  Num ---> "<<_ssd_read_num<<endl;
	cout<<"* SSD Write Num ---> "<<_ssd_write_num<<endl;
	cout<<"* HDD Read  Num ---> "<<_hdd_read_num<<endl;
	cout<<"* HDD Write Num ---> "<<_hdd_write_num<<endl;
	cout<<"--------------Trace Operation-------------"<<endl;
	cout<<"* Insert Num ---> "<<_insert_num<<endl;
	cout<<"* Search Num ---> "<<_search_num<<endl;
	cout<<"* Range Search Num ---> "<<_range_search_num<<endl;
	cout<<"* Update Num ---> "<<_update_num<<endl;
	cout<<"* Delete Num ---> "<<_delete_num<<endl;
	cout<<"* Total Page Num ---> "<<_page_number<<endl;
	cout<<"* flush count ---> "<<flush_count<<endl;
	cout<<"* search_tree_count---> "<<search_tree_count<<endl;
	cout<<"* update operation hit in index buffer---> "<<hit_write_in_bpb<<endl;
}

int get_index_id(char * indexChar)
{
	const int num = 11;
	int k;
	for(k=0; k<num; k++)
	{
		if(strcmp(_index[k], indexChar) == 0)
			return k;
	}
	printf("Error--->get_index_id\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	//'search':0, 'insert':1, 'update':2, 'delete':3
	//'pk_customer':0, 'ndx_customer_name':1, 'pk_warehouse':2, 'pk_district':3,
	//'pk_new_order':4, 'pk_order_line':5, 'pk_stock':6, 'pk_oorder':7,
	//'ndx_oorder_carrier':8, 'pk_item':9

	char yn;
	int k;
	int c_operate;
	int c_index;
	int c_value;
	int * c_key;
	int *c_key2;
	int exp_type;
	//initialize
	int page_size;
	int pagenum_buf;

	diskinfo = new diskSet;
	indexinfo = new indexSet;
	bpbs = new bpbSet;
	blooms = new bloomSet;
	index_num = 0;
	disk_num = 0;
	for(k=0; k<11; k++)
		index_array(k) = NULL;

	int buf_flag;

	cout<<"trace is below:"<<endl;
	//char * tracefile = "/home/ycc/new_trace/xie-base-out-hash.txt";
	//char * tpccfile ="/home/ycc/new_trace/xie-trace-hash-v3.txt";

	//char * tracefile = "/home/ycc/new_trace/rawout-index-tpcc-ware10-create-hash.txt";
	//char * tpccfile ="/home/ycc/new_trace/rawout-tpcc-ware10-100_100-hash.txt";
	//char * tpccfile ="/home/ycc/new_trace/rawout-tpcc-ware50-100_1000-hash.txt";

	//char * tracefile = "/home/ycc/new_trace/range_query/create_1000w_5000w_delete_0.05";
	//char * tpccfile ="/home/ycc/new_trace/range_query/query_200w_gap_1000";
	//char * tracefile = "/home/ycc/new_trace/query_ratio/create_1000w";
	//char * tpccfile ="/home/ycc/new_trace/query_ratio/query_0.0";
	//char * tracefile = "/home/ycc/new_trace/scalability/500w/create";
	//char * tpccfile ="/home/ycc/new_trace/scalability/500w/query_0.5";
	string tracefile ("none");
	string tpccfile ("none");
	string ssd_path("/media/ssd/");
	string ssd_name("ssd_");
	if(argc==1){
		cout<<"error--> you should specify at least the trace file of index creation"<<endl;
		exit(0);
	}
	char parameter;
	while ((parameter = getopt(argc, argv, "d:c:m:n:")) != -1){
		switch (parameter){
		case 'd':
			ssd_path=string(optarg); break;
		case 'c':
			tracefile=string(optarg); break;
		case 'm':
			tpccfile=string(optarg); break;
		case 'n':
			rate=(rate*11)/atoi(optarg); break;
		}
	}
	cout<<"trace file of create index:"<<"\n"<<tracefile<<endl;
	cout<<"trace file of mixed operations:"<<"\n"<<tpccfile<<endl;
	cout<<"-----------------------------------------"<<endl;

	cout<<"Enter the exp type tpcc(1) or indexCreate(0)?"<<endl;
	cin>>exp_type;
	cout<<"enter the selection for your buffer: 1-no buf; 2-lru buf"<<endl;
	cin>>buf_flag;
	/*
	if(exp_type == -1)
	{
		cout<<"The Back Door? Are you sure(y/n)?"<<endl;
		cin>>yn;
		blackFun(yn);
		return 1;
	}*/
	
	if(buf_flag != 1 && buf_flag != 2)
	{
		cout<<"you enter an error number"<<endl;
		exit(0);
	}
	
	/*cout<<"enter the page size(KB) ? --- ";
	cin>>page_size;
	page_size = page_size*1024;*/
	page_size=4096;
	if(buf_flag != 1)
	{
		cout<<"enter the buffer size(page number) ? --- ";
		cin>>pagenum_buf;
	}
	else
		pagenum_buf = 1024;

	bufmgr * buffer = NULL;
	if(buf_flag == 1)
		buffer = (bufmgr *)(new nobuf(pagenum_buf*(1-bpb_rate), page_size));
	else
		buffer = (bufmgr *)(new lrubuf(pagenum_buf*(1-bpb_rate), page_size));

	double _nerr;
	int _ofnum;		
	double ERROR_RATE;

	cout<<"enter the bloom filter error---"; cin>>ERROR_RATE;
	cout<<"enter the node err--->"; cin>>_nerr;
	cout<<"enter the ofnum_tradeoff--->"; cin>>_ofnum;

	bloom bloomf1(KEY1_NUM,ERROR_RATE);
	bloomf1.print_bloom();
	bloom bloomf2(KEY2_NUM,ERROR_RATE);
	bloomf2.print_bloom();
	bloom bloomf3(KEY3_NUM,ERROR_RATE);
	bloomf3.print_bloom();
	bloom bloomf4(KEY4_NUM,ERROR_RATE);
	bloomf4.print_bloom();
	
	bloomf_1 = &bloomf1;
	bloomf_2 = &bloomf2;
	bloomf_3 = &bloomf3;
	bloomf_4 = &bloomf4;	

	//char * ssdpath = new char[sizeof(ssd_path)+sizeof(ssd_name)+4];
	//sprintf(ssdpath, "%s%s", ssd_path, ssd_name);
	string ssdpath=ssd_path+ssd_name;
	char * path = new char[ssdpath.length()+4];
	strcpy(path,ssdpath.c_str());
	ssd = new iofdctl(path, Disk_Storage_Size, SSD_TYPE, page_size, disk_num);
	disk_array(disk_num) = ssd;
//-----------------------------------------------------------------------

	//char * tracefile = "/home/yuan/workspace/All_Trace/index_trace/rawout-index-tpcc-ware10-create-trace.txt";
	//char * tpccfile = "/home/yuan/workspace/All_Trace/index_trace/rawout-tpcc-ware10-100_100.txt";

	char * _flag = "--->";
	char * ch_line[5];
	char * ch_keys[4];
	
	FILE * trace = fopen(tracefile.c_str(), "rt");
	//char * line = (char*)malloc(sizeof(char)*100);
	char line[100];
	char * ch_ptr;
	fseek(trace, 0, SEEK_SET);
	fgets(line, 100, trace);
	int key_size;
	
	int dot_sum = 0;
	cout<<"Experiment is start--->\nPlease Wait......"<<endl;
	while(!feof(trace))
	{
		_sum++;
		 
	//	if(_sum >= 2000000)
	//		break;
	//	cout<<"exe times---"<<_sum<<endl;
		
		if(_sum%200000 == 0)
			cout<<_flag<<flush;
		
		if(feof(trace))
			break;
		k = 0;
		ch_ptr = line;
		while((ch_line[k] = strtok(ch_ptr, "|"))!=NULL)
		{
			k++;
			ch_ptr = NULL;
		}
	
		c_operate = get_operate_id(ch_line[0]);  //get operate
		c_index = get_index_id(ch_line[1]);  //get index
		//get keys in trace 
		k=0; ch_ptr = ch_line[2];
		while((ch_keys[k] = strtok(ch_ptr, ","))!=NULL)
		{
			k++;
			ch_ptr = NULL;
		}
		key_size = k;
		c_key = (int*)malloc(sizeof(int)*k);
		c_key2=(int*)malloc(sizeof(int)*k);
		for(k=0; k<key_size; k++)
			sscanf(ch_keys[k], "%d", &c_key[k]);
		if(c_operate==4){ //get the max key of the key range
			k=0; ch_ptr = ch_line[3];
			while((ch_keys[k] = strtok(ch_ptr, ","))!=NULL)
			{
				k++;
				ch_ptr = NULL;
			}
			if(k!=key_size){
				cout<<"error--> range search doesn't have the equal type of minkey and maxkey"<<endl;
				exit(0);
			}
			for(k=0; k<key_size; k++)
				sscanf(ch_keys[k], "%d", &c_key2[k]);
		}
		//get value in trace
		if(c_operate<4)
		    sscanf(ch_line[3], "%d", &c_value);
		else
			sscanf(ch_line[4], "%d", &c_value);
		if(index_array(c_index) == NULL)
		{
			newIndex(c_index, buffer, page_size, key_size, _nerr, _ofnum);		
			if(c_index == 5)
				_count++;
		}
		
		switch(c_operate)
		{
			case 0:  //search
				_search_num++;
				break;
			case 1:  //insert
				_insert_num++;
				break;
			case 2:  //update
				_update_num++;
				break;
			case 3:  //delete
				_delete_num++;
				break;
			case 4: //range search
				_range_search_num++;
				break;
			default:
				break;
		}
    	//cout<<"sum=="<<_sum<<endl;
		_sum=_sum+0;
		exeSingle(c_index, c_operate, c_value, c_key, c_key2,key_size);

		free(c_key);
		free(c_key2);
		GLOBAL_TIME++; 
		if(buf_flag == 1)
			((nobuf*)buffer)->clearbuf();

		fgets(line, 100, trace);	
		if(feof(trace) && exp_type==1)
		{
			clear_index_buffer();
			cout<<"------------------------------------Experiment Result is Below----------------------------"<<endl;
			fclose(trace);
			cout<<"-----index create finish-Result is below----"<<endl;
			if(buf_flag != 1)
				buffer->clearbuf();
			exp_print(buffer);
			cout<<"-----below is tpcc query-----"<<endl;
			trace = fopen(tpccfile.c_str(), "rt");
			fseek(trace, 0, SEEK_SET);
			fgets(line, 100, trace);
			initCounter(buffer);
			exp_type = -1;
			_flag = "~~)";
		}
	}
	cout<<"\n"<<"Run Over, Result is below"<<endl;
	fclose(trace);
	//free(line);
	cout<<"-----tpcc query finish-Result is below----"<<endl;
	clear_index_buffer();
	exp_print(buffer);
	delete [] path;
	return 0;
}

