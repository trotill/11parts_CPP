/*
 * node.h
 *
 *  Created on: 18 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_NODE_H_
#define SRC_ENGINE_NODE_H_

#include "thread.h"
#include "ipc_socket.h"
#include "global.h"
#include "settings_adm.h"



class  NodeT: public IPC_SocketT {
	public:
	NodeT(void){
		InitLHBus(CnT->NODE_PORT_CPP,CnT->NODE_PORT_NODEJS,(char*)CnT->NODE_GROUP_IP.c_str());
	}
	~NodeT(void){
		CloseNET_sock();
	}
    eErrorTp ApplySetting(string json);
	protected:
    eErrorTp ReqNetInfo(string net,string reqparams);
    eErrorTp RouterApplySetting(string json){
    	printf("RouterApplySetting obsolete, change to ApplySetting!!!\n");
    	return ApplySetting(json);
    }
    eErrorTp DeInit(string json);
    eErrorTp LoadAndApplySetting(string setting_name);
    eErrorTp Configure(void);
    eErrorTp ConfigNode(char*);
    eErrorTp ConfigNodeDef(void);

    SettingsAdm Sm;
};


#endif /* SRC_ENGINE_NODE_H_ */
