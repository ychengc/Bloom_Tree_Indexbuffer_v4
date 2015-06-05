#ifndef _BLOOM_H_
#define _BLOOM_H_

#include "MurmurHash3.h"
typedef unsigned long long UINT64_RC;

class bloom
{
	private:
		//data
		const int key_max_num;
		int hash_num;

		uint32_t bit_array_len;
		int bit_array_bytes;
		
		const double error_std;
		double error_real_std;

		uint32_t get_count;
		uint32_t set_count;
		uint32_t get_miss_count;

		//function
		void bit_set(int* bit_array, UINT64_RC hash);
		int bit_check(int* bit_array, UINT64_RC hash);
		
	public:
		bloom(int key_num, double err_in);
		void bloom_init(int* bit_array);
		int bloom_set(int* bit_array, char* key, int key_len);
		int bloom_get(int* bit_array, char* key, int key_len);
		double cur_error_get(int cur_num);
		int get_bit_bytes();
		void print_bloom();
};
#endif
