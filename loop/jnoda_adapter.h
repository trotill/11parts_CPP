/*
 * jnoda_adapter.h
 *
 *  Created on: 15 дек. 2018 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_LOOP_JNODA_ADAPTER_H_
#define SRVIOT_SRC_ENGINE_LOOP_JNODA_ADAPTER_H_

#include "engine/thread.h"
#include "engine/ipc_socket.h"
#include "engine/node_req.h"
#include "engine/global.h"

class Jnoda_adapter  : public ThreadT {
	public:
	Jnoda_adapter(eDebugTp debug_level);
	virtual ~Jnoda_adapter (void){
		GPRINT(HARD_LEVEL,"Destroy Jnoda_adapter\n");
	}
	private:
		virtual eErrorTp Loop(void* thisPtr);
		shared_ptr<NodeReqT> NRT;
		buffer FifoPreBuf;
		u32 loopdelay=50;
};



#endif /* SRVIOT_SRC_ENGINE_LOOP_JNODA_ADAPTER_H_ */
