/*
 * update_admin.cxx
 *
 *  Created on: 19 авг. 2019 г.
 *      Author: root
 */

#include "update_admin.h"

//#include <dirent.h>
//#include <algorithm>
#include "engine/lib/11p_bin.h"

Update_admin::Update_admin(eDebugTp debug_level):ThreadT("Update_admin","upad",debug_level){
		GPRINT(HARD_LEVEL,"Create Update_admin\n");



		JSON_ReadConfigField(CnT->json_cfg,"upad_loopdelay",loopdelay);


		JSON_ReadConfigField(CnT->json_cfg,"upad_storages",ue_params.upad_storages_str);
		Json::Reader reader;
		if( reader.parse(ue_params.upad_storages_str , ue_params.upad_storages )){
			GPRINT(NORMAL_LEVEL,"Update storage %s\n",ue_params.upad_storages_str.c_str());
		}
		else
			GPRINT(NORMAL_LEVEL,"Error parse storages json str [%s]",ue_params.upad_storages_str.c_str());


		JSON_ReadConfigField(CnT->json_cfg,"upad_firmware_file",ue_params.upad_firmware_file);
		JSON_ReadConfigField(CnT->json_cfg,"upad_interactive",ue_params.upad_interactive);

		u32 upad_checkupdate=0;
		JSON_ReadConfigField(CnT->json_cfg,"upad_checkupdate",upad_checkupdate);
		if (upad_checkupdate)
			find_update_on_storages=true;

		JSON_ReadConfigField(CnT->json_cfg,"upad_source_remove",ue_params.upad_source_remove);
		JSON_ReadConfigField(CnT->json_cfg,"upad_force",ue_params.upad_force);
		GPRINT(NORMAL_LEVEL,"fw ftype [%s], interactive %d, src remove %d, force %d\n",ue_params.upad_firmware_file.c_str(),ue_params.upad_interactive,ue_params.upad_source_remove,ue_params.upad_force);
		//thread_hung_max_tick

		//
	}

Update_admin::~Update_admin (void){
		GPRINT(HARD_LEVEL,"Destroy Update_admin\n");
	}


eErrorTp Update_admin::Loop(void* thisPtr){

#ifdef DEBUG_UPDATE
	 update_checked=true;
#endif

		ue=make_shared<UpdateEngine>(debug_level,ue_params,this);
//	sleep(10);
	//Customer.ResetDeviceToFactory();
	//printf("CnT->first_run %d\n",CnT->first_run);
	while(skey){
			//GPRINT(NORMAL_LEVEL,"LOOP\n");
			//if (jnoda_ready)
				//GPRINT(NORMAL_LEVEL,"CnT->first_run %d\n",CnT->first_run);
		    if ((jnoda_ready)&&(CnT->first_run)){
		    	GPRINT(MEDIUM_LEVEL,"jnoda_ready and CnT->first_run\n");
		    	if (update_checked==false){
		    		//SendEventFromCnodaToUI(string("wait"));
		    		GPRINT(MEDIUM_LEVEL,"update_checked==false\n");
		    		if (ue->CheckAndUpdate()==ERROR){
		    			if (find_update_on_storages)
		    				ue->FindAndPrepareUpdate();

		    			if (ue->CheckAndUpdate()==ERROR){
		    				SendSystemFromCnodaToUI(UPDPREP_EVENT_UPD_HIDE_SYSTEM_BLOCK);
		    			}
		    		}
		    		//SendEventFromCnodaToUI(string("ready"));
		    		update_checked=true;
		    	}
		    }
		    //exit(1);
			while (GetUcastFifoMessage(FifoPreBuf,Mheader)!=ERROR)
			{
					switch(Mheader.MsgType)
					{
						case srvmJNODA_READY:

							jnoda_ready=true;
						break;
						case srvmWEB_CLIENT_ACT:

							GPRINT(NORMAL_LEVEL,"got srvmWEB_CLIENT_ACT %s\n",FifoPreBuf.p());
							if (ue->CheckUpdateMarker()==NO_ERROR){
								GPRINT(NORMAL_LEVEL,"Found updater marker\n");
								ue->RemoveUpdateMarker();
								GPRINT(NORMAL_LEVEL,"Updater marker is remove\n");
							}
							//SendSystemFromCnodaToUI(UPDPREP_EVENT_UPD_COMPLETE);
							//SendSystemFromCnodaToUI(UPDPREP_EVENT_UPD_HIDE_SYSTEM_BLOCK);
							break;
						case srvmCHECK_STOR_FOR_UPDATE:
							ue->FindAndPrepareUpdate();
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



