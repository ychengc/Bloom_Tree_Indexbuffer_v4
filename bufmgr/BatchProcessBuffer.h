#ifndef BATCHPROCESSBUFFER_H_
#define BATCHPROCESSBUFFER_H_

#include "bufv.h"
#include "../btree/btreestruct.h"
#include <iostream>
#include <string.h>
using namespace std;

template <typename KEY_TYPE>
class taskentry{
public:
	KEY_TYPE key;
	VALUE_TYPE next;
	char flag;
};

template <typename KEY_TYPE>
class bpb{
	public:
	bpb(int bufsz, int page_size);
	int insert_te(taskentry<KEY_TYPE> te);
	int delete_te(int position);
	int delete_te_batch(taskentry<KEY_TYPE> *entrybuff,int ecount);
	int IsBPBFull();
	int findvalue(KEY_TYPE minvalue,KEY_TYPE maxvalue,int &startpos,int &endpos);
	int find_minvalue(KEY_TYPE minvalue,int startpos,int endpos);
	int find_maxvalue(KEY_TYPE maxvalue,int startpos,int endpos);
	VALUE_TYPE searchkey(KEY_TYPE key);
	int binary_search_pos(taskentry<KEY_TYPE> te);  //find the position which equal the target task entry
	void clear();
	void print();
	~bpb();
//	protected:
	int bufsize;
	BYTE * buffer;
	int pagesize;
	int curnum;
	int maxnum;
};

template <typename KEY_TYPE>
bpb<KEY_TYPE>::bpb(int bufsz, int page_size){
	bufsize=bufsz;
	pagesize=page_size;
	if(iMALLOC((void**)&buffer, pagesize, pagesize*bufsize)){
		cout<<"Exception--->iMALLOC fail in nonbuf()"<<endl;
		exit(0);
	}
	int sz=sizeof(taskentry<KEY_TYPE>);
	maxnum=(bufsize*PAGESIZE)/sz;
	curnum=0;
}

template <typename KEY_TYPE>   //delete the positionth taskentry
int bpb<KEY_TYPE>::delete_te(int position){
	BYTE *b,*b2;
	int i;
	taskentry<KEY_TYPE> *temp,*temp2;
    if((curnum>1)&&(position<curnum)){
    	for(i=position;i<curnum;i++){
    		b=buffer+(i-1)*sizeof(taskentry<KEY_TYPE>);
    		b2=b+sizeof(taskentry<KEY_TYPE>);
    		temp=(taskentry<KEY_TYPE> *)b;
    		temp2=(taskentry<KEY_TYPE> *)b2;
    		*temp=*temp2;
    	}
    }
	curnum--;
	return 1;
}

template <typename KEY_TYPE>
int  bpb<KEY_TYPE>::delete_te_batch(taskentry<KEY_TYPE> *entrybuff,int ecount){
	int   deletepos=entrybuff-(taskentry<KEY_TYPE> *)(this->buffer)+1;
	int deletepos2=((BYTE*)(entrybuff)-this->buffer)/sizeof(taskentry<KEY_TYPE>)+1;
	if(deletepos!= deletepos2){
		cout<<"error-->in delete_te_batch--deletepos"<<endl;
		exit(0);
	}
	if((deletepos+ecount-1)>curnum){
		cout<<"error-->in delete_te_batch"<<endl;
		exit(0);
	}
	BYTE *b,*b2;
    b=(BYTE*)(entrybuff);
    b2=b+ecount*sizeof(taskentry<KEY_TYPE>);
    int move_byte=(curnum-(deletepos-1+ecount))*sizeof(taskentry<KEY_TYPE>);
    memmove(b,b2,move_byte);
    curnum=curnum-ecount;
    return 1;
}

