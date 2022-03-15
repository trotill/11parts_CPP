/*
 * thread.h
 *
 *  Created on: 15 дек. 2018 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_THREAD_H_
#define SRVIOT_SRC_ENGINE_THREAD_H_

#include "fifo.h"
#include "print.h"

#define THREAD_RUN_TYPE_ONCE_ALWAYS 0
#define THREAD_RUN_TYPE_ONCE_ONMSG 1
#define THREAD_RUN_TYPE_RESTART_ONMSG 2


eErrorTp StartPthread(Thread_t * thr,void *(*__start_routine) (void *),void * X);
eErrorTp StopPthread(Thread_t * thr);

class Thread_baseT : virtual public GprintT {
	public:
	    typedef std::function <eErrorTp (void* thisPtr)> LoopServerFunc;
	    eErrorTp Run(void);
	    eErrorTp Stop(void);
		Thread_baseT(void);
		virtual ~Thread_baseT(void);
		eErrorTp StopPthreadC(Thread_t * thr);
		eErrorTp StartPthreadC(Thread_t * thr,void *(*__start_routine) (void *),void * X);
		LoopServerFunc LoopServer;
		BufHandler FifoPreBuf;
		eQuitTp RunServerQState=RUN;
		u8 TERMState=TERM_STATE_RISE;//0 - run, 1 - terminate
		u8 TERMReq=NO_REQ_RISE;//0 - no effect, 1 - trminate req
		u32 AnotherThrCtrlEnMask=0;//Activate AnotherThrReqMask/AnotherThrStatMask 0 - disable, 1 - enable
		u32 AnotherThrReqMask=0;//0 - stop req, 1 - run req
		u32 AnotherThrStatMask=0;//0 - stop, 1 run
	private:
		Thread_t thr=0;
	protected:

		bool IsMcast=false;
	    virtual eErrorTp Loop (void* thisPtr){return NO_ERROR;};
		static void* ThrWrapper(void* thisPtr)
			{
				pthread_detach(pthread_self());
				((Thread_baseT*) thisPtr)->LoopServer(thisPtr);
				((Thread_baseT*) thisPtr)->TERMState=1;
				((Thread_baseT*) thisPtr)->GPRINT(NORMAL_LEVEL,"ThrWrapper exit\n");
		        return NULL;
		    }
};


class ThreadT  : public Thread_baseT, public FifoEngineT {
	public:
	    ThreadT (string printname,string objname,eDebugTp debug_level):Thread_baseT(),FifoEngineT(printname,objname,debug_level){

		};
	    virtual ~ThreadT (void){
		}

	    sInterThrMsgHeader Mheader;
	    bool jnoda_ready=false;

	private:
		virtual eErrorTp Loop (void* thisPtr){
			GPRINT(NORMAL_LEVEL,"ThreadT LOOP\n");
			return NO_ERROR;
		}
};



#endif /* SRVIOT_SRC_ENGINE_THREAD_H_ */
