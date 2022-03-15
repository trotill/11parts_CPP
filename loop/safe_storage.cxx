/*
 * safe_storage.cxx
 *
 *  Created on: 17 дек. 2018 г.
 *      Author: root
 */

#include "loop/safe_storage.h"
#include "engine/thread.h"

void DumpSAST(void){
	CnT->dnk_var->Dump("",false);
}



Safe_storage::Safe_storage(eDebugTp debug_level):ThreadT("Safe_stor","sast",debug_level){
		GPRINT(HARD_LEVEL,"Create Safe_storage\n");
		JSON_ReadConfigField(CnT->json_cfg,"sast_loopdelay",loopdelay);
		JSON_ReadConfigField(CnT->json_cfg,"sast_syncinterval",syncinterval);
		JSON_ReadConfigField(CnT->json_cfg,"sast_saveinterval",saveinterval);
		JSON_ReadConfigField(CnT->json_cfg,"sast_blockRebootRollout",sast_blockRebootRollout);

		Sm.InitGset();
		Customer.Init_SAST(&Sm);
		initSignSettings();
	}



eErrorTp Safe_storage::initSignSettings(){
	for (string & cfgName:Customer.signedConfigs){
		CnT->signedConfigs.push_back(cfgName);
		Sm.checkSignSetting(cfgName);
		//Customer.signedConfigs[n]
		//Customer.signedConfigs[n];
	}
	return NO_ERROR;
}

Safe_storage::~Safe_storage (void){
		GPRINT(HARD_LEVEL,"Destroy Safe_storage\n");
	}

eErrorTp Safe_storage::CompareAndSaveSettings(SettingsAdm * Sm)
{
	GPRINT(MEDIUM_LEVEL,"CompareAndSaveSettings\n");
	CnT->dnk_var->SAST_SaveChanged_Settings(CnT->sast_db);
	return Customer.SyncSettings_SAST(Sm);//CompareAndSaveSettingsPD(&Sm,OldSaved);
}

