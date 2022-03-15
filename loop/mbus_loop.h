/*
 * mbus_liip.h
 *
 *  Created on: 18 дек. 2020 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_LOOP_MBUS_LOOP_H_
#define SRVIOT_SRC_ENGINE_LOOP_MBUS_LOOP_H_

#ifdef _MODBUSCPP

#include "engine/engine.h"
#include "custom_project/custom_project.h"
#include "engine/proto/modbuscpp.h"



class mbusLoopGroupEndPoint{
	public:
	eErrorTp config(Json::Value & cfg);
	mbusLoopGroupEndPoint(modbuscpp_client * mbusLine,ThreadT * gp);
	eErrorTp read();
	eErrorTp write();
	eErrorTp readOdm(Json::Value & event);
	eErrorTp writeOdm(Json::Value & event);
	modbuscpp_client * mLine;
	u16 startAddr=0;
	u16 countRegs=0;
	u16 devAddr=0;
	u32 interval=0;
	buffer emuBuf;
	bool needInit=true;
	u32 emu=0;
	u16 try_count=3;
	string groupName="";
	string devName="";
	string channel;
	string mode="";
	string access="read";
	//buffer buf;
	Json::Value dnk;
	Json::Value dnkReadEmu;
	Json::Value eventCfg;
	vector<string> listener;
	ThreadT * print;
	private:
		RTC_Timer timer;
		u8 sendDnkResult=0;
		Json::Value genEvent(string resp,string mode,string & id);
		Json::Value  genReadEventDNK(string msg,string & id,Json::Value resultDnk);
		Json::Value  genReadEvent(string msg,string & id);
		Json::Value  genWriteEvent(string msg,string & id);
		eErrorTp sendPackToListeners(Json::Value & pack);
		eErrorTp encodePackDataDNK(Json::Value & map,buffer & buf, Json::Value & pack,string & id);
		eErrorTp codePackDataDNK(Json::Value & map,buffer & buf,Json::Value & pack);
		eErrorTp writeSeq(string & id);
		eErrorTp readSeq(string & id);
		eErrorTp readEmu(){//получает значения из emu.xxxx преобразует в регистры и пишет в emuBuf
			print->GPRINT(MEDIUM_LEVEL,"emu move from DNK emu.xxx [%d] mode [%s] addr [%d] count [%d]\n",devAddr,mode.c_str(),startAddr,countRegs);
			buffer buf(countRegs*2);
			eErrorTp err=ERROR;
			Json::Value pack;


			codePackDataDNK(dnkReadEmu,buf,pack);

			if (mode=="input")
				memcpy(emuBuf.p(),buf.p(),countRegs*2);
			else
			if (mode=="holding")
				memcpy(emuBuf.p(),buf.p(),countRegs*2);
			else
			if (mode=="discrete")
				memcpy(emuBuf.p(),buf.p(),countRegs);
			else
			if (mode=="coil")
				memcpy(emuBuf.p(),buf.p(),countRegs);

			return NO_ERROR;
		}
};

class mbusLoopGroup{
	public:
	eErrorTp config(Json::Value & groupCfg,Json::Value & listener,string groupName,string devName,u32 devAddr,string channel);

	mbusLoopGroup(modbuscpp_client * mbusLine, ThreadT * gp);

	eErrorTp readWriteByInterval();
	eErrorTp readWriteOdm(Json::Value & event);
	modbuscpp_client * mLine;
	ThreadT * print;
	u32 emu=0;
	vector<shared_ptr<mbusLoopGroupEndPoint>> ep;

};

class mbusLoopDevice{
	public:
	eErrorTp config(Json::Value & devCfg,string channel);
	mbusLoopDevice(modbuscpp_client * mbusLine, ThreadT * gp);
	eErrorTp readWriteByInterval();
	eErrorTp readWriteOdm(Json::Value & event);
	modbuscpp_client * mLine;
	ThreadT * print;
	vector<shared_ptr<mbusLoopGroup>> device;
};

class mbusLoopChannel{
	public:
	mbusLoopChannel(ThreadT * pr,eDebugTp debugLevel){
		print=pr;
		print->debug_level=debugLevel;
		print->	SourceStr="mbusLoop";
		print-> ObjPref="mbusLoop";
		print->GPRINT(NORMAL_LEVEL,"Run mbusLoopChannel\n");

	}
	eErrorTp config(Json::Value cfg);
	eErrorTp readWriteByInterval();
	eErrorTp readWriteOdm(Json::Value & event);
	ThreadT * print;
	bool configured=false;
	vector<shared_ptr<mbusLoopDevice>> chann;
	vector<shared_ptr<modbuscpp_client>> mbusLine;

};

class mbusLoopT: public ThreadT{
public:
	mbusLoopT(eDebugTp debug_level);
	virtual ~mbusLoopT (void){
		GPRINT(HARD_LEVEL,"Destroy mbusLoopT\n");
	}

	eErrorTp emu();
	private:
		buffer FifoPreBuf;
		virtual eErrorTp Loop(void* thisPtr);
		u32 loopdelay=10;
		bool configured=false;
		u32 profReadWriteByInterval=0;
		SettingsAdm sAdm;
		shared_ptr<mbusLoopChannel> mlc;

		eErrorTp config(char * jsonStr){
			Json::Value root=parseJSON(jsonStr);
			//printJSON("config",root);
			mlc.reset();
			mlc=make_shared<mbusLoopChannel>(this,debug_level);

			if (mlc->config(root)==NO_ERROR){
				configured=true;

			}
			else
				return ERROR;

			return NO_ERROR;
		}

};


#endif
#endif /* SRVIOT_SRC_ENGINE_LOOP_MBUS_LOOP_H_ */
