/*
 * adapter.cxx
 *
 *  Created on: 15 мар. 2019 г.
 *      Author: root
 */


#include "engine/periphery/adapter.h"

SerialAdapterT::SerialAdapterT(string type,u32 timeout_ms,eDebugTp llevel){
	loglevel=llevel;

	if (timeout_ms==0){
		mode|=O_NONBLOCK ;
	}
	else
		mode|=O_ASYNC;

	this->timeout_ms=timeout_ms;
	if (type=="uart"){
		enSerType=UART;
		//printf("type==uart\n");
	}
	if (type=="rs485"){
		enSerType=RS485;
	}
}

eErrorTp SerialAdapterT::ConfigureUART(string & tty, u32 speed,u8 stopbit, int exflags){

		//printf("Configure enSerType=%d\n",enSerType);
		if (enSerType==UART){
			//printf("enSerType==UART\n");
			usart=make_shared<TTYclass>(tty,sname,"uart",speed,stopbit,exflags,mode,loglevel);

			if (timeout_ms==0){
				Writebuf=std::bind(&SerialAdapterT::WritebufUART, this, std::placeholders::_1,std::placeholders::_2);
				Readbuf=std::bind(&SerialAdapterT::ReadbufUART, this, std::placeholders::_1,std::placeholders::_2);
			}
			else{
				Writebuf=std::bind(&SerialAdapterT::WritebufUART_blocked, this, std::placeholders::_1,std::placeholders::_2);
				Readbuf=std::bind(&SerialAdapterT::ReadbufUART_blocked, this, std::placeholders::_1,std::placeholders::_2);
			}

			Clearbuf=std::bind(&SerialAdapterT::ClearbufUART, this);
			return usart->glbERROR;

			//Writebuf=WritebufUART;
		}
		else
			return ERROR;
		//string & tty, string & parens, char * name, u32 speed, u8 stopbit, int exflags,
		return NO_ERROR;
	}

	//for RS485,RS422
eErrorTp SerialAdapterT::ConfigureRS485_Auto_CTSRTS(string tty, u32 speed,u8 stopbit,int exflags){
		//rs485_manual

		if (enSerType==RS485){
			//printf("enSerType==UART\n");
//CRTSCTS
			usart=make_shared<TTYclass>(tty,sname,"uart",speed,stopbit,exflags,mode,loglevel);
			usart->ConfRS485();
			Writebuf=std::bind(&SerialAdapterT::WritebufUART, this, std::placeholders::_1,std::placeholders::_2);
			if (timeout_ms==0)
				Readbuf=std::bind(&SerialAdapterT::ReadbufUART, this, std::placeholders::_1,std::placeholders::_2);
			else
				Readbuf=std::bind(&SerialAdapterT::ReadbufUART_blocked, this, std::placeholders::_1,std::placeholders::_2);
			Clearbuf=std::bind(&SerialAdapterT::ClearbufUART, this);
				//
		}
		else
			return ERROR;
		//string & tty, string & parens, char * name, u32 speed, u8 stopbit, int exflags,
		return NO_ERROR;
	}

eErrorTp SerialAdapterT::ConfigureRS485_Manual_CTSRTS(string tty, u32 speed,u8 stopbit,int exflags,u32 gp,bool gp_inv){
		//rs485_manual

		if (enSerType==RS485){
			//printf("enSerType==UART\n");
			onebyte_time=((1000000 * (1 + 8  + stopbit)) / speed);
			usart=make_shared<TTYclass>(tty,sname,"uart",speed,stopbit,exflags,mode,loglevel);
			//string num,string direct,string value

			if (gp_inv){
				dirRS485_in="1";
				dirRS485_out="0";
			}

			gpRS485=make_shared<gpio>(IntToStr(gp),"out",(char*)dirRS485_in.c_str());
			Writebuf=std::bind(&SerialAdapterT::WritebufRS485_manual, this, std::placeholders::_1,std::placeholders::_2);
			Readbuf=std::bind(&SerialAdapterT::ReadbufUART, this, std::placeholders::_1,std::placeholders::_2);
			Clearbuf=std::bind(&SerialAdapterT::ClearbufUART, this);
		}
		else
			return ERROR;
		//string & tty, string & parens, char * name, u32 speed, u8 stopbit, int exflags,
		return NO_ERROR;
	}

	//for I2C/SPI
eErrorTp SerialAdapterT::Configure(u8 id,u8 cs_addr, u32 speed){
			//string & tty, string & parens, char * name, u32 speed, u8 stopbit, int exflags,
			return NO_ERROR;
		}

	//{

	//}
eErrorTp SerialAdapterT::WritebufUART(u8 * buf,u32 len){
		return	usart->TTYwriteBuf(buf,len);
	}

eErrorTp SerialAdapterT::WritebufUART_blocked(u8 * buf,u32 len){
	//	printhex(buf,len,16);
		return	usart->TTYwriteBufBlocked(buf,len);
	}


eErrorTp SerialAdapterT::ClearbufUART(void){
		return	usart->TTYClear();
	}

eErrorTp SerialAdapterT::WritebufRS485_manual(u8 * buf,u32 len){
		eErrorTp err;
		gpRS485->set((char*)dirRS485_out.c_str());
		//printf("Set out %s\n",dirRS485_out.c_str());
		err=usart->TTYwriteBufBlocked(buf,len);
	//	usleep(onebyte_time*(len));
	//	usleep(5);
		//mdelay(100);
		gpRS485->set((char*)dirRS485_in.c_str());
		//printf("Set in %s\n",dirRS485_in.c_str());
		return err;
	}

u32 SerialAdapterT::ReadbufUART(u8 * buf,u32 maxlen){
				return usart->TTYnreadBuff(buf,maxlen);
		}

u32 SerialAdapterT::ReadbufUART_blocked(u8 * buf,u32 maxlen){

				return usart->TTYnreadBuffBlocked(buf,maxlen,timeout_ms);
		}
