/*
 * modbuscpp.h
 *
 *  Created on: 14.11.2018
 *      Author: root
 */

#ifndef MODBUSCPP_H_
#define MODBUSCPP_H_

#include "engine/print.h"
#include <modbus/modbus.h>

#include "engine/periphery/gpio.h"
#include "engine/lib/11p_bin.h"

typedef enum {MBUS_REGISTER,MBUS_REGISTERS,MBUS_BITS,MBUS_BIT,MBUS_INPUT_BITS,MBUS_INPUT_REGISTER} tpModBusData;

typedef void (rts_sw_callback) (modbus_t * ctx, int on);

static gpio  * __gpio_RTS_MODBUS=NULL;
class modbuscpp_client {
	public:
	modbuscpp_client(eDebugTp debugLevel);
	modbuscpp_client();
	~modbuscpp_client(void);
	//
	eErrorTp InitMaster(string ttyport,u32 baudrate,char parity, int data_bit, int stop_bit);
	eErrorTp Write(u8 slave_addr,u32 addr,u32 size16,u16 * buf,tpModBusData type);
	//Read, для MBUS_BITS (Read Coil Status) и MBUS_INPUT_BITS (Read Discrete Inputs) один байт = 1бит, т.е. например для 10 бит, нужен буфер 10 байт
	//для остального 2 байта = 1 значение, т.е. например для 10 рег, нужен буфер 20 байт
	eErrorTp Read(u8 slave_addr,u32 addr,u32 countItem,u16 * dest,tpModBusData type,u8 try_count);
	//ReadHoldingRegs, один элемент = 2 байта
	eErrorTp ReadHoldingReg(u8 slave_addr,u32 addr,u32 countItem,buffer & buf,u8 try_count);
	//ReadInputRegs, один элемент = 2 байта
	eErrorTp ReadInputReg(u8 slave_addr,u32 addr,u32 countItem,buffer & buf,u8 try_count);
	eErrorTp ReadCoil(u8 slave_addr,u32 addr,u32 countItem,buffer & buf,u8 try_count);
	//ReadDiscreteInput, один элемент = 1 байт
	eErrorTp ReadDiscreteInput(u8 slave_addr,u32 addr,u32 countItem,buffer & buf,u8 try_count);

	eErrorTp WriteSingleCoil(u8 slave_addr,u32 addr,buffer & buf);
    //WriteMultipleCoil, один элемент = 1 байт
	eErrorTp WriteMultipleCoil(u8 slave_addr,u32 addr,u32 countItem,buffer & buf);
	eErrorTp WriteSingleReg(u8 slave_addr,u32 addr,buffer & buf);
	 //WriteMultipleReg, один элемент = 2 байта
	eErrorTp WriteMultipleReg(u8 slave_addr,u32 addr,u32 countItem,buffer & buf);

	eErrorTp EnableDebug(void);
	eErrorTp DisableDebug(void);
	eErrorTp SetTimeout(u32 timeout_sec,u32 timeout_usec);
	void SetSwDelay(u32 timeout_msec);
	eErrorTp SetupLevelRTS(u32 mode);
	static void rts_sw (modbus_t * ctx, int on);
	eErrorTp SetupSoftwareRTS(int rts_gpio,int us);
	eErrorTp ReportSlaveID(u8 slave_addr,u8 * dest);
	eErrorTp InitMaster(string slaveip,u16 port);
	eErrorTp reconnect();
	private:
	modbus_t *ctx=NULL;
	bool netMode=false;
	string slaveip="0.0.0.0";
	u16 port=0;
	//u8 *query;
	u8 SERVER_ID=1;
	u32 sw_delay=20;
	GprintT mbp;
	//bool rts_gpio_init=false;
};



#endif /* MODBUSCPP_H_ */
