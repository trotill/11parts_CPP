/*
 * engine.cxx
 *
 *  Created on: 15 янв. 2019 г.
 *      Author: root
 */

#include "engine.h"


eErrorTp ThreadCollector::AddSharedModule(shared_ptr<FifoEngineT> & obj,string name){
		string plug="1";
		u32 logl=HARD_LEVEL;
		//string nn="";
		//JSON_ReadConfigField(CnT->json_cfg,(char*)name.c_str(),plug);
		JSON_ReadConfigField(CnT->json_cfg,(char*)name.c_str(),plug);
		JSON_ReadConfigField(CnT->json_cfg,(char*)string(name+"_loglevel").c_str(),logl);
		debug_level=(eDebugTp)logl;
		if (plug=="1"){
			obj=make_shared<FifoEngineT>("shared",name,logl);
			Customer.SharedPrint.debug_level=debug_level;
			GPRINT(NO_USE,"\n\nShared Fifo loglevel %d\n\n",logl);
		}
		return NO_ERROR;
	}
ThreadCollector::ThreadCollector():ThreadT("ThreadCollector","tc",MEDIUM_LEVEL){
	 	SourceStr="ThreadCollector";
	 	ObjPref="tc";
	 	debug_level=MEDIUM_LEVEL;
		AddSharedModule(shared_fifo,"sh");
		AddModule<Jnoda_adapter>(string("jnad"));
		AddModule<Websrv_adapter>(string("wead"));
		AddModule<Safe_storage>(string("sast"));
		AddModule<Factory_reset>(string("fars"));
		AddModule<Watchdog_admin>(string("waad"));
		AddModule<Guard_loop>(string("gulo"));
		AddModule<Update_admin>(string("upad"));
		AddModule<Reporter_loop>(string("relo"));
		AddModule<Mq_collector>(string("mqco"));
		ProjectThread();
		 GPRINT(MEDIUM_LEVEL,"ThreadCollector created\n");
	}
eErrorTp ThreadCollector::Start(string key){
	for (auto it = coll.begin(); it != coll.end(); ++it)
	  {
	//	if (coll_params[it->first]->type==THREAD_RUN_TYPE_ONCE_ALWAYS){
		 //for (u32 n=0;n<coll_params.size();n++){
		  if ((key==it->first)){
			  SetTermObjectT(it->second.get(),NO_REQ_RISE);
			  it->second->Run();
			  GPRINT(MEDIUM_LEVEL,"Start %s\n",it->first.c_str());

			  return NO_ERROR;
		  }
		// }
		//}
	  }
	return ERROR;
}
eErrorTp ThreadCollector::Stop(string key){
		GPRINT(MEDIUM_LEVEL,"try stop %s\n",key.c_str());
		if (coll.count(key)==0){
			GPRINT(MEDIUM_LEVEL,"thread %s not found, skip\n",key.c_str());
			return ERROR;
		}
		SetTermObjectT(coll[key].get(),TERM_REQ_RISE);
		u32 n=2;
		while(CheckTermObjectT(coll[key].get())!=TERM_STATE_RISE){
			mdelay(n);
			//printf("d%d\n",n);
			if (n<50)
				n++;
		}
		//coll[key]->~ThreadT();

		GPRINT(MEDIUM_LEVEL,"Stoped %s\n",key.c_str());
		coll[key].get()->Stop();
		return NO_ERROR;
}

eErrorTp ThreadCollector::RunAll(void){

#ifdef _SKEY
		kent k;
#endif
		Run();

		GPRINT(MEDIUM_LEVEL,"***Thread list\n");
		for (u32 k=0;k<coll_params.size();k++)
		{
			GPRINT(MEDIUM_LEVEL,"Prio [%d] thread [%s] type [%d]\n",k,coll_params[k]->name.c_str(),coll_params[k]->type);

		}

		GPRINT(MEDIUM_LEVEL,"***\n");
		for (u32 n=0;n<coll_params.size();n++)
		  {
		//	if (coll_params[it->first]->type==THREAD_RUN_TYPE_ONCE_ALWAYS){
			for (auto it = coll.begin(); it != coll.end(); ++it){
		      if ((coll_params[n]->name==it->first)&&(coll_params[n]->type==THREAD_RUN_TYPE_ONCE_ALWAYS)){
		    	  it->second->Run();
		    	  GPRINT(MEDIUM_LEVEL,"Run %s\n",it->first.c_str());
		    	  break;
		      }
			 }
			//}
		  }

		shared_fifo->SendDataFromCnodaToWEBSRV("",TO_WEB_SRV_TYPE_CNODA_READY);
		shared_fifo->SendCnodaReadyToJNODA();
		return NO_ERROR;
	}



eErrorTp ThreadCollector::Remove(string key){
		GPRINT(MEDIUM_LEVEL,"try remove %s\n",key.c_str());
		if (coll.count(key)==0){
			GPRINT(MEDIUM_LEVEL,"thread %s not found, skip\n",key.c_str());
			return ERROR;
		}
		SetTermObjectT(coll[key].get(),TERM_REQ_RISE);
		u32 n=2;
		while(CheckTermObjectT(coll[key].get())!=TERM_STATE_RISE){
			mdelay(n);
			//printf("d%d\n",n);
			if (n<50)
				n++;
		}
		//coll[key]->~ThreadT();
		coll.erase(key);
		GPRINT(MEDIUM_LEVEL,"Removed %s\n",key.c_str());
		return NO_ERROR;
	}
