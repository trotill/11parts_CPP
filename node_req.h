/*
 * node_req.h
 *
 *  Created on: 18 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_NODE_REQ_H_
#define SRC_ENGINE_NODE_REQ_H_
#include "auth.h"

#define MAX_CACHE_ELEMENTS 30
#define NOREREQ_STRING_BUF 10000

typedef struct sRecCache
{
	string iface;
	string req;
	string data;
	TIME_T SendTime;
	u32 FiltTime;
}sRecCache;

class CasherT {
	public:
	CasherT();
	CasherT(u32 max);
	~CasherT();
	eErrorTp RequestFilter(string & iface,char * req,u32 FltTime);
	eErrorTp ToCashe(string & iface,char * req,char * data);
	eErrorTp FromCashe(string & iface,string & req,string & data);
	private:
		u32 max_cashe;
		sRecCache ** Cache;
};

class NodeReqT : public NodeT{
	public:
		NodeReqT (eDebugTp debug_level);
		~NodeReqT (void);
		bool JnodaRun=false;
		BufHandler SrvReadBuf;
		BufHandler SrvSendBuf;
		BufHandler SrvBufLH;
		string smsd_GotSMS_action="smsd";
		string reconf_device_action="cadm_reconf_device";
		string serialport_gsm_GotSMS_action="spgSMS";
		string serialport_gsm_GotCall_action="spgIncall";
		CasherT Cache;
		eErrorTp ReqNODE(u32 pack, u32 vers,BufHandler * json_req,BufHandler * json_resp);
		eErrorTp MatchResult(string & iface,string & req,u32 len,BufHandler * json_resp);
		eErrorTp RND_Parse(void);
		eErrorTp CMD(BufHandler * json_req,BufHandler * json_resp);
		eErrorTp CMD_System(u32 ptype,Json::Value & jsonReq,Json::Value & jsonResp);
		eErrorTp CMD_Event(string json);
		eErrorTp ParseServerSOCKET_IO(Json::Value & root);
		eErrorTp ReqEvent(u32 vers,BufHandler * json_req,BufHandler * json_resp);
		eErrorTp ReconfDeviceAction(Json::Value & root);
		eErrorTp jnodaCMD_System_Settings_post_def(string savename,string & json);
		eErrorTp jnodaCMD_System_Settings_pre_def(string savename,string & json);
		u32 FieldNameToID(u32 ptype,string & name);
		//eErrorTp UpdateImageStd(string & json_req,string & json_resp);
		SecureAccessT * Group[MAX_GROUP]={NULL};
	private:
		SettingsAdm Sm;

};

eErrorTp repSendTestMail(string & json);
#endif /* SRC_ENGINE_NODE_REQ_H_ */
