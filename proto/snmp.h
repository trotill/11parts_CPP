/*
 * snmp.h
 *
 *  Created on: 22 марта 2018 г.
 *      Author: root
 */

#ifndef SNMP_H_
#define SNMP_H_
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
//#include <signal.h>
//#include <fcntl.h>
//#include <sys/stat.h>
//#include <sys/types.h>
//#include <unistd.h>
//#include "guards.h"
//#include <json/json.h>

#include "engine/print.h"
#include "engine/lib/11p_string.h"


#define MAX_PARAMETERS 100
#define MAX_SYMBOLS 1024

int OidHandler(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests);

u32 GetNodeType(string stype);

class OIDObj{
	public:
		OIDObj(string sname, string saccess, string soid, string stype, string sdefval);
		u32 first_run=1;
		int intValue=0;
		string strValue="";
		int boolValue=0;
		string name="";
		u32 node_type;
		Mutex_t Mutex;
	private:
		eErrorTp RegOIDValue(string & name,string & oid,int modes);
};

class SNMP_WrNamedFifoMutex{
	public:
	SNMP_WrNamedFifoMutex(){
		mutex_init(WrNamedFifo);
	}
	void lock(){

		mutex_lock(WrNamedFifo);
	}
	void unlock(){
		mutex_unlock(WrNamedFifo);
	}
	Mutex_t WrNamedFifo;
};

extern SNMP_WrNamedFifoMutex SNMP_mutex;

class SNMP_adm : public GprintT {

	public:
	eErrorTp SNMP_Init(string & subagent_cfg);
	eErrorTp SNMP_DumpCfg();
	eErrorTp ParseSNMPAgentStr(char * bufr,u32 len);
	eErrorTp GetSNMP_Val(void);
	eErrorTp SetSNMP_Val(string name,string val);
	eErrorTp SetSNMP_Val(string name,u32 val);

	private:
			char SNMPGetBuf[10000];
			//string source_description[2];
			string snmp_fifo_name;
			Json::Value root_suba;

};

typedef struct sGlobalSNMP
{
	char ReadBuf[10000];
	u32 agentx_enable;
	u32 debug_level;
	string agent_name;
	OIDObj * OIDs[MAX_PARAMETERS]={NULL};
	Json::Value root;
	int fdfifo_in;
	int fdfifo_out;
}sGlobalSNMP;
extern sGlobalSNMP sGSN;






#endif /* SNMP_H_ */
