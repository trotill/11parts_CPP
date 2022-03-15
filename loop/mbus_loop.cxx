/*
 * mbus_loop.cxx
 *
 *  Created on: 18 дек. 2020 г.
 *      Author: root
 */

#ifdef _MODBUSCPP
#include "mbus_loop.h"

mbusLoopT::mbusLoopT(eDebugTp debug_level):ThreadT("mbus","mblo",debug_level){
	JSON_ReadConfigField(CnT->json_cfg,"mblo_loopdelay",loopdelay);
	//u32 ll;
	//JSON_ReadConfigField(CnT->json_cfg,"mblo_loglevel",ll);
	//debug_level=ll;
	JSON_ReadConfigField(CnT->json_cfg,"mblo_profReadWriteByInterval",profReadWriteByInterval);

}

eErrorTp mbusLoopT::Loop(void* thisPtr){
	bool rtrg=false;

	bool doConfigure=false;
	string configData="{}";
	while(1){

		if (configured){

			if (profReadWriteByInterval){
				profiler prof("readIntervalProf");
				mlc->readWriteByInterval();
				prof.prof_show();
			}
			else{
				mlc->readWriteByInterval();
			}
		}
		else{
			if (doConfigure){
				sleep(1);
				GPRINT(NORMAL_LEVEL,"Do reconnect\n");
				if (config((char*)configData.c_str())==ERROR){
					doConfigure=true;
				}
				else{
					doConfigure=false;
				}
			}
		}
		while (GetUcastFifoMessage(FifoPreBuf,Mheader)!=ERROR){
			GPRINT(PARANOIDAL_LEVEL,"Get MsgType %d\n",Mheader.MsgType);
			//printf("Mheader.MsgType %d sz %d buf %s\n",Mheader.MsgType,FifoPreBuf.size(),FifoPreBuf.p());

			if (jnoda_ready){
			}


			switch(Mheader.MsgType){
				case srvmJNODA_READY:
					jnoda_ready=true;
				break;

				case srvmMODBUS_MESSAGE:{
					if (configured){
						Json::Value cfg;
						GPRINT(MEDIUM_LEVEL,"srvmMODBUS_MESSAGE readWriteOdm cfg [%s]\n",FifoPreBuf.p());
						cfg=parseJSON((char*)FifoPreBuf.p());
						//printJSON("cfg",cfg);
						//CnT->dnk_var->Dump("emu",false);
						mlc->readWriteOdm(cfg);
						//CnT->dnk_var->Dump("emu",false);

						//exit(1);
						//zr();
					}
					else{
						GPRINT(MEDIUM_LEVEL,"SKIP readWriteOdm srvmMODBUS_MESSAGE, unconfigured cfg [%s]\n",FifoPreBuf.p());
					}
				}
				break;

				case srvmMODBUS_CONFIG:{
					//printf("srvmMODBUS_CONFIG sz %d buf %s\n",FifoPreBuf.size(),FifoPreBuf.p());
					//exit(1);


					if (config((char*)FifoPreBuf.p())==ERROR){
						doConfigure=true;
						configData=(char*)FifoPreBuf.p();

					}
					else{
						doConfigure=false;
					}

				}
				break;

				case srvmMOSQUITTO_READY:
					GPRINT(NORMAL_LEVEL,"Get srvmMOSQUITTO_READY, run MQTT_setup()\n");
				break;
				default:
					GPRINT(NORMAL_LEVEL,"undef msg type [%d]\n",Mheader.MsgType);
			}

		}
		if (TERMReq){
			break;
		}
		mdelay(loopdelay);
	}
	return NO_ERROR;
}
#endif
