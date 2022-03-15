/*
 * websrv_adapter.h
 *
 *  Created on: 30 мар. 2020 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_LOOP_WEBSRV_ADAPTER_H_
#define SRVIOT_SRC_ENGINE_LOOP_WEBSRV_ADAPTER_H_

#include "engine/thread.h"
#include "engine/ipc_socket.h"
#include "engine/node_req.h"
#include "engine/global.h"

#define MAX_ITEM_WEB_UI_JSON 20
#define LICENSE_ERROR_DEVICE_IS_BROKEN "lic_error"

class Websrv_adapter  : public ThreadT,IPC_SocketT {
	public:
	Websrv_adapter(eDebugTp debug_level);
	virtual ~Websrv_adapter (void){
		GPRINT(HARD_LEVEL,"Destroy Websrv_adapter\n");
	}
	private:
		//buffer FifoPreBuf;
		eErrorTp SendJSON_ToNode(string cmd,string json);
		eErrorTp DelayedSendUI_JSON_BUF(u32 & web_ui_client_cnt,
				vector<string> & js_ui_cache,
				u32 & time1s_cache_delay,
				u32 & time1s_cache_delay_counter
				);
		virtual eErrorTp Loop(void* thisPtr);
		//shared_ptr<IPC_SocketT> ips;
		u16 WDT_PORT_CPP=11300;
		u16 WDT_PORT_NODEJS=11400;
		string NODE_GROUP_IP="239.100.100.1";
		u32 loopdelay=50;
};



#endif /* SRVIOT_SRC_ENGINE_LOOP_WEBSRV_ADAPTER_H_ */
