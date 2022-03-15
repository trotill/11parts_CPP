/*
 * 11p_bin.cxx
 *
 *  Created on: 27 дек. 2018 г.
 *      Author: root
 */



#include "11p_bin.h"

void GenMemBUG(){
	 printf("aaa %s",0);
	 u32 dd=232321;
	 printf("aaa %s",dd);
}

void ChangeBit ( u8 nbit, u8 bit, u32 & val )
{
    if(bit)
    	val |= (1 << nbit);
    else
    	val &= ~(1 << nbit);

    return;
}

u8 GetBit ( u8 nbit, u32 val )
{
    return (val>>nbit)&1;
}

int Bits1CountCalc(u8 * value, u32 len)
{
 u32 i, s=0, m;

 for (m=0;m<len;m++)
 {
	 for(i=0; i<8; i++)
		 s += ((value[m]>>i)&1);
 }

 return s;
}

u32 bitCopy(u8 * dst_buf,u32 dst_offset_bit,u32 src_size_bit,u8 * src_buf,u32 src_offset){
	//printf("bitCopy src_offset %d dst_offset_bit %d src_size_bit %d\n",src_offset,dst_offset_bit,src_size_bit);

		u32 bufdestsize=0;
		u32 offsb;
		u32 doffs;
		u32 ddoffs;
		u32 dfg=(src_size_bit+dst_offset_bit)>>3;
		bufdestsize=dfg;
		if ((dfg<<3)!=(src_size_bit+dst_offset_bit))
			bufdestsize++;

		u32 n=0;
		for (u32 i=src_offset;i<(src_size_bit+src_offset);i++,n++){
			offsb=(i>>3);

			doffs=dst_offset_bit+n;
			ddoffs=doffs>>3;

			if ((src_buf[offsb]>>((i-(offsb<<3))))&1){

				//printf("set bit %d\n",soffs);
				dst_buf[ddoffs]|=1<<(doffs-(ddoffs<<3));
				//set 1
			}
			else
			{
				//printf("clr bit %d\n",soffs);
				dst_buf[ddoffs]&=~(1<<(doffs-(ddoffs<<3)));
				//set 0
			}
		}
		return  bufdestsize;
}

#if 0
//Битовое копирование. bufsrc копирует sizesrc_bit бит в bufdest по смещению offsetdest_bit
u32 bitCopy(u8 * dst_buf,u32 dst_offset_bit,u32 src_size_bit,u8 * src_buf){

		u32 bufdestsize=0;
		u32 offsb;
		u32 soffs;
		u32 doffs;
		u32 ddoffs;
		u32 dfg=(src_size_bit+dst_offset_bit)>>3;
		bufdestsize=dfg;
		if ((dfg<<3)!=(src_size_bit+dst_offset_bit))
			bufdestsize++;

		for (u32 i=0;i<src_size_bit;i++){
			offsb=i>>3;

			soffs=(offsb<<3)+(i-(offsb<<3));

			doffs=dst_offset_bit+soffs;
			ddoffs=doffs>>3;

			if ((src_buf[offsb]>>((i-(offsb<<3))))&1){

				//printf("set bit %d\n",soffs);
				dst_buf[ddoffs]|=1<<(doffs-(ddoffs<<3));
				//set 1
			}
			else
			{
				//printf("clr bit %d\n",soffs);
				dst_buf[ddoffs]&=~(1<<(doffs-(ddoffs<<3)));
				//set 0
			}
		}
		return  bufdestsize;
}
#endif


std::string bufferToString(char* buffer, int bufflen)
{
    std::string ret(buffer, bufflen);

    return ret;
}

void PowerOff(void)
{

	system("init 3");
}

double Round(double data,u8 precision){
	u32 k=0;
	k=pow(10,precision);
	return round((double)data*k)/k;
}
