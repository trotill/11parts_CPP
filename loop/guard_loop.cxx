/*
 * guard_loop.cxx
 *
 *  Created on: 27 дек. 2018 г.
 *      Author: root
 */

#include "guard_loop.h"

void Guard_loop::tot_init(void){

	string value="0";
	string tot;
	string tot_file=CnT->CACHE_PATH+"/tot";
	u32 uptime=GetUptime();

	SetValueSAST("x.v.uptime",uptime);
	GetValueSAST("x.v.motohour",saved_motohour);
	GPRINT(NORMAL_LEVEL,"Load motohour in storage %u\n",saved_motohour);


	if (existsSync(tot_file)==NO_ERROR){
		//система уже стартовала, база отсчета определена в файле tot
		string tot_data;
		ReadStringFile((char*)tot_file.c_str(),tot_data);
		motohour_base=stoul(tot_data);

		//motohour=CnT->pvar.project_var.motohour=stoul(tot_data);
		GPRINT(NORMAL_LEVEL,"Load motohour_base from tot file %u\n",motohour_base);
	}
	else{
		motohour_base=saved_motohour;
		string save_base=string_format("%u",motohour_base);
		//printf("write tot_data %s to tot_file %s \n",tot_data.c_str(),tot_file.c_str());
		WriteStringFile((char*)tot_file.c_str(),save_base);
		GPRINT(NORMAL_LEVEL,"Save motohour_base to tot file %u\n",motohour_base);
	}

	if (!gulo_disable_tot){
		source_work_ts=motohour_base+uptime;
		SetValueSAST("x.v.motohour",source_work_ts);
		Customer.SetValue("total_operating_time",to_string(source_work_ts));
	}

	Customer.SetValue("operating_time",to_string(uptime));
}
Guard_loop::Guard_loop(eDebugTp debug_level):ThreadT("Guard_loop","gulo",debug_level){
		GPRINT(HARD_LEVEL,"Create Guard_loop\n");
		JSON_ReadConfigField(CnT->json_cfg,"gulo_loopdelay",loopdelay);
		JSON_ReadConfigField(CnT->json_cfg,"gulo_tempinterval",tempinterval);
		JSON_ReadConfigField(CnT->json_cfg,"gulo_disable_tot",gulo_disable_tot);
		JSON_ReadConfigField(CnT->json_cfg,"gulo_totinterval",totinterval);
		JSON_ReadConfigField(CnT->json_cfg,"gulo_cpu_temp_sysfs_path",cpu_temp_sysfs_path);

		extStorageDev[0]="/dev/sda";
		extStorageDev[1]="/dev/sda1";

		JSON_ReadConfigField(CnT->json_cfg,"gulo_extstorage_dev",extStorageDev);//Детект и монтирование внешних USB
		JSON_ReadConfigField(CnT->json_cfg,"gulo_extstorage_mountpoint",extStorageMountPoint);
		JSON_ReadConfigField(CnT->json_cfg,"gulo_extstorage_enable",extStorageEnable);
		JSON_ReadConfigField(CnT->json_cfg,"gulo_extstorage_automount",doMount);

		if (extStorageEnable){
			for (u8 n=0;n<extStorageDev.size();n++)//Принудительное размонтирование внешних накопителей
				Umount ((char*)extStorageDev[n].asCString());
		}

		if (existsSync(cpu_temp_sysfs_path.c_str())==ERROR){
			string tp="/sys/class/hwmon/hwmon0/temp1_input";
			if (existsSync(tp)==NO_ERROR){
				cpu_temp_sysfs_path=tp;
			}
			else{
				tp="/sys/class/thermal/thermal_zone0/temp";
				if (existsSync(tp)==NO_ERROR){
					cpu_temp_sysfs_path=tp;
				}
			}
		}
		tot_init();

		if (CnT->dinfo.webJnodaUsePrivate){
			//string fpath=CnT->PRIVATE_PATH;//string_format("%s%s",CnT->CACHE_PATH.c_str(),"/private");
			if (existsSync(CnT->PRIVATE_PATH)==ERROR){
				CnT->reqPrivate=true;
			}
		}
		//cpu_temp_sysfs_path

	}

Guard_loop::~Guard_loop (void){
		GPRINT(HARD_LEVEL,"Destroy Guard_loop\n");
	}

u32 Guard_loop::GetUptime(void)
		{
			string res=ExecResult("cat","/proc/uptime");
			if (res!=""){
				size_t pos=res.find(".");
				string s = res.substr (0,pos);
				uptime=stoul(s);
			}

			return uptime;
		}