template <typename KEY_TYPE>
int bpb<KEY_TYPE>::insert_te(taskentry<KEY_TYPE> te){
	if(curnum>=maxnum){
		cout<<"error--> in insert_te"<<endl;
		exit(0);
	}
	int i,pos;
	BYTE *b,*b2;
	taskentry<KEY_TYPE> *temp;
	pos=binary_search_pos(te)+1;
	b=buffer+(pos-1)*sizeof(taskentry<KEY_TYPE>);
	temp=(taskentry<KEY_TYPE> *)b;
	if( (pos>curnum)||(temp->key>te.key) ){
		b2=b+sizeof(taskentry<KEY_TYPE>);
		int move_byte=(curnum-pos+1)*sizeof(taskentry<KEY_TYPE>);
		if(move_byte>0)
		   memmove(b2,b,move_byte);
		b=buffer+(pos-1)*sizeof(taskentry<KEY_TYPE>);
		temp=(taskentry<KEY_TYPE> *)b;
		temp->key=te.key;
		temp->next=te.next;
		temp->flag=te.flag;
		curnum++;
		return 1;
	}
	else if(temp->key==te.key){
		if((temp->flag=='I')&&(te.flag=='I')){
               temp->next=te.next;
		}
		else if((temp->flag=='I')&&(te.flag=='D')){
			   temp->flag='D';
		}
		else if((temp->flag=='D')&&(te.flag=='I')){
			   temp->next=te.next;
			   temp->flag='I';
		}
		else if((temp->flag=='D')&&(te.flag=='D')){

		}
		hit_write_in_bpb++;
		return 0;
	}
	else{
		cout<<"error-->in insert_te"<<endl;
		exit(0);
	}
}

template <typename KEY_TYPE>
int bpb<KEY_TYPE>::IsBPBFull(){
	if(curnum<maxnum)
		return 0;
	else if(curnum==maxnum)
		return 1;
	else{
		cout<<"BPB error"<<endl;
		exit(0);
	}
}

template <typename KEY_TYPE> //return the min position where its value>=minvalue
int bpb<KEY_TYPE>::find_minvalue(KEY_TYPE minvalue,int startpos,int endpos){
	int low=startpos-1;
	int high=endpos-1;
	int middle=0;
	taskentry<KEY_TYPE> *temp;
	BYTE *b;
	b=buffer+low*sizeof(taskentry<KEY_TYPE>);
	temp=(taskentry<KEY_TYPE> *)b;
	if(temp->key>=minvalue)
		return startpos;
	b=buffer+high*sizeof(taskentry<KEY_TYPE>);
	temp=(taskentry<KEY_TYPE> *)b;
	if(temp->key<minvalue)
		return -1;
	//===================binary search for the min position
	while(low<=high){
		middle = (low + high)/2;
		temp=(taskentry<KEY_TYPE> *)(buffer+middle*sizeof(taskentry<KEY_TYPE>));
		if(temp->key==minvalue)
			return (middle+1);
		else if(temp->key>minvalue)
			high=middle-1;
		else
			low=middle+1;
	}
	return (low+1);
}

template <typename KEY_TYPE> //return the (max position+1) where its value<maxvalue
int bpb<KEY_TYPE>::find_maxvalue(KEY_TYPE maxvalue,int startpos,int endpos){
	int low=startpos-1;
	int high=endpos-1;
	int middle=0;
	taskentry<KEY_TYPE> *temp;
	BYTE *b;
	b=buffer+low*sizeof(taskentry<KEY_TYPE>);
	temp=(taskentry<KEY_TYPE> *)b;
	if(temp->key>=maxvalue)
		return startpos;
	b=buffer+high*sizeof(taskentry<KEY_TYPE>);
	temp=(taskentry<KEY_TYPE> *)b;
	if(temp->key<maxvalue)
		return endpos+1;
	//===================binary search for the max position
	while(low<=high){
		middle = (low + high)/2;
		temp=(taskentry<KEY_TYPE> *)(buffer+middle*sizeof(taskentry<KEY_TYPE>));
		if(temp->key==maxvalue)
			return (middle+1);
		else if(temp->key>maxvalue)
			high=middle-1;
		else
			low=middle+1;
	}
	return (low+1);
}

