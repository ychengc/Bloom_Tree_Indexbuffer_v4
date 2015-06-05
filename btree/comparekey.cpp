#include "btreestruct.h"

int compare_key(const int *self, const int *other, int keynum){
	int i;
	for(i=0;i<keynum;i++){
		if(self[i]<other[i]||other[i]==INFINITY)
			return -1;
		else{
			if(self[i]>other[i]||self[i]==INFINITY)
				return 1;
		}
	}
	return 0;
}
