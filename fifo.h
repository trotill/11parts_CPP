/*
 * fifo.h
 *
 *  Created on: 15 дек. 2018 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_FIFO_H_
#define SRVIOT_SRC_ENGINE_FIFO_H_

#include "print.h"
#include "memadmin.h"

class sFifoItem
{
	public:
	    sFifoItem(string name){
	    	this->objname=name;
	    }
		fragfifo FFifo;
		bool FifoReady=false;
		string objname="";
};


class FifoT{
	public:
	FifoT(){
		mutex_init(mutex);
	}
	~FifoT(){
	}
	std::map<string,shared_ptr<sFifoItem>> fifo_item;
	Mutex_t mutex;
};

class FifoEngineT:virtual public GprintT{
	public:
	FifoEngineT (string printname,string objname,eDebugTp debug_level);
	 virtual ~FifoEngineT (void);

	eErrorTp GetUcastFifoMessage(buffer & buf, u8 & MsgType, string fifoName);
	eErrorTp GetUcastFifoMessage(buffer & buf, sInterThrMsgHeader & MType);
	eErrorTp GetUcastFifoMessage(BufHandler * buf, sInterThrMsgHeader & Mheader);
	eErrorTp SendUcastFifoMessage(BufHandler * buf, sInterThrMsgHeader & Mheader, string Dest);

	eErrorTp SendMcastFifoMessage(BufHandler * buf,sInterThrMsgHeader & Mheader);
	eErrorTp SendSharedFifoMessage(u8 Msg,string Dest, string & data);
	eErrorTp SendSharedFifoMessage(u8 Msg,string Dest, Json::Value & data);
	eErrorTp SendSharedFifoMessage(u8 Msg,string Dest, u8 * Data=NULL,u32 DataLen=0);
	eErrorTp SendEventFromCnodaToUI(string dMessageJson);
	eErrorTp SendDataFromCnodaToUI_base(string dMessage,u32 json_pack_type,u32 json_pack_version, string user ,char * action);
	eErrorTp RequestEventFromCnodaToJnoda_base(char * iface,char * reqname,char * reqvalue, u32 timeout);
	eErrorTp SendSystemFromCnodaToUI(string dMessageJson,string user);
	eErrorTp SendEventFromCnodaToUI(string dMessageJson,string user);
	eErrorTp SendWebEventFromCnodaToUI(string req,string iface,Json::Value & result);
	eErrorTp activationMessageActivated();
	eErrorTp activationMessageNeedLic();
	eErrorTp SendSystemFromCnodaToUI(string dMessageJson);
	eErrorTp SendUserWebEventFromCnodaToUI(string userEvent,Json::Value & args);
	eErrorTp SendUserWebEventFromCnodaToUI(string userEvent,string ssid,Json::Value & args);
	//eErrorTp SendUserWebEventFromCnodaToUI(string userEvent,Json::Value & args);
	eErrorTp SendDataFromCnodaToWEBSRV(string dMessage,string Action);

	eErrorTp SendReport_SMS(vector<string> & tel,string data);
	eErrorTp SendReport_eMail(vector<string> & email_addr,string data,string subject,vector<string> & file,bool html);
	eErrorTp SendReport_eMail(vector<string> & email_addr,string data,bool html);
	eErrorTp SendReadyToJNODA();
	eErrorTp SendCnodaReadyToJNODA();
	eErrorTp SendWaitToJNODA();
	eErrorTp SendEventToJNODA( string dMessageJson);

	//SendRestartSettingToJNODA format - {"t":[11,1],"d":{"type":"restart_setting","arg":["ethernet_network0"]}}
	eErrorTp SendRestartSettingToJNODA(string setting_name);
	eErrorTp SendRestartNecron(void);
	eErrorTp SendRestartClusterToJNODA(void);
	eErrorTp SetCnodaValue(string name,string val);
	eErrorTp SetCnodaValue(string name,u32 val);
	eErrorTp SendActionLocal(u8 Msg,string Dest,string action,Json::Value & data);
	eErrorTp SendActionLocal(u8 Msg,string Dest,string action,string comp,Json::Value & data);
	eErrorTp SendMQ(char * topic,string & data,bool comp);//Send to MQTT admin, comp - compress/raw
	eErrorTp SendMQ(char * topic,Json::Value & jdata,bool comp=false,u8 prec=5);//Send to MQTT admin, comp - compress/raw
	//private:
		sFifoItem * fifo=NULL;
		string objname="";
		u32 fifo_idx=0;
	protected:
		eErrorTp SendReport(Json::Value & repdata);
		vector<string> oname;
#ifdef _SKEY
		bool skey=false;
#else
		bool skey=true;
#endif
};

extern shared_ptr<FifoEngineT> shared_fifo;

#endif /* SRVIOT_SRC_ENGINE_FIFO_H_ */