template <typename KEY_TYPE>
int bpb<KEY_TYPE>::findvalue(KEY_TYPE minvalue,KEY_TYPE maxvalue,int &startpos,int &endpos)
{
	int count=0;
	int pos=startpos;
	 int stratpos_t;
    if(minvalue==Infinity&&maxvalue!=Infinity){
        pos=find_maxvalue(maxvalue,startpos,endpos);
        endpos=pos-1;
        count=endpos-startpos+1;
    }
    else if(maxvalue==Infinity){
    	count=endpos-startpos+1;
    }
    else if(minvalue!=Infinity&&maxvalue!=Infinity){
            stratpos_t=find_minvalue( minvalue,startpos,endpos);
             pos=find_maxvalue(maxvalue,startpos,endpos);
             endpos=pos-1;
             count=endpos-startpos+1;
    }
    return count;
}
/*
template <typename KEY_TYPE>
int bpb<KEY_TYPE>::findvalue(KEY_TYPE minvalue,KEY_TYPE maxvalue,int &startpos,int &endpos)
{
	taskentry<KEY_TYPE> *temp;
	BYTE *b1;
	int count=0;
	int pos=startpos;
	b1=buffer+(pos-1)*sizeof(taskentry<KEY_TYPE>);
	temp=(taskentry<KEY_TYPE> *)b1;
    if(minvalue==Infinity&&maxvalue!=Infinity){
        while( (temp->key<maxvalue)&&(pos<=endpos) ){
        	count++;
        	pos++;
        	temp=temp+1;
        }
        endpos=pos-1;
    }
    else if(maxvalue==Infinity){
    	count=endpos-startpos+1;
    }
    else if(minvalue!=Infinity&&maxvalue!=Infinity){
        while((temp->key>=minvalue)&&(temp->key<maxvalue)&&(pos<=endpos)){
        	count++;
        	pos++;
        	temp=temp+1;
        }
    	endpos=pos-1;
    }
    return count;
}*/

template <typename KEY_TYPE>
bpb<KEY_TYPE>::~bpb(){
	free(buffer);
}

template <typename KEY_TYPE>
VALUE_TYPE bpb<KEY_TYPE>::searchkey(KEY_TYPE key){
	int low=0;
	int high=curnum-1;
	int middle=0;
	taskentry<KEY_TYPE> * temp;
	while(low<=high){
		middle=(low+high)/2;
		temp=(taskentry<KEY_TYPE> *)(this->buffer+middle*sizeof(taskentry<KEY_TYPE>));
		if(temp->key==key)
			return temp->next;
		else if(temp->key>key)
			high=middle-1;
		else
			low=middle+1;
	}
	return Infinity ;

}

template <typename KEY_TYPE>
int bpb<KEY_TYPE>::binary_search_pos(taskentry<KEY_TYPE> te){
	int low=0;
	int high=curnum-1;
	int middle=0;
	taskentry<KEY_TYPE> * temp;
	while(low<=high){
		middle=(low+high)/2;
		temp=(taskentry<KEY_TYPE> *)(this->buffer+middle*sizeof(taskentry<KEY_TYPE>));
		if(temp->key==te.key)
			return middle;
		else if(temp->key>te.key)
			high=middle-1;
		else
			low=middle+1;
	}
	if(low>curnum-1){
		if(low!=curnum){
			cout<<"error-->in binary_search_pos--low>curnum-1"<<endl;
			exit(0);
		}
	}
	else{
		temp=(taskentry<KEY_TYPE> *)(this->buffer+low*sizeof(taskentry<KEY_TYPE>));
		if(temp->key<=te.key){
			cout<<"error-->in binary_search_pos--searchpos"<<endl;
			exit(0);
		}
	}
	return low;
}

template <typename KEY_TYPE>
void bpb<KEY_TYPE>::clear(){
	curnum=0;
	memset(buffer,0,pagesize*bufsize);
}

template <typename KEY_TYPE>
void bpb<KEY_TYPE>::print(){
	ofstream fout("/home/ycc/workspace/bexdata/bpbout",ios::app);
	int i;
	taskentry<KEY_TYPE> * temp;
	BYTE *b;
	for(i=1;i<=curnum;i++){
		b=buffer+(i-1)*sizeof(taskentry<KEY_TYPE>);
		temp=(taskentry<KEY_TYPE> *)b;
		fout<<"key:"<<temp->key<<",value:"<<temp->next<<",flag:"<<temp->flag<<endl;
	}
}

#endif /* BATCHPROCESSBUFFER_H_ */
