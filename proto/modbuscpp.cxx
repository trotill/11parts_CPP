/*
 * modbuscpp.cxx
 *
 *  Created on: 14.11.2018
 *      Author: root
 */

#include "modbuscpp.h"


eErrorTp modbuscpp_client::ReadHoldingReg(u8 slave_addr,u32 addr,u32 countItem,buffer & buf,u8 try_count){
	buf.create(countItem*2);
	eErrorTp err= Read(slave_addr,addr,countItem,(u16*)buf.p(),MBUS_REGISTER,try_count);
	if (err==ERROR){
		buf.clear();
	}
	return err;
}

eErrorTp modbuscpp_client::ReadInputReg(u8 slave_addr,u32 addr,u32 countItem,buffer & buf,u8 try_count){
	buf.create(countItem*2);
	eErrorTp err=Read(slave_addr,addr,countItem,(u16*)buf.p(),MBUS_INPUT_REGISTER,try_count);
	if (err==ERROR){
		buf.clear();
	}
	return err;
}

eErrorTp modbuscpp_client::ReadCoil(u8 slave_addr,u32 addr,u32 countItem,buffer & buf,u8 try_count){
	buf.create(countItem);
	eErrorTp err= Read(slave_addr,addr,countItem,(u16*)buf.p(),MBUS_BITS,try_count);
	if (err==ERROR){
		buf.clear();
	}
	return err;
}

eErrorTp modbuscpp_client::ReadDiscreteInput(u8 slave_addr,u32 addr,u32 countItem,buffer & buf,u8 try_count){
	buf.create(countItem);
	eErrorTp err= Read(slave_addr,addr,countItem,(u16*)buf.p(),MBUS_INPUT_BITS,try_count);
	if (err==ERROR){
		buf.clear();
	}
	return err;
}

eErrorTp modbuscpp_client::Read(u8 slave_addr,u32 addr,u32 countItem,u16 * dest,tpModBusData type,u8 try_count){
		if (ctx==NULL)
			return ERROR;
		int rc=-1;
		modbus_flush(ctx);
		modbus_set_slave(ctx, slave_addr);

	//	u8 * buf8=(u8*)calloc(1,count_regs);
		//u16 size8=count_regs;
		//for (u32 j=0;j<count_regs;j++)
		//	buf8[j]=(u8)dest[j];

		for (u8 z=0;z<try_count;z++){
	   	  switch (type){
			case MBUS_REGISTERS:
			case MBUS_REGISTER:
				//3 (0x03) — чтение значений из нескольких регистров хранения (Read Holding Registers)
				rc = modbus_read_registers(ctx, addr, countItem, dest);
			break;
			case MBUS_BITS:
				//1 (0x01) — чтение значений из нескольких регистров флагов (Read Coil Status).
				rc = modbus_read_bits(ctx, addr, countItem,(u8*)dest);
			break;
			case MBUS_INPUT_BITS:
				//2 (0x02) — чтение значений из нескольких дискретных входов (Read Discrete Inputs).
				rc = modbus_read_input_bits(ctx, addr, countItem, (u8*)dest);
			break;
			case MBUS_INPUT_REGISTER:
				//4 (0x04) — чтение значений из нескольких регистров ввода (Read Input Registers).
				rc = modbus_read_input_registers(ctx, addr, countItem, dest);
			break;
			default:
				mbp.GPRINT(NORMAL_LEVEL,"Incorrect MBUS type\n");
	   	  }
	   	  if (rc != -1) break;
	   	  mbp.GPRINT(NORMAL_LEVEL,"Addr 0x%02x read error, try again, delay %dmS\n",slave_addr,sw_delay);
	   	  mdelay(sw_delay);
		}

	   //	free(buf8);
	   	if (rc == -1) {
				mbp.GPRINT(NORMAL_LEVEL,"Error %d while reading: %s, addr 0x%02x\n", errno, modbus_strerror(errno),slave_addr);
	   	        return ERROR;
	   	    }

	   	return NO_ERROR;
	}

eErrorTp modbuscpp_client::WriteMultipleReg(u8 slave_addr,u32 addr,u32 countItem,buffer & buf){
	if (buf.size()<(countItem*2)){
		mbp.GPRINT(NORMAL_LEVEL,"Incorrect write buffer size\n");
		return ERROR;
	}
	return Write(slave_addr,addr,countItem,(u16*)buf.p(),MBUS_REGISTERS);
}

eErrorTp modbuscpp_client::WriteSingleReg(u8 slave_addr,u32 addr,buffer & buf){
	if (buf.size()<2){
		mbp.GPRINT(NORMAL_LEVEL,"Incorrect write buffer size\n");
		return ERROR;
	}
	return Write(slave_addr,addr,1,(u16*)buf.p(),MBUS_REGISTER);
}