eErrorTp Guard_loop::GetTemperature(void)
		{
			string value=ExecResult("cat",(char*)cpu_temp_sysfs_path.c_str());
			if ((value!="")&&(CnT->pvar.project_var.temperature!=value))
				Customer.SetValue("cpu_temp",value);
			return NO_ERROR;
		}

eErrorTp Guard_loop::CalcWorkTime(bool force)
{
			//TIME_T stm=time(NULL);
			u32 uptime=GetUptime();
			//if (stm>tot_time)
			//{

				source_work_ts=uptime+motohour_base;
				//printf("\n\n\nCalcWorkTime %d\n\n\n",source_work_ts);
				if (force){
					if (!gulo_disable_tot)
						SetValueSAST("x.v.motohour",source_work_ts);

					SetValueSAST("x.v.uptime",uptime);

					if (!gulo_disable_tot)
						Customer.SetValue("total_operating_time",to_string(source_work_ts));

					Customer.SetValue("operating_time",to_string(uptime));
					return NO_ERROR;
				}

				//printf("source_work_ts %d old_source_work_ts %d\n",source_work_ts,old_source_work_ts);
				if (old_source_work_ts!=source_work_ts)
				{
					//if ((source_work_ts%3600)==0)
					//if ((source_work_ts%3600)==0)
				//	{
					if (!gulo_disable_tot)
						SetValueSAST("x.v.motohour",source_work_ts);

					SetValueSAST("x.v.uptime",uptime);
						//CnT->pvar.project_var.motohour=source_work_ts;
					//}
					//if ((source_work_ts%5)==0)
					//{
						//printf("source_work_ts %d\n",source_work_ts);
					if (!gulo_disable_tot)
						Customer.SetValue("total_operating_time",to_string(source_work_ts));

					Customer.SetValue("operating_time",to_string(uptime));
					//}
					old_source_work_ts=source_work_ts;
				}
			//}
			return NO_ERROR;
}

