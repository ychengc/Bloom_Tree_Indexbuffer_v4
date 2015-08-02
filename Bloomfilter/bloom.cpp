#include "bloom.h"
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
using namespace std;

#define SHIFT 5
#define MASK 0x1F
#define SALT_CONSTANT 0x97c29b3a

//---------------------public--------------------------
bloom::bloom(int key_num, double err_in):key_max_num(key_num),error_std(err_in)
{
	double err_t = 1;
	int x;
	int k = 0;
	for(x=2; err_t>error_std && x<100; x++)
	{
		k = ceil(x*log(2));
		err_t = pow((1 - exp((double)(-k)/x)), k);
	}
	
	if((double)(x-1)*key_max_num > (UINT64_RC)(-1))
	{
		cout<<"bloom bit-array length overflow check"<<endl;
		exit(0);
	}

	error_real_std = err_t;
	bit_array_len = (x-1)*key_max_num;
	hash_num = k;
	bit_array_bytes = bit_array_len/8 + (((bit_array_len & 7)>0)?1:0);
	
}

void bloom::print_bloom()
{
	cout<<"bloom filter calculator is init:"<<"\n";	
	cout<<"hash function num is "<<hash_num<<"\n";
	cout<<"error probability reall is "<<error_real_std<<"\n";
	cout<<"bit array length is "<<bit_array_len<<"\n";
	cout<<"bit array size(bytes) is "<<bit_array_bytes<<endl;
}

void bloom::bloom_init(int* bit_array)
{
	memset(bit_array, 0, bit_array_bytes);
}

int bloom::bloom_set(int* bit_array, char* key, int key_len)
{
	UINT64_RC checksum[2];
	MurmurHash3_x86_128(key, key_len, SALT_CONSTANT, checksum);
	UINT64_RC h1 = checksum[0];
	UINT64_RC h2 = checksum[1];
	UINT64_RC hashes;

	int i;
	for(i=0; i<hash_num; i++)
	{
		hashes = (h1 + i*h2) % bit_array_len;
		bit_set(bit_array, hashes);
	}
	set_count++;
	return 0;
}

int bloom::bloom_get(int* bit_array, char* key, int key_len)
{
	UINT64_RC checksum[2];
	MurmurHash3_x86_128(key, key_len, SALT_CONSTANT, checksum);
	UINT64_RC h1 = checksum[0];
	UINT64_RC h2 = checksum[1];
	UINT64_RC hashes;

	int i;
	for(i=0; i<hash_num; i++)
	{
		hashes = (h1 + h2*i) % bit_array_len;
		if(!bit_check(bit_array, hashes))
		{
			return 0;  //key is not in bloom
		}
	}
	return 1;  //key is in bloom
}

double bloom::cur_error_get(int cur_num)
{
	double t = (key_max_num-cur_num)/(double)(key_max_num);
	return ((1-t) * error_real_std + t);
}

int bloom::get_bit_bytes()
{
	return bit_array_bytes;
}

//--------------------private------------------------
void bloom::bit_set(int* bit_array, UINT64_RC hash)
{
	bit_array[hash>>SHIFT] |= (1 << (hash & MASK));
}

int bloom::bit_check(int* bit_array, UINT64_RC hash)
{
	return bit_array[hash>>SHIFT] & (1<<(hash & MASK));
}
