#include "global.h"

int GLOBAL_TIME = 0;
int _sum = 0;
int _split_num = 0;
int _split_intree_num = 0;
int _split_leaf_num = 0;
int _merge_num = 0;
int _merge_intree_num = 0;
int _merge_leaf_num = 0;
int _ssd_read_num = 0;
int _ssd_write_num = 0;
int _hdd_read_num = 0;
int _hdd_write_num = 0;
int _insert_num = 0;
int _search_num = 0;
int _range_search_num=0;
int _update_num = 0;
int _delete_num = 0;
time_t runTime = 0;
time_t io_time = 0;
unsigned int bloom_search_count=0;
unsigned int bloom_error_count=0;
int _page_number = 0;
int _debug_flag = 0;
int *_debug_test = NULL;
int flush_count=0;
int search_tree_count=0;
int hit_write_in_bpb=0;
//int no_hit_write_in_bpb=0;