eErrorTp modbuscpp_client::WriteMultipleCoil(u8 slave_addr,u32 addr,u32 countItem,buffer & buf){
	if (buf.size()<(countItem)){
		mbp.GPRINT(NORMAL_LEVEL,"Incorrect write buffer size\n");
		return ERROR;
	}
	return Write(slave_addr,addr,countItem,(u16*)buf.p(),MBUS_BITS);
}

eErrorTp modbuscpp_client::WriteSingleCoil(u8 slave_addr,u32 addr,buffer & buf){
	if (buf.size()<1){
		mbp.GPRINT(NORMAL_LEVEL,"Incorrect write buffer size\n");
		return ERROR;
	}
	return Write(slave_addr,addr,1,(u16*)buf.p(),MBUS_BIT);
}

eErrorTp modbuscpp_client::Write(u8 slave_addr,u32 addr,u32 size16,u16 * buf,tpModBusData type){
		if (ctx==NULL)
			return ERROR;
		int rc=-1;

		//printf("modbus_flush(ctx);\n");
		modbus_flush(ctx);
		//printf("modbus_set_slave(ctx, slave_addr);\n");
		modbus_set_slave(ctx, slave_addr);

		//printf("u8 * buf8=(u8*)calloc(1,size16);\n");
		//u8 * buf8=(u8*)calloc(1,size16);
		//u16 size8=size16;
		//for (u32 j=0;j<size16;j++)
		//	buf8[j]=(u8)buf[j];


	   	switch (type){
	   		case MBUS_INPUT_REGISTER:
			case MBUS_REGISTER:{
				//6 (0x06) — запись значения в один регистр хранения (Preset Single Register).
				u32 reg=0;
				memcpy(&reg,(u8*)buf,2);
				//printf("Write 0x%02x reg 0x%04x\n",addr,reg);
				rc = modbus_write_register(ctx, addr,reg);
				//printf("MBUS_REGISTER rc %d\n",rc);
			}
			break;
			case MBUS_REGISTERS:{
				//16 (0x10) — запись значений в несколько регистров хранения (Preset Multiple Registers)
				rc = modbus_write_registers(ctx, addr,size16,buf);
			}
			break;
			case MBUS_BIT:{
				//5 (0x05) — запись значения одного флага (Force Single Coil).
				u32 status=0;
				memcpy(&status,(u8*)buf,1);
				rc = modbus_write_bit(ctx, addr, status);
			}
			break;
			case MBUS_INPUT_BITS:
			case MBUS_BITS:{
				//The buf array must contains bytes set to TRUE or FALSE.

				//15 (0x0F) — запись значений в несколько регистров флагов (Force Multiple Coils)
				//printhex(buf8,size8,16);
				rc = modbus_write_bits(ctx, addr, size16,(u8*)buf);
			}

			break;
			default:{
				mbp.GPRINT(NORMAL_LEVEL,"Incorrect MBUS type\n");
			}
	   	}

	   //	free(buf8);
	   	if (rc == -1) {
				mbp.GPRINT(NORMAL_LEVEL,"Error %d while write: %s\n", errno, modbus_strerror(errno));
	   	        return ERROR;
	   	    }

	   	return NO_ERROR;
	}

modbuscpp_client::modbuscpp_client(eDebugTp debugLevel)
	{
		mbp.SourceStr="modbus";
		mbp.ObjPref="modbus";
		mbp.debug_level=debugLevel;
	}

modbuscpp_client::modbuscpp_client(void)
	{
		mbp.SourceStr="modbus";
		mbp.ObjPref="modbus";
		mbp.debug_level=NORMAL_LEVEL;
	}
modbuscpp_client::~modbuscpp_client(void){
		if (ctx!=NULL){
			modbus_close(ctx);
			modbus_free(ctx);
		}
		if (__gpio_RTS_MODBUS!=NULL){
			delete __gpio_RTS_MODBUS;
			__gpio_RTS_MODBUS=NULL;
		}
	}
	//
eErrorTp modbuscpp_client::InitMaster(string ttyport,u32 baudrate,char parity, int data_bit, int stop_bit){
		/*
		 * The baud argument specifies the baud rate of the communication, eg. 9600, 19200, 57600, 115200, etc.
		   The parity argument can have one of the following values
				N for none
				E for even
				O for odd
		   The data_bits argument specifies the number of bits of data, the allowed values are 5, 6, 7 and 8.
		   The stop_bits argument specifies the bits of stop, the allowed values are 1 and 2.
		 */
	  	  if (ctx!=NULL)
	  		  return ERROR;

		  ctx = modbus_new_rtu(ttyport.c_str(), baudrate, parity, data_bit, stop_bit);

		  modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);

		  //modbus_set_slave(ctx, SERVER_ID);

		  if (modbus_connect(ctx) == -1) {
		         mbp.GPRINT(NORMAL_LEVEL,"Connection failed: %s\n",modbus_strerror(errno));
		         modbus_free(ctx);
		         ctx=NULL;
		         return ERROR;
		   }
		  else
			  return NO_ERROR;
	}

