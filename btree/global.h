#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <iostream>
#include <time.h>
using namespace std;

extern int GLOBAL_TIME;// = 0;
typedef int DISK_TYPE;
const DISK_TYPE SSD_TYPE = 1;
const DISK_TYPE HDD_TYPE = -1;
const int Infinity = -30;

extern int _sum;
extern int _split_num;
extern int _split_intree_num;
extern int _split_leaf_num;
extern int _merge_num;
extern int _merge_intree_num;
extern int _merge_leaf_num;
extern int _ssd_read_num;
extern int _ssd_write_num;
extern int _hdd_read_num;
extern int _hdd_write_num;
extern int _insert_num;
extern int _search_num;
extern int _range_search_num;
extern int _update_num;
extern int _delete_num;
extern time_t runTime;
extern time_t io_time;
extern unsigned int bloom_search_count;
extern unsigned int bloom_error_count;
extern int _page_number;
extern int _debug_flag;
extern int *_debug_test;
extern int flush_count;
extern int search_tree_count;
extern int hit_write_in_bpb;
//extern int no_hit_write_in_bpb;
#endif
