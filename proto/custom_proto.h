/*
 * customproto.h
 *
 *  Created on: 26 июл. 2019 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_PROTO_CUSTOM_PROTO_H_
#define SRC_ENGINE_PROTO_CUSTOM_PROTO_H_

#include "engine/algo/crc.h"
#include "engine/basic.h"


#define CUSTOM_PROTO_MAX_BUF 100000

class CustomProto_id1size1checksum1type1data250crc1_a_T{
	public:
	CustomProto_id1size1checksum1type1data250crc1_a_T(){
		header=0xaa;
		maxlen=251;
	}
	CustomProto_id1size1checksum1type1data250crc1_a_T(u8 header,u32 maxlen,u8 header_size){
		this->header=header;
		this->maxlen=maxlen;
	}
	u32 header=0xaa;
	u32 maxlen=250;
	u32 i=0;
	u32 buf_offset=0;
	u32 state=0;
	u32 size=0;
	u8 type=0;
	u8 data_pointer=0;


	u8 get_from_buf(u8 & cmd,u8 * data,u8 & data_len,u8 * buf,u16 bufsize,eErrorTp & err){
		u8 ch;
		//u8 data_len=0;
		u8 crc8;
		err=ERROR;
		for(u16 n=0;n<bufsize;n++)
		{
				ch=buf[n];
				//printf("ch %02x state %d\n",(u8)ch,state);

				if ((ch==header)&&(state==0))
				{
					state=1;

				}
				else{
				if (state==5){
					//crc
					CalcCRC8_ShotInit(crc8,type);
					//CalcCRC8_Shot(&type,crc8,1);
					if (data_len!=0)
						CalcCRC8_Shot(data,crc8,data_len);
					state=0;
					size=0;
					data_pointer=0;
					//printf("crc ch %02x %02x data_len %d\n",ch,crc8,data_len);
					if (ch==crc8){
						err=NO_ERROR;
						cmd=type;
						return n;
					}
					else
						printf("Error CRC8 cmd 0x%02x\n",type);

				}
				if (state==4){

					data[data_pointer]=ch;
					data_pointer++;

					if (data_pointer==data_len)
						state=5;
				}
				if (state==3){
					//type
					type=ch;
					if (data_len==0)
						state=5;
					else
						state=4;
				}
				if (state==2){
					if ((header^size)==ch){
						state=3;
					}
					else
					{
						//ERROR
						state=0;
						size=0;
						data_pointer=0;
					}
				}
				if (state==1){
					size=ch;
					data_len=size-5;
					state=2;
				}
				}

		}

		return bufsize;
	}
	u8 set_to_buf(u8 cmd,u8 * data,u8 datalen,u8 * buf){
		buf[0]=header;
		buf[1]=datalen+5;
		buf[2]=buf[0]^buf[1];
		buf[3]=cmd;
		memcpy(&buf[4],data,datalen);
		u8 crc8;
		CalcCRC8_ShotInit(crc8,cmd);
		//CalcCRC8_Shot(&cmd,crc8,1);
		if (datalen>0)
			CalcCRC8_Shot(data,crc8,datalen);
		buf[datalen+4]=crc8;
		return datalen+5;
	}
};

class CustomProto_addr1reg1val1_T{
	//Протокол только для отправки данных
	//Пакет отправки фикс. размера 3 байта  [адрес, регистр, значение]
	//Пакет подтверждения [содержимое пакета отправки, два байта Ок]
	public:

	u8 cntr=0;
	eErrorTp get_from_buf(u8 * wait_seq,u8 * rbuf,u16 rbuflen){

		for (u8 n=0;n<rbuflen;n++){
			//for (u8 i=0;i<5;i++){
				if (wait_seq[cntr]==rbuf[n]){
					cntr++;
				}
				else
					cntr=0;
				if (cntr==5)
				{
					cntr=0;
					return NO_ERROR;
				}
		}

		return ERROR;
	}

	u32 set_to_buf(u8 cmd,u8 addr,u8 val){
		return cmd|(addr<<8)|(val<<16);
	}
};

class CustomProtoT{
	public:
	CustomProto_id1size1checksum1type1data250crc1_a_T id1size1checksum1type1data250crc1_a;
	CustomProto_addr1reg1val1_T addr1reg1val1;
	u8 tbuf[CUSTOM_PROTO_MAX_BUF];
};


#endif /* SRC_ENGINE_PROTO_CUSTOM_PROTO_H_ */
