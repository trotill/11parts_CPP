/*
 * adapter.h
 *
 *  Created on: 17 янв. 2019 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_PERIPHERY_ADAPTER_H_
#define SRC_ENGINE_PERIPHERY_ADAPTER_H_
#include "engine/print.h"
#include "engine/periphery/tty.h"
#include "engine/periphery/gpio.h"

class SerialAdapterT{
	public:
	SerialAdapterT(string type,u32 timeout_ms,eDebugTp llevel);
	typedef std::function <eErrorTp (u8 *,u32)> Writebuf_TDef;
	typedef std::function <u32 (u8 *,u32)> Readbuf_TDef;
	typedef std::function <eErrorTp (void)> Clear_TDef;

	//Writebuf=WritebufUART;
	//for USART
	Writebuf_TDef Writebuf;
	Readbuf_TDef Readbuf;
	Clear_TDef Clearbuf;
	eErrorTp ConfigureUART(string & tty, u32 speed,u8 stopbit, int exflags);

	//for RS485,RS422
	eErrorTp ConfigureRS485_Auto_CTSRTS(string tty, u32 speed,u8 stopbit,int exflags);
	eErrorTp ConfigureRS485_Manual_CTSRTS(string tty, u32 speed,u8 stopbit,int exflags,u32 gp,bool gp_inv);



	//	tty->TTYwriteBuf((char*)em->RecvBuf.buf_base,em->RecvBuf.base_len);
	//  count=tty->TTYnreadBuff(buf,sizeof(buf))
	private:
	eErrorTp Configure(u8 id,u8 cs_addr, u32 speed);
	eErrorTp WritebufUART(u8 * buf,u32 len);
	eErrorTp WritebufUART_blocked(u8 * buf,u32 len);
	eErrorTp WritebufRS485_manual(u8 * buf,u32 len);
	u32 ReadbufUART(u8 * buf,u32 maxlen);
	u32 ReadbufUART_blocked(u8 * buf,u32 maxlen);
	eErrorTp ClearbufUART(void);
		enum {UART,RS485,RS422,SPI,I2C} enSerType;
		string sname="sadapt";
		eDebugTp loglevel;
		bool rs485_manual=false;
		shared_ptr<TTYclass> usart;
		shared_ptr<gpio> gpRS485;
		u32 onebyte_time=0;
		string dirRS485_in="0";
		string dirRS485_out="1";
		u32 timeout_ms=0;
		u32 mode=O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL;
};


#endif /* SRC_ENGINE_PERIPHERY_ADAPTER_H_ */