eErrorTp Safe_storage::RollOutArchiveConfigs(string json){
	printf("srvmSAST_ROLLOUT_ARCHIVE_CONFIGS [%s]\n",json.c_str());
	 Json::Value data;
	 Json::Reader rd;
	 if (rd.parse(json,data)){
		// if (root.isMember("d")){
			 //Json::Value data=root["d"];
		 zr();
			 if (data.isMember("cadmFname")&&data.isMember("cadmPasswd")&&data.isMember("cadmKeyItems")&&data.isMember("cadmPbkdf2")){
				 zr();
				 string sid=data["sid"].asString();
				 //if (data.isMember("sid")){

				// }
				 //SendEventFromCnodaToUI("wait",sid);
				 zr();
				 SendWaitToJNODA();
				// zr();
				 string key="";
				 string passwd=data["cadmPasswd"].asString();
				 string path;
				 if (data["cadmSetCfg"].isArray())
					path=CnT->DOWNLOAD_PATH+"/"+data["cadmSetCfg"][0].asString();
				 else
					 path=CnT->DOWNLOAD_PATH+"/"+data["cadmSetCfg"].asString();

				 zr();
				 for (u32 n=0;n<data["cadmKeyItems"].size();n++){
					// zr();
					 string sname="settings."+data["cadmKeyItems"][n][0].asString();
					 string sfield=data["cadmKeyItems"][n][1].asString();
					 string svalue;
					 Json::Value JSON;
					 if ((Sm.LoadSetting(JSON,sname)!=ERROR)&&(Sm.GetSettingValue(JSON,sfield,svalue)!=ERROR)){
						// zr();
						 printf("svalue [%s]\n",svalue.c_str());
						 key+=svalue;
					 }
				 }
				 zr();
				 key+=passwd;
				 printf("srvmSAST_ROLLOUT_ARCHIVE_CONFIGS KEY [%s]\n",key.c_str());
				 string pbkdf2="";
				 if (data["cadmPbkdf2"].asString()=="true")
					 pbkdf2="-pbkdf2";

				 string result_file=CnT->CACHE_PATH+"/"+"routconf.result";
				 remove(result_file.c_str());
				// string cmd=sf("openssl enc -aes-256-cbc -d -in \"%s\" %s -k \"%s\" | tar xz -C %s 2>%s",path.c_str(),pbkdf2.c_str(),key.c_str(),sf("%s/../",CnT->SETTING_STORAGE_MOUNTP[0].c_str()).c_str(),result_file.c_str());
				 string cmd=sf("openssl enc -aes-256-cbc -d -in \"%s\" %s -k \"%s\" | tar xz -C / 2>%s",path.c_str(),pbkdf2.c_str(),key.c_str(),result_file.c_str());
				// string cmd=sf("openssl enc -aes-256-cbc -d -in \"%s\" %s -k \"%s\" | tar xz -C /run/ 2>%s",path.c_str(),pbkdf2.c_str(),key.c_str(),result_file.c_str());
				 BashResult(cmd);
				 string result="";
				 string response="";

				 if (ReadStringFile((char*)result_file.c_str(),result)!=ERROR){
					 GPRINT(NORMAL_LEVEL,"error rollout configs result \n[%s]\n",result.c_str());
					 response="ROLLOUT_CFG_ERROR";

				 }
				 if (result.length()==0){
					 GPRINT(NORMAL_LEVEL,"rollout configs ok!!!\n");
					 response="ROLLOUT_CFG_OK";

				 }
				// SendEventFromCnodaToUI("ready",sid);
				 SendReadyToJNODA();
				 Json::Value jsonResp;
				 jsonResp["data"]["msg"]=response;
				 SendEventFromCnodaToUI(FastWriteJSON(jsonResp),sid);
				 remove(result_file.c_str());
				 if ((result.length()==0)){
					 SendSharedFifoMessage(srvmSAST_ROLLOUT_ARCHIVE_READY,"all");
					 sleep(2);
					 if (!sast_blockRebootRollout){
						 GPRINT(NORMAL_LEVEL,"RebootSystem!!!\n");
						 RebootSystem();
					 }
				 }

			 }
		// }

	 }
	return NO_ERROR;
}
eErrorTp Safe_storage::Loop(void* thisPtr){

		//u32 set_sync_cntr=0;
		//u32 set_sync_count=syncinterval;

		//u32 set_saver_cntr=0;
		//u32 set_saver_count=saveinterval;

		LoopTimer CompareAndSave(saveinterval,loopdelay);
		LoopTimer SyncGset(syncinterval,loopdelay);
		//DumpSAST();
		//sSaved OldSaved;
			while(1){

				if (skey==false)
					sleep(20);
				//GPRINT(HARD_LEVEL,"set_saver_cntr %d set_saver_count %d\n",set_saver_cntr,set_saver_count);
				//if (set_saver_cntr>=set_saver_count){
				//			set_saver_cntr=0;
				if (CompareAndSave.alarm())
						CompareAndSaveSettings(&Sm);
				//}
				//else
				//	set_saver_cntr++;

				//GPRINT(HARD_LEVEL,"set_sync_cntr %d set_sync_count %d\n",set_sync_cntr,set_sync_count);
				//if (set_sync_cntr>=set_sync_count){
				//	set_sync_cntr=0;
				if (SyncGset.alarm())
					Sm.SyncGsetSettings(1);

				//}
				//else
				//	set_sync_cntr++;

				while (GetUcastFifoMessage(FifoPreBuf,Mheader)!=ERROR)
				{
					switch(Mheader.MsgType)
					{
						case srvmJNODA_READY:
								jnoda_ready=true;
						break;
						case srvmSAST_FORCE_COMP_SAVE:
							CompareAndSaveSettings(&Sm);
						break;
						case srvmSAST_ROLLOUT_ARCHIVE_CONFIGS:
							RollOutArchiveConfigs((char*)FifoPreBuf.p());
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
			CompareAndSaveSettings(&Sm);
			Sm.SyncGsetSettings(1);
			return NO_ERROR;
		}