eErrorTp Guard_loop::extUmount(){
	if (extStorageEnable==0)
		return NO_ERROR;

	sStorageInfo storage;
	Json::Value msg;
	if (extStorageMount==1){
		GetStorageInfo(&storage,(char*)extStorageDevMounted.c_str(),(char*)extStorageMountPoint.c_str());
		if ((storage.IsPresent)&&(storage.IsMount)){
			Umount (extStorageDevMounted.c_str());
			msg["dev"]=extStorageDevMounted;
			msg["stat"]="umount";
			msg["err"]="";
			msg["path"]=extStorageMountPoint;
			msg["FreeSizeMB"]=storage.FreeSizeMB;
			msg["PercentageOfUse"]=storage.PercentageOfUse;
			msg["TotalSizeMB"]=storage.TotalSizeMB;
			extStorageMount=0;
			SendUserWebEventFromCnodaToUI("extStor",msg);
			return NO_ERROR;
		}
	}

	extStorageMount=0;
	msg["dev"]=extStorageDevMounted;
	msg["stat"]="umount";
	msg["err"]="noUmount";
	msg["path"]=extStorageMountPoint;
	SendUserWebEventFromCnodaToUI("extStor",msg);
	return ERROR;
}
eErrorTp Guard_loop::extStorageAdmin(){
	if (extStorageEnable==0)
		return NO_ERROR;
	Json::Value msg;
	sStorageInfo storage;
	if (doMount==0){
		//Если монтирование накопителя не требуется, отправляется инф. о его наличии
		//doMount может быть 0 только после отправки команды на размонтирование, т.е. к этому моменту, накопитель должен быть выбран
		GetStorageInfo(&storage,extStorageDevMounted.c_str(),extStorageMountPoint.c_str());
		msg["err"]="";
		msg["dev"]=extStorageDevMounted;
		msg["path"]=extStorageMountPoint;
		if ((storage.IsPresent)){
			if (storage.IsMount){
				msg["stat"]="mount";
				msg["FreeSizeMB"]=storage.FreeSizeMB;
				msg["PercentageOfUse"]=storage.PercentageOfUse;
				msg["TotalSizeMB"]=storage.TotalSizeMB;
			}
			else{
				msg["stat"]="detect";
			}
		}
		else{
			msg["stat"]="nodetect";
		}
		SendUserWebEventFromCnodaToUI("extStor",msg);
		return NO_ERROR;
	}


	//zr();

	if (extStorageMount==1){
		GetStorageInfo(&storage,extStorageDevMounted.c_str(),extStorageMountPoint.c_str());
		//printf("Check extStorageDevMounted %s extStorageMountPoint %s extStorageMount %d pres %d mount %d\n",extStorageDevMounted.c_str(),extStorageMountPoint.c_str(),extStorageMount,storage.IsPresent,storage.IsMount);
		if ((storage.IsPresent)&&(storage.IsMount)){
			//sStorageInfo * storage,char * MountDev,char * MountPoint
			//zr();
			msg["dev"]=extStorageDevMounted;
			msg["stat"]="mount";
			msg["err"]="";
			msg["path"]=extStorageMountPoint;
			msg["FreeSizeMB"]=storage.FreeSizeMB;
			msg["PercentageOfUse"]=storage.PercentageOfUse;
			msg["TotalSizeMB"]=storage.TotalSizeMB;
			SendUserWebEventFromCnodaToUI("extStor",msg);
			return NO_ERROR;
		}
	}

	//else
	//	GPRINT(NORMAL_LEVEL,"Detect error, remove mounted storage %s\n",extStorageDevMounted.c_str());

	if ((extStorageMount==1)&&((storage.IsPresent==0)||(storage.IsMount==0))){
		//zr();
		extStorageMount=0;
		Umount ((char*)extStorageDevMounted.c_str());
		GPRINT(MEDIUM_LEVEL,"extStorage dev %s fell off, force remount\n",extStorageDevMounted.c_str());
	}

	bool needMsg=false;
	for (u32 n=0;n<extStorageDev.size();n++){
		GetStorageInfo(&storage,extStorageDev[n].asCString(),extStorageMountPoint.c_str());
		//zr();
	//	GPRINT(NORMAL_LEVEL,"extStorageDev %s MP %s pres %d mnt %d PercentageOfUse %d TotalSizeMB %d FreeSizeMB %d extStorageMount %d\n",extStorageDev[n].asCString(),extStorageMountPoint.c_str(),storage.IsPresent,storage.IsMount,storage.PercentageOfUse,storage.TotalSizeMB,storage.FreeSizeMB,extStorageMount);
		if ((storage.IsPresent==1)&&(storage.IsMount==0)&&(extStorageMount==0)){
			//zr();

			extStorageFound=1;
			GPRINT(MEDIUM_LEVEL,"Found external dev %s\n",extStorageDev[n].asCString());

			msg["dev"]=extStorageDev[n].asString();
			msg["stat"]="detect";
			msg["err"]="";
			msg["path"]="";
			if ((extStorageFound==1))
				SendUserWebEventFromCnodaToUI("extStor",msg);

			if (existsDir(extStorageMountPoint.c_str())==ERROR){
				GPRINT(MEDIUM_LEVEL,"Make path for mount ext Storage %s\n",extStorageMountPoint.c_str());
				if (MkPath(extStorageMountPoint.c_str(), 0xffffffff)==ERROR){
					printf("Critical error, storage %s not create, please reconfigure system\n",extStorageMountPoint.c_str());
					exit(1);
				}
			}

			msg["path"]=extStorageMountPoint;


			if (extStorageMount==0){
				needMsg=true;
			//	printf("Try mount %s to %s\n",extStorageDev[n].asCString(),extStorageMountPoint.c_str());
				if (Mount(extStorageDev[n].asCString(),extStorageMountPoint.c_str(),NULL,0,"")==NO_ERROR){
					GetStorageInfo(&storage,extStorageDev[n].asCString(),extStorageMountPoint.c_str());
					msg["stat"]="mount";
					msg["err"]="";
					msg["FreeSizeMB"]=storage.FreeSizeMB;
					msg["PercentageOfUse"]=storage.PercentageOfUse;
					msg["TotalSizeMB"]=storage.TotalSizeMB;
					extStorageDevMounted=extStorageDev[n].asString();
					GPRINT(MEDIUM_LEVEL,"Storage %s to %s mount success\n",extStorageDevMounted.c_str(),extStorageMountPoint.c_str());

					extStorageMount=1;
					//SendUserWebEventFromCnodaToUI("extStor",msg);//найдено и примонтировано
					break;
				}
				else{
					GPRINT(MEDIUM_LEVEL,"Storage %s to %s mount error\n",extStorageDev[n].asCString(),extStorageMountPoint.c_str());
					msg["err"]="noMount";
					msg["stat"]="umount";
					//найдено и не примонтировано
				}
			}

		}


		if (storage.IsPresent==0){
			//zr();
			if (extStorageFound){
				//zr();
				extStorageFound=0;
				needMsg=true;
				msg["dev"]=extStorageDevMounted.c_str();
				msg["stat"]="nodetect";
				msg["err"]="";
				msg["path"]=extStorageMountPoint;
				if (extStorageMount){
					msg["err"]="noUmount";
					msg["path"]="";
					GPRINT(MEDIUM_LEVEL,"Storage %s remove with error\n",extStorageDevMounted.c_str());
				}
				else
					GPRINT(MEDIUM_LEVEL,"Storage %s remove\n",extStorageDevMounted.c_str());
				//SendUserWebEventFromCnodaToUI("extStor",msg);//извлечено с ошибкой или без
				extStorageMount=0;
				break;
			}
		}
	}

	if (needMsg){
		//Отправка сообщения примонтировано или нет
		SendUserWebEventFromCnodaToUI("extStor",msg);
	}
	else{
		//Отправка сообщения не вставлен накопитель
		msg["dev"]="";
		msg["stat"]="nodetect";
		msg["err"]="";
		msg["path"]=extStorageMountPoint;
		SendUserWebEventFromCnodaToUI("extStor",msg);
	}

	return NO_ERROR;
}
eErrorTp Guard_loop::Loop(void* thisPtr){
		//sSaved OldSaved;
	//u32 delay_max=1000/loopdelay;
	u32 delay_cnt=0;
	//u32 temp_delay_max=2000/loopdelay;
	u32 temp_delay_cnt=0;
	LoopTimer totint(totinterval,loopdelay);
	LoopTimer tempint(tempinterval,loopdelay);
	RTC_Timer storageAdmRtc(2000);
	RTC_Timer sharedAlgoRtc(1000);
	//string tot_file=CnT->CACHE_PATH+"/tot";
	u32 device_reboot_cnt=0;
	u32 server_reboot_cnt=0;
	u32 uptime=GetUptime();

	//if (uptime<5*60){
	if (CnT->first_run){
		GetValueSAST("x.v.device_reboot_cnt",device_reboot_cnt);
		device_reboot_cnt++;
		SetValueSAST("x.v.device_reboot_cnt",device_reboot_cnt);
		//printf("device_reboot_cnt %d\n",device_reboot_cnt);
	}

	GetValueSAST("x.v.server_reboot_cnt",server_reboot_cnt);
	server_reboot_cnt++;
	SetValueSAST("x.v.server_reboot_cnt",server_reboot_cnt);

	SendSharedFifoMessage(srvmSAST_FORCE_COMP_SAVE,"sast",NULL,0);
	SendRestartClusterToJNODA();
	while(skey){
		if (totint.alarm())
				CalcWorkTime(false);

		if (tempint.alarm()){
			GetTemperature();
		}
		if (sharedAlgoRtc.alarm()){
			//zr();
			if (CnT->authorised==false){//если проблемы с ключем или сигнатурами, слать мессаж на его переустановку
				//zr();
				activationMessageNeedLic();
			}
			if (CnT->reqPrivate){//если не найден приват, отправить запрос на его установку
				Json::Value needPrivate;
				needPrivate["activation"]["stat"]="needPrivate";
				SendSystemFromCnodaToUI(FastWriteJSON(needPrivate));
			}
		}

		//extStorageDev
		//GetStorageInfo(sStorageInfo * storage,char * MountDev,char * MountPoint)
		//GPRINT(NORMAL_LEVEL,"LOOP\n");
		if (storageAdmRtc.alarm())
			extStorageAdmin();

		while (GetUcastFifoMessage(FifoPreBuf,Mheader)!=ERROR)
		{
			switch(Mheader.MsgType)
			{
				case srvmEXT_STOR:{
					Json::Value extStor=JSON_parse(FifoPreBuf.p());
					if (extStor["cmd"]=="mount"){
						doMount=1;
					}
					else{
						doMount=0;
						extUmount();
					}
				}
				break;

				case srvmJNODA_READY:
					//GPRINT(NORMAL_LEVEL,"srvmJNODA_READY\n");
				{
						jnoda_ready=true;
						if (totinterval!=0) CalcWorkTime(true);
						if (tempinterval!=0) GetTemperature();
						Customer.SetValue("total_operating_time",to_string(source_work_ts));
				}
				break;
				case srvmMQ_MQTT_READY:
					Customer.SetValue("total_operating_time",to_string(source_work_ts));
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
	//WriteStringFile(tot_file.c_str(),source_work_ts);
	if (!gulo_disable_tot){
		source_work_ts=GetUptime()+motohour_base;
		if (source_work_ts!=0)
		  Sm.ChangeSettingValue("settings.total_operating_time","tot",source_work_ts);
	}
	return NO_ERROR;
}


