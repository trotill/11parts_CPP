/*
 * buffer.h
 *
 *  Created on: 12 апр. 2021 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_BUFFER_H_
#define SRVIOT_SRC_ENGINE_BUFFER_H_

#include "basic.h"

class buffer{
	public:

	buffer(){
		buf=NULL;
		max_size=0;
	}
	buffer(u32 size)noexcept{
		//printf("OPEN %d\n",size);
		buf=(u8*)calloc(1,size);
		if (buf!=NULL)
			max_size=size;
	}
	~buffer(){
		if (buf!=NULL)
			free(buf);
		//printf("CLOSE %d\n",max_size);
	}

	u8 * p(){
		return buf;
	}
	void clear(){
		memset(buf,0,max_size);
	}
	void create(u32 size){
			//printf("OPEN %d\n",size);

		if (buf!=NULL)
			free(buf);
		buf=(u8*)calloc(1,size);
		if (buf!=NULL)
			max_size=size;
	}
	//bitwise copying in new buffer
	shared_ptr<buffer> pbit(u32 bit_offset,u32 bit_len){
		u32 res_sz=(bit_len/8)+1;
		shared_ptr<buffer> res=make_shared<buffer>(res_sz);

		u32 offs=bit_offset/8;

		u32 byte_cnt=(bit_len/8);
		if ((bit_len<8)||(bit_len%8)){
			byte_cnt+=1;
		}
		u32 last_bits=bit_len%8;
		u8 * b=&buf[offs];
		//printf("offs=%d byte_cnt=%d bit_len=%d\n",offs,byte_cnt,bit_len);
		//printhex(b,16,16);
		u32 z=0;
		u32 zd=0;
		u32 k=0;
		u32 bo=bit_offset;
		if (bit_offset>=8)
			bo=bit_offset%8;
		for (u32 n=bo;n<(bit_len+bo);n++){
			u32 of=n;
			if (n>=8)
				of=n%8;

			if (n<8){
				res->p()[zd]|=((b[z]>>of)&1)<<k;
			}
			else{
				if (of==0)
					z++;
				res->p()[zd]|=((b[z]>>of)&1)<<k;
			}
			k++;
			if (k==8){
				k=0;
				zd++;
			}
		}
		return res;
	}

	u8 * p(u32 offset,u8 * data,u32 data_len){
		if ((offset+data_len)>max_size)
			return buf;

		memcpy(&buf[offset],data,data_len);
	//	printf("buf[%d]=%d\n",offset,buf[offset]);
		return buf;
	}
	u32 size(){
		return max_size;
	}
	private:
	u8 * buf=NULL;
	u32 max_size=0;
};



#endif /* SRVIOT_SRC_ENGINE_BUFFER_H_ */
