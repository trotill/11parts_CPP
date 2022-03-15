/*
 * iBOARDS.h
 *
 *  Created on: 16 июля 2014 г.
 *      Author: root
 */

#ifndef ENGINE_H_
#define ENGINE_H_

#include "engine/thread.h"

#include "engine/global.h"
#include "engine/loop/jnoda_adapter.h"
#include "engine/loop/websrv_adapter.h"
#include "engine/loop/reporter_admin.h"
#include "engine/loop/safe_storage.h"
#include "engine/loop/factory_reset.h"
#include "engine/loop/watchdog_admin.h"
#include "engine/loop/guard_loop.h"
#include "engine/loop/mq_collector.h"
#include "engine/loop/update_admin.h"
#include "custom_project/custom_main.h"
#ifdef _SKEY
#include "engine/periphery/skey.h"
#endif

#define REPEATED_THREAD_CMD_DELAY 10000

template <class T>
u32 CheckTermObjectT(T * r)
{
	if (r!=NULL)
		return r->TERMState;
	else
		return TERM_STATE_RISE;
}

template <class T>
eErrorTp SetTermObjectT(T * r,u8 TERM)
{
	if (r!=NULL)
	{
		r->TERMReq=TERM;
		return NO_ERROR;
	}
	else
		return ERROR;
}

class ThreadParamsT{
	public:

	ThreadParamsT(u8 type,u8 srvmMsg,string name,u32 repeated_thread_delay,u32 anyActionTimeoutWDT):type(type),srvmMsg(srvmMsg),name(name),rt(repeated_thread_delay){
		if (anyActionTimeoutWDT!=0){
			aatWDT_RTC.setInMSec(anyActionTimeoutWDT,LOOPTIMER_TIMEOUT_MODE);
			enAatWDT=true;
		}
	}
	u8 type=THREAD_RUN_TYPE_ONCE_ALWAYS;
	u8 srvmMsg=srvmUNDEF;
	string name;
	RTC_Timer rt;
	RTC_Timer aatWDT_RTC;
	bool enAatWDT=false;
	bool firstStart=true;
};

class ThreadCollector : public ThreadT{
	public:
	template<class T>
	eErrorTp AddModule(string name){
		string plug="1";
		u32 logl=HARD_LEVEL;
		JSON_ReadConfigField(CnT->json_cfg,(char*)name.c_str(),plug);
		JSON_ReadConfigField(CnT->json_cfg,(char*)string(name+"_loglevel").c_str(),logl);
		if (plug=="1"){
			coll.emplace(name,make_shared<T>(logl));
			coll_params.emplace_back(make_shared<ThreadParamsT>(THREAD_RUN_TYPE_ONCE_ALWAYS,srvmUNDEF,name,REPEATED_THREAD_CMD_DELAY,0));
		}
		return NO_ERROR;
	}

	template<class T>
	eErrorTp Create(string key){
			string plug="1";
			u32 logl=HARD_LEVEL;
			JSON_ReadConfigField(CnT->json_cfg,(char*)key.c_str(),plug);
			JSON_ReadConfigField(CnT->json_cfg,(char*)string(key+"_loglevel").c_str(),logl);
			if (plug=="1"){
				coll.emplace(key,make_shared<T>(logl));
			}
			return NO_ERROR;
		}
	eErrorTp AddAction(string name,u8 type,u8 srvmMsg){
				string plug="1";
				JSON_ReadConfigField(CnT->json_cfg,(char*)name.c_str(),plug);
				if (plug=="1"){
					coll_params.emplace_back(make_shared<ThreadParamsT>(type,srvmMsg,name,REPEATED_THREAD_CMD_DELAY,0));
				}
				return NO_ERROR;
	}

	template<class T>//repeated_thread_delay - задержка перед перезапуском потока,
	//anyActionTimeoutWDT - таймаут первого запуска, если 0 не использ. Если в течении таймаута поток не стартанет, necron перезапускается,
	//используется для гарантированного запуска, на случай если сообщение от Jnoda не дойдет
	eErrorTp AddModule(string name,u8 type,u8 srvmMsg,u32 repeated_thread_delay,u32 anyActionTimeoutWDT){
		string plug="1";
		u32 logl=HARD_LEVEL;
		JSON_ReadConfigField(CnT->json_cfg,(char*)name.c_str(),plug);
		JSON_ReadConfigField(CnT->json_cfg,(char*)string(name+"_loglevel").c_str(),logl);
		if (plug=="1"){
			coll.emplace(name,make_shared<T>(logl));
			coll_params.emplace_back(make_shared<ThreadParamsT>(type,srvmMsg,name,repeated_thread_delay,anyActionTimeoutWDT));
		}
		return NO_ERROR;
	}

	template<class T>
	eErrorTp AddModule(string name,u8 type,u8 srvmMsg){
		return AddModule<T>((string)name,(u8)type,(u8)srvmMsg,(u32)REPEATED_THREAD_CMD_DELAY,0);
	}

	eErrorTp AddSharedModule(shared_ptr<FifoEngineT> & obj,string name);
	ThreadCollector();
	eErrorTp RunAll(void);
	eErrorTp Stop(string key);
	eErrorTp Start(string key);
	eErrorTp Create(string key);
	eErrorTp Remove(string key);
	eErrorTp RemoveAll(void);
	virtual ~ThreadCollector(void);
	std::map<string,shared_ptr<ThreadT>> coll;
	std::vector<shared_ptr<ThreadParamsT>> coll_params;

	eErrorTp ProjectThread();
	private:
		virtual eErrorTp Loop(void* thisPtr);
		u32 loopdelay=100;

};




#endif /* IBOARDS_H_ */