eErrorTp modbuscpp_client::reconnect(){

	if (netMode){
		mbp.GPRINT(NORMAL_LEVEL,"Do reconnect\n");
		modbus_close(ctx);
		modbus_free(ctx);
		ctx=NULL;
		ctx = modbus_new_tcp(slaveip.c_str(), port);
		if (modbus_connect(ctx) == -1) {
			 mbp.GPRINT(NORMAL_LEVEL,"reconnect failed: %s\n",modbus_strerror(errno));
			 modbus_close(ctx);
			 modbus_free(ctx);
			 ctx=NULL;
			 return ERROR;
		}
		else{
			mbp.GPRINT(NORMAL_LEVEL,"Reconnect success\n");
			return NO_ERROR;
		}
	}
	else
		return ERROR;
}

eErrorTp modbuscpp_client::InitMaster(string slaveip,u16 port){
	  if (ctx!=NULL)
		  return ERROR;

	  netMode=true;
	  this->slaveip=slaveip;
	  this->port=port;
	  ctx = modbus_new_tcp(slaveip.c_str(), port);
	  //modbus_set_slave(ctx, SERVER_ID);

	  if (modbus_connect(ctx) == -1) {
			  mbp.GPRINT(NORMAL_LEVEL,"Connection failed: %s\n",modbus_strerror(errno));
			 modbus_free(ctx);
			 ctx=NULL;
			 return ERROR;
		 }
	  else
		  return NO_ERROR;
}

eErrorTp modbuscpp_client::EnableDebug(void){
		if (ctx==NULL)
			return ERROR;
	  modbus_set_debug(ctx, true);
	  return NO_ERROR;
	}
eErrorTp modbuscpp_client::DisableDebug(void){
		if (ctx==NULL)
			return ERROR;
	  modbus_set_debug(ctx, false);
	  return NO_ERROR;
	}

//sw_delay
void modbuscpp_client::SetSwDelay(u32 timeout_msec){
	sw_delay=timeout_msec;
}
eErrorTp modbuscpp_client::SetTimeout(u32 timeout_sec,u32 timeout_usec){
		if (ctx==NULL)
			return ERROR;
		//modbus_set_byte_timeout(ctx, byte_timeout);
		modbus_set_response_timeout(ctx, timeout_sec,timeout_usec);
		return NO_ERROR;
	}

eErrorTp modbuscpp_client::SetupLevelRTS(u32 mode){
		if (ctx==NULL)
			return ERROR;
		//MODBUS_RTU_RTS_DOWN
		//MODBUS_RTU_RTS_UP
		//MODBUS_RTU_RTS_NONE
		modbus_rtu_set_rts(ctx, mode);
		return NO_ERROR;
	}

 void modbuscpp_client::rts_sw (modbus_t * ctx, int on){
		//if (RTS_GP==0)
			//return;

		if (on){
			__gpio_RTS_MODBUS->set("1");
			//system("echo 1 > /sys/class/gpio/gpio23/value");
			//GpioOnOff(23,1);
			//printf("RTS on\n");
		}
		else{
			__gpio_RTS_MODBUS->set("0");
			//system("echo 0 > /sys/class/gpio/gpio23/value");
			//GpioOnOff(23,0);
			//printf("RTS off\n");
		}
	}

eErrorTp modbuscpp_client::SetupSoftwareRTS(int rts_gpio,int us){
		if (ctx==NULL)
			return ERROR;

		 if (__gpio_RTS_MODBUS==NULL){
			 //GpioOutInit(rts_gpio,0);
			 //RTS_GP=rts_gpio;
			 //string num,string direct,string value)
			 __gpio_RTS_MODBUS=new gpio(IntToStr(rts_gpio),"out","0");
		 }
		 modbus_rtu_set_custom_rts(ctx, rts_sw);
		 modbus_rtu_set_rts_delay(ctx,us);
		 return NO_ERROR;

	}
eErrorTp modbuscpp_client::ReportSlaveID(u8 slave_addr,u8 * dest)
	{
		/*
		int rc=-1;
		modbus_flush(ctx);
		modbus_set_slave(ctx, slave_addr);
		rc=modbus_report_slave_id(ctx,dest);
		if (rc > 1) {
			mbp.GPRINT(NORMAL_LEVEL,"Run Status Indicator: %s\n", dest[1] ? "ON" : "OFF");
			return NO_ERROR;
		}
		else
			mbp.GPRINT(NORMAL_LEVEL,"Error report slave %s\n");
*/
		return ERROR;
	}