eErrorTp ThreadCollector::RemoveAll(void){
		Thread_baseT * r;
		for (auto it = coll.begin(); it != coll.end(); ++it){
			GPRINT(MEDIUM_LEVEL,"try remove %s\n",it->first.c_str());
			if (coll.count(it->first)==0){
				GPRINT(MEDIUM_LEVEL,"thread %s not found, skip\n",it->first.c_str());
				continue;
			}
			r=coll[it->first].get();
			SetTermObjectT(r,TERM_REQ_RISE);
			u32 n=2;
			while(CheckTermObjectT(r)!=TERM_STATE_RISE){
				mdelay(n);
				//printf("d%d\n",n);
				if (n<50)
					n++;
			}
			//coll[key]->~ThreadT();
			//coll.erase(key);
			GPRINT(MEDIUM_LEVEL,"Removed %s\n",it->first.c_str());
		}
		if (coll.size()!=0)
			coll.clear();
		return NO_ERROR;
	}

eErrorTp ThreadCollector::Loop(void* thisPtr){

//	Mheader.MsgType=srvmJNODA_READY;

	while(1){
			//Customer.SNMP.GetSNMP_Val();
		//printf("ThreadColl dbg_lvl %d\n",debug_level);
		//aatWDT_RTC
		for (u32 n = 0;n<coll_params.size();n++){
		//	string thr_name=coll_params[n]->name;
			if ((coll_params[n]->enAatWDT)&&(coll_params[n]->firstStart)){//&&(coll[thr_name]->TERMState==TERM_STATE_RISE)){
				if (coll_params[n]->aatWDT_RTC.alarm()){
					//завершение necron, поток не запущен за установленное время
					string thr_name=coll_params[n]->name;
					GPRINT(NORMAL_LEVEL,"Thread %s timeout, restart necron\n",thr_name.c_str());
					SendRestartNecron();
				}
			}
		}
		//JSON_PACK_TYPE_WD_RESTART
		while (GetUcastFifoMessage(&FifoPreBuf,Mheader)!=ERROR){
			GPRINT(HARD_LEVEL,"Get MsgType %d\n",Mheader.MsgType);
			for (u32 n = 0;n<coll_params.size();n++)
			  {
				//
				GPRINT(HARD_LEVEL,"check %s msg %d act type %d\n",coll_params[n]->name.c_str(),coll_params[n]->srvmMsg,coll_params[n]->type);
				if (coll_params[n]->type==THREAD_RUN_TYPE_ONCE_ONMSG){
					if (Mheader.MsgType==coll_params[n]->srvmMsg){
						GPRINT(HARD_LEVEL,"Try %s RUN_TYPE_ONCE_ONMSG msg %d\n",coll_params[n]->name.c_str(),coll_params[n]->srvmMsg);
						string thr_name=coll_params[n]->name;
						if (coll_params[n]->rt.alarm()||(coll_params[n]->firstStart)){
							coll_params[n]->firstStart=false;
							coll_params[n]->enAatWDT=false;
							if (coll[thr_name]->TERMState!=ACTIVE_STATE_RISE)
							{
								coll[thr_name]->Run();
								GPRINT(MEDIUM_LEVEL,"Run %s\n",thr_name.c_str());
							}
							else{
								GPRINT(MEDIUM_LEVEL,"Skip Run %s TERMState==ACTIVE_STATE_RISE\n",thr_name.c_str());
							}
						}
						else{
							GPRINT(MEDIUM_LEVEL,"Skip start once %s, delay not sustained\n",thr_name.c_str());
						}
					}

				}
				if (coll_params[n]->type==THREAD_RUN_TYPE_RESTART_ONMSG){
					if (Mheader.MsgType==coll_params[n]->srvmMsg){
						string thr_name=coll_params[n]->name;
						GPRINT(HARD_LEVEL,"Try %s RUN_TYPE_RESTART_ONMSG msg %d\n",coll_params[n]->name.c_str(),coll_params[n]->srvmMsg);
						if (coll_params[n]->rt.alarm()||(coll_params[n]->firstStart)){
							coll_params[n]->firstStart=false;
							coll_params[n]->enAatWDT=false;
							if (coll[thr_name]->TERMState!=ACTIVE_STATE_RISE)
							{
								coll[thr_name]->Run();
								GPRINT(MEDIUM_LEVEL,"Thr %s stoped, only run\n",thr_name.c_str());
								//Start(thr_name);
							}
							else{
								GPRINT(MEDIUM_LEVEL,"Restart %s\n",thr_name.c_str());
								Stop(thr_name);
								Start(thr_name);

							}
						}
						else
							GPRINT(MEDIUM_LEVEL,"Skip Restart %s, delay not sustained\n",thr_name.c_str());
					}
				}
			  }

			switch(Mheader.MsgType){
				case srvmJNODA_READY:
					jnoda_ready=true;
				break;
				default:
					GPRINT(MEDIUM_LEVEL,"undef msg type [%d]\n",Mheader.MsgType);
			}
		}
		if (TERMReq){
			break;
		}
		mdelay(loopdelay);
	}
	return NO_ERROR;
}

ThreadCollector::~ThreadCollector(){
		GPRINT(HARD_LEVEL,"Destroy ThreadCollector\n");
		if (coll.size()!=0)
			coll.clear();
	}

