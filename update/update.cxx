/*
 * update.cxx
 *
 *  Created on: 19 дек. 2018 г.
 *      Author: root
 */


#include "update.h"
#include "engine/proto/json_proto.h"
#include "engine/global.h"
#include "engine/lib/11p_string.h"
#include "engine/lib/11p_process.h"
#include "engine/lib/11p_files.h"
#include "engine/lib/11p_bin.h"
#include "engine/minisw/minisw.h"


//char PassportDataJSON[PASSPORT_MAX_SIZE];

UpdateEngine::UpdateEngine(eDebugTp debug_level,UpdateEngine_params & params,FifoEngineT * msg_sender){
		this->debug_level=debug_level;
		SourceStr="UpdateEngine";
		upad_firmware_file=params.upad_firmware_file;
		upad_force=params.upad_force;
		upad_interactive=params.upad_interactive;
		upad_source_remove=params.upad_source_remove;
		upad_storages=params.upad_storages;
		upad_storages_str=params.upad_storages_str;
		if (msg_sender!=NULL)
			msender=msg_sender;
		//tpSendSystemFCToUI SendSystemFromCnodaToUI_upd;

		//SendSystemFromCnodaToUI_upd=SystemFromCnodaToUI;
};

eErrorTp UpdateEngine::SendSystemFromCnodaToUI_upd(string str){
	if (msender==NULL){
		return SendSystemFromCnodaToUI(str);
	}
	else{
		return msender->SendSystemFromCnodaToUI(str);
	}
}
eErrorTp UpdateEngine::GetImagePassport(string fname, u32 offset,char * result,u32 result_size)
{
	std::ifstream  file;

	//GPRINT(NORMAL_LEVEL,"fname %s\n",fname.c_str());
	file.open(fname);
	if (file.is_open())
	{
		file.seekg(offset);
		file.read(result,result_size);
		//GPRINT(NORMAL_LEVEL,"PassportDataRaw %s\n",PassportDataRaw);
		if ((!file)||file.fail())
		{
			file.close();
			GPRINT(NORMAL_LEVEL,"Error read file [%s]\n",fname.c_str());
			return ERROR;
		}
		file.close();

		//printhex(PassportDataRaw,128,16);
		//GPRINT(NORMAL_LEVEL,"Pdata %s\n",result);
		if (result[0]=='{'){
			GPRINT(NORMAL_LEVEL,"Pdata %s\n",result);
			return NO_ERROR;
		}
		//if (result[0]=='{')
	//	{
		//	strncpy(PassportDataJSON,result,result_size);
		//	return NO_ERROR;
			//GPRINT(NORMAL_LEVEL,"Pdata %s\n",PassportDataJSON);
		//}

		//printhex(PassportDataJSON,128,16);


	}

	return ERROR;

}
eErrorTp UpdateEngine::FIT_UBIx2_RWx1_backup_passport(char * PassportDataRawSave,string fw,u32 offset)
{
	std::fstream  file(fw.c_str(),ios::binary | ios::out | ios::in );
	//memcpy(PassportDataRawSave,PassportDataRaw,PASSPORT_MAX_SIZE);
	if (file.is_open())
	{
		file.seekp(offset,std::ios_base::beg);
		//memset(PassportDataRaw,0,PASSPORT_MAX_SIZE);
		file.write((const char *)PassportDataRawSave,PASSPORT_MAX_SIZE);
		if ((!file)||file.fail())
		{
			file.close();
			GPRINT(NORMAL_LEVEL,"Error write file [%s]\n",fw.c_str());
			return ERROR;
		}
		file.flush();
		file.close();

		return NO_ERROR;
	}

	return ERROR;
}

eErrorTp UpdateEngine::FIT_UBIx2_RWx1_fill_zero(char * PassportDataRawSave,char* PassportDataRaw,string fw,u32 offset)
{
	std::fstream  file(fw.c_str(),ios::binary | ios::out | ios::in );
	memcpy(PassportDataRawSave,PassportDataRaw,PASSPORT_MAX_SIZE);
	if (file.is_open())
	{
		file.seekp(offset,std::ios_base::beg);
		memset(PassportDataRaw,0,PASSPORT_MAX_SIZE);
		file.write((const char *)PassportDataRaw,PASSPORT_MAX_SIZE);
		if ((!file)||file.fail())
		{
			file.close();
			GPRINT(NORMAL_LEVEL,"Error write file [%s]\n",fw.c_str());
			return ERROR;
		}
		file.flush();
		file.close();

		return NO_ERROR;
	}

	return ERROR;
}



eErrorTp UpdateEngine::SavePassport(string dfold,char * passport,string passportname)
{
	dfold+='/';
	string fname=dfold+passportname;
	std::ofstream  file(fname.c_str(),ios::binary);
	if (file.is_open())
	{
		file.write((const char *)passport,strlen(passport));
		if ((!file)||file.fail())
		{
			file.close();
			GPRINT(NORMAL_LEVEL,"Error write file [%s]\n",fname.c_str());
			return ERROR;
		}
		GPRINT(NORMAL_LEVEL,"Create passport %s\n",fname.c_str());
	}
	return NO_ERROR;
}

eErrorTp UpdateEngine::SaveFinger(string dfold,string finger)
{
	dfold+='/';
	string fname=dfold+finger;
	std::ofstream  file(fname.c_str(),ios::binary);
	if (file.is_open())
	{
		file.write((const char *)finger.c_str(),strlen(finger.c_str()));
		if ((!file)||file.fail())
		{
			file.close();
			GPRINT(NORMAL_LEVEL,"Error write file [%s]\n",fname.c_str());
			return ERROR;
		}
		GPRINT(NORMAL_LEVEL,"Create finger %s\n",fname.c_str());
	}
	return NO_ERROR;
}

eErrorTp DoUpdate(char * json) {

	printf("Do update %s\n",json);
	Json::Value root;
	Json::Reader reader;
	eErrorTp err=ERROR;
	string fw;
	string str;
	bool parsingSuccessful = reader.parse( json, root );
	if (parsingSuccessful)
	{
		if (root[JSON_DATA_SYMB].isObject()&&(root[JSON_DATA_SYMB][JSON_FIRMWARE_FIELD_NAME].isNull()==false))
		{
			fw=root[JSON_DATA_SYMB][JSON_FIRMWARE_FIELD_NAME].asString();
			printf("imcheck file %s\n",fw.c_str());
			str=string_format("%s/imcheck %s/%s %s %s",CnT->CNODA_PATH.c_str(),CnT->DOWNLOAD_PATH.c_str(),fw.c_str(),CnT->FIRMWARE_PATH.c_str(),CnT->FIRMWARE_NAME.c_str());
			if (system(str.c_str())==0)
			{
				printf("Update checked and ready!!!\n");
				err=NO_ERROR;
			}
			else
				printf("Error check update %s %s\n",fw.c_str(),str.c_str());
		}
	}

	return err;
}


eErrorTp UpdateImageStd(string & json_req,string & json_resp){
	eErrorTp err=ERROR;
	if (DoUpdate((char*)json_req.c_str())==ERROR){
		json_resp="update_err";
	}
	else{
		json_resp="update_ok";
		err=NO_ERROR;
		SendSystemFromCnodaToUI(UPDPREP_EVENT_UPD_REBOOT);
		RebootSystem();
	}
	return err;
}

//Копирует исходный файл в папку update и удаляет u_fit
eErrorTp UpdateEngine::FIT_ROOTx2_RWx1_Finalize(Update_info & passport,bool AllowCopyFw,bool remove_source){

	string dest_file=CnT->FIRMWARE_PATH+'/'+CnT->FIRMWARE_NAME;
	if (passport.passport["rootcompression"].isNull()==false){
		dest_file+='.'+passport.passport["rootcompression"].asString();
	}

	auto frm=[&](string & file){
		if (remove_source){
			remove((char*)file.c_str());
			GPRINT(NORMAL_LEVEL,"Force remove %s\n",file.c_str());
		}
		else
			GPRINT(NORMAL_LEVEL,"Skip remove %s\n",file.c_str());

		return NO_ERROR;
	};

	if (rename (passport.source_file.c_str(), dest_file.c_str())!=0){
		 if (AllowCopyFw){
			 if (CopyFile((char*)passport.source_file.c_str(), (char*)dest_file.c_str())==ERROR){
				 frm(passport.source_file);
				 return ERROR;
			 }
			 else
				 GPRINT(NORMAL_LEVEL,"fw copy %s to %s\n",passport.source_file.c_str(),dest_file.c_str());
		}
		else
			 GPRINT(NORMAL_LEVEL,"skip copy %s to %s, copying is prohibited\n",passport.source_file.c_str(),dest_file.c_str());
	}
	else
		 GPRINT(NORMAL_LEVEL,"fw rename %s to %s\n",passport.source_file.c_str(),dest_file.c_str());

	if ((passport.passport["stortype"].isNull()==false)&&(passport.passport["ufit_ro"].isNull()==false))
	{
		string cmd;
		GPRINT(NORMAL_LEVEL,"Erase fit parts in %s!!!\n",passport.passport["ufit_ro"].asString().c_str());
		if (passport.passport["stortype"].asString().compare("nand")==0){
			cmd=string_format("flash_erase %s 0 0",passport.passport["ufit_ro"].asString().c_str());
		}
		else{
			cmd=string_format("dd if=/dev/zero of=%s bs=%d count=%d",passport.passport["ufit_ro"].asString().c_str(),MB,16);
		}

		BashResult(cmd);
	}

	frm(passport.source_file);
	return NO_ERROR;
}


eErrorTp UpdateEngine::Finalize(Update_info & passport,Update_Finalize & params){

	eErrorTp err=NO_ERROR;
	if (passport.passport["updtype"]=="FIT_ROOTx2_RWx1"){
	//	params.dest_file=dest_file;

		if (passport.passport["ushared_rw"].isNull()==false){
			err=CheckFreeSpaceInStorageForFile((char*)passport.source_file.c_str(),(char*)CnT->FIRMWARE_PATH.c_str());
			GPRINT(NORMAL_LEVEL,"Check freespace in storage %s for fw - %s \n",passport.passport["ushared_rw"].asCString(),(err==NO_ERROR)?"okay":"error");
		}
		//if (err==ERROR)
			//return ERROR;

		//if (err==NO_ERROR)
		bool allow_copy=false;
		if (err==NO_ERROR)
			allow_copy=true;

		return FIT_ROOTx2_RWx1_Finalize(passport,allow_copy,params.remove_source);
		//else
			//return ERROR;
	}
	return ERROR;
}

eErrorTp UpdateEngine::Finalize_update(Update_info & updinfo){
		Update_Finalize params;
		GPRINT(NORMAL_LEVEL,"Do finalize\n");

		return Finalize(updinfo,params);
	}
//return FLAGS
//Если mode remove - то записать файл в случае успеха в dimagename и удалить исходный файл в конце функции
//Если mode noremove - то не записывать файл прошивки и не удалять исходный файл
u32 UpdateEngine::Update_fw_check(string & file,string & dfolder,Update_info & passport){
	//bool incorrect_passport=false;
	//bool incorrect_image=false;
	Json::Value root;
	u32 ret_flags=0;
	//eErrorTp err=ERROR;
		Json::Reader reader;
		string finger_type="";
		string finger="";
		string updtype="";
		//eErrorTp err=ERROR;
		//GPRINT(NORMAL_LEVEL,"m0\n");

		//GPRINT(NORMAL_LEVEL,"m1\n");
		passport.source_file=file;
		if (GetImagePassport(file,PASSPORT_OFFSET_VAR1,PassportDataRaw,sizeof(PassportDataRaw))==NO_ERROR)
		{
			//GPRINT(NORMAL_LEVEL,"pasport [%s]\n",PassportDataJSON);
			bool parsingSuccessful = reader.parse( PassportDataRaw, root );
			if (parsingSuccessful)
				{	if (root["finger_type"].isNull()==false) finger_type=root["finger_type"].asString();
					if (root["finger"].isNull()==false) finger=root["finger"].asString();
					if (root["updtype"].isNull()==false) updtype=root["updtype"].asString();

					passport.passport=root;
					passport.passport_str=PassportDataRaw;

					if (finger_type=="md5")
					{
						if (updtype=="FIT_ROOTx2_RWx1")
						{
							string full_fingerout=ExecResult((char*)"md5sum",(char*)file.c_str());
							string full_finger=full_fingerout.substr(0,full_fingerout.find(" "));

							FIT_UBIx2_RWx1_fill_zero(PassportDataRawSave,PassportDataRaw,file,PASSPORT_OFFSET_VAR1);

							string sumout=ExecResult((char*)"md5sum",(char*)file.c_str());
							string sum=sumout.substr(0,sumout.find(" "));

							FIT_UBIx2_RWx1_backup_passport(PassportDataRawSave,file,PASSPORT_OFFSET_VAR1);

							GPRINT(NORMAL_LEVEL,"passport finger %s calc sum [file %s] %s\n",finger.c_str(),file.c_str(),sum.c_str());
							if (sum.compare(finger.c_str())==0)
							{
								try {
									GPRINT(NORMAL_LEVEL,"Ok, md5 has coincided!!!\n");

									if (SavePassport(dfolder,PassportDataRaw,"passport")==ERROR)
										throw 1;

									if (SaveFinger(dfolder,full_finger)==ERROR)
										throw 1;

									if (SaveFinger(dfolder,finger)==ERROR)
										throw 1;
									if ((root["fitfinger"].isNull()==true)||(SaveFinger(dfolder,root["fitfinger"].asString())==ERROR))
										throw 2;

									if ((root["rootfinger"].isNull()==true)||(SaveFinger(dfolder,root["rootfinger"].asString())==ERROR))
										throw 3;


									//return ret_flags;

								}
								catch (int a)
									{
										 if (a==1){
											 GPRINT(NORMAL_LEVEL,"Fault, error save finger\n");
										 }
										 if (a==2){
											 GPRINT(NORMAL_LEVEL,"Fault, not found addition fitfinger or error save fitfinger\n");
										 }
										 if (a==3){
											 GPRINT(NORMAL_LEVEL,"Fault, not found addition rootfinger or error save rootfinger\n");
										 }
										ret_flags|=UPDATE_ERROR_INCORRECT_DESTINATION;

									}

							}

							else{
								ret_flags|=UPDATE_ERROR_INCORRECT_IMAGE;
								//Если образ имеет паспорт но он битый, то его принудительно удалить
							}
						}
						else
							ret_flags|=UPDATE_ERROR_INCORRECT_PASSPORT;
							//GPRINT(NORMAL_LEVEL,"Failt, m5 not coincided, calc sum != passport sum %s != %s\n",sum.c_str(),finger.c_str());
					}
					else
						ret_flags|=UPDATE_ERROR_INCORRECT_PASSPORT;
					// GPRINT(NORMAL_LEVEL,"Undef update algo!!!");
				}
			else
				ret_flags|=UPDATE_ERROR_INCORRECT_PASSPORT;
				//GPRINT(NORMAL_LEVEL,"Error: pasport is broken!!!\n");
		}
		else{
			GPRINT(NORMAL_LEVEL,"Not found passport\n");
			ret_flags|=UPDATE_ERROR_INCORRECT_PASSPORT;
		}
			//incorrect_passport=true;
		//else
			//GPRINT(NORMAL_LEVEL,"Error: pasport not found!!!\n");

		//argv[1],argv[2]
		//если паспорт не верный, значит это другой образ или просто файл, ничего с ним не делать если стоит force_remove_source_image==false
		//если паспорт верный, значит это образ, если он битый то принудительно удалить его


		//if (ret_flags==0)
			//GPRINT(NORMAL_LEVEL,"Succes, firmware ready for update!!!\n");

		passport.status=ret_flags;

		return ret_flags;
}

eErrorTp UpdateEngine::UpdatePlaceStump(void){
	string undel_tmp_dir=CnT->FIRMWARE_PATH+'/'+string(UPDATE_STUMP);
	ofstream ofs(undel_tmp_dir);
	Json::Value stamp_root;
	//Json::FastWriter writer;
	stamp_root["time"]=(u32)TIME((u32*)NULL);
	string sr_str=FastWriteJSON(stamp_root);
	ofs << sr_str;

	return NO_ERROR;
}

//Копирует файлы из списка undelete.set в shared
eErrorTp UpdateEngine::UpdateBackupSettings(Json::Value & passport){
	eErrorTp err=NO_ERROR;
	string undel_file_list=CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]+'/'+string(UNDELETE_FILE);
	string parsed=StyledWriteJSON(passport);
	GPRINT(NORMAL_LEVEL,"UpdateBackupSettings %s\n",(char*)parsed.c_str());
	zr();
	if (passport.isMember("undelete")){
		zr();
		string undelete_data=FastWriteJSON(passport["undelete"]);
		zr();
		GPRINT(NORMAL_LEVEL,"Found undelete list in passport, overlay with new data %s\n",(char*)undelete_data.c_str());
		zr();
		undel_file_list=CnT->CACHE_PATH+"/undelete.passp";
		zr();
		WriteStringFile((char*)undel_file_list.c_str(),undelete_data);
		zr();
	}

	zr();
	string undel_tmp_dir=CnT->FIRMWARE_PATH+'/'+string(UNDELETE_FILE);
	string undel_file_list_dst=undel_tmp_dir+'/'+string(UNDELETE_FILE);
	zr();

	SendSystemFromCnodaToUI_upd(UPDATE_EVENT_UPD_BACKUP_SETT);
	zr();
	GPRINT(NORMAL_LEVEL,"Run backup settings\n");
	zr();
	if (MkPath(undel_tmp_dir.c_str(),0xffffffff)==ERROR)
	{
		GPRINT(NORMAL_LEVEL,"error mkpath %s\n",(char*)undel_tmp_dir.c_str());
		return ERROR;
	}
	if (existsSync(undel_file_list)==NO_ERROR){
		if (CopyFile((char*)undel_file_list.c_str(),(char*)undel_file_list_dst.c_str())!=ERROR){
			Json::Value undel_root;
			Json::CharReaderBuilder reader;
			std::ifstream config_doc(undel_file_list_dst, std::ifstream::binary);
			std::string errs;
			if (Json::parseFromStream(reader, config_doc, &undel_root, &errs)){
				for (u16 n=0;n<undel_root.size();n++){
					string sname=CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]+'/'+undel_root[n].asString();
					string dname=undel_tmp_dir+'/'+undel_root[n].asString();

					if ((existsSync(sname)==NO_ERROR)&&(CopyFile((char*)sname.c_str(),(char*)dname.c_str())==ERROR)){
						GPRINT(NORMAL_LEVEL,"error copy undel file [%s] to [%s]\n",(char*)sname.c_str(),(char*)dname.c_str());
						err=ERROR;
					}
					else
						GPRINT(NORMAL_LEVEL,"backup %s\n",(char*)sname.c_str());
				}
			}
			else{
				GPRINT(NORMAL_LEVEL,"error parse undel file [%s]\n",(char*)errs.c_str());
				err=ERROR;
			}
		}
		else{
			GPRINT(NORMAL_LEVEL,"error backup undel file\n");
			err=ERROR;
		}

	}
	else{
		GPRINT(NORMAL_LEVEL,"undel file not exist %s\n",(char*)undel_file_list.c_str());
	}
	return err;
}

eErrorTp UpdateEngine::DO_Update_FIT_ROOTx2_RWx1(Update_info & passport,u8 update_type,enrunfromSt runFrom,string FIRMWARE_PATH){
	//write rootfs
	if ((passport.passport.isMember("rootsize")==false)||(passport.passport.isMember("stortype")==false)){
		return ERROR;
	}


	string target_idx_rootro="uroot_ro";
	string target_idx_fitro="ufit_ro";

	if (runFrom==runfromUpdater){
		target_idx_rootro="froot_ro";
		target_idx_fitro="ffit_ro";
	}

	if (runFrom==runfromFactory){
			target_idx_rootro="uroot_ro";
			target_idx_fitro="ufit_ro";
		}

	u32 root_size=stoul(passport.passport["rootsize"].asString());
	u32 root_size_inblocks=(root_size/UPDATE_BLOCK_SIZE_DEFAULT)+1;
	string ddcmd;

	if (passport.passport["stortype"].asString()!="nand"){
		GPRINT(NORMAL_LEVEL,"write rootfs\n");
		SendSystemFromCnodaToUI_upd(UPDATE_EVENT_UPD_WRITE_ROOTFS);
		if (update_type==UPDATE_FW_TYPE_GZIP)
			//dd if=/www/pages/update/firmware.gzip skip=2048 ibs=8192|gunzip|dd of=/dev/mmcblk1p2 bs=8192
			BashResult(string_format("dd if=%s skip=%d bs=%d|gunzip|dd of=%s bs=%d",passport.source_file.c_str(),16,MB,passport.passport[target_idx_rootro].asCString(),MB));
		else
			BashResult(string_format("dd if=%s of=%s skip=%d bs=%d",passport.source_file.c_str(),passport.passport[target_idx_rootro].asCString(),16,MB));


		if (passport.passport["finger_type"].asString()=="md5"){
			string calc_md5=GetMD5_SumDD(passport.passport[target_idx_rootro].asString(),root_size,UPDATE_BLOCK_SIZE_DEFAULT);

			if (calc_md5==passport.passport["rootfinger"].asString()){
				GPRINT(NORMAL_LEVEL,"rootfinger checked\n");
				GPRINT(NORMAL_LEVEL,"write fit\n");
				SendSystemFromCnodaToUI_upd(UPDATE_EVENT_UPD_WRITE_FIT);
				BashResult(string_format("dd if=%s of=%s bs=%d count=%d",passport.source_file.c_str(),passport.passport[target_idx_fitro].asCString(),MB,16));
				//GPRINT(NORMAL_LEVEL,"m1\n");
				//string tmpfile=CnT->FIRMWARE_PATH+"/tmpfit";
				//BashResult(sf("dd if=/dev/zero of=%s bs=%d count=%d",tmpfile.c_str(),MB,16));
				//BashResult(sf("dd if=%s of=%s bs=%d count=%d conv=notrunc",passport.passport["ufit_ro"].asCString(),tmpfile.c_str(),MB,15));
				u32 fitsize=stoul(passport.passport["fitsize"].asString());

				calc_md5=GetMD5_SumDD(passport.passport[target_idx_fitro].asString(),fitsize,UPDATE_BLOCK_SIZE_DEFAULT);
				//remove(tmpfile.c_str());
				//GPRINT(NORMAL_LEVEL,"m2 passport.passport[fitfinger].isNull() %d\n",passport.passport["fitfinger"].isNull());

				//Json::StyledWriter wr;
				//cout << wr.write(passport.passport);
				string fitfinger=passport.passport["fitfinger"].asString();
				GPRINT(NORMAL_LEVEL,"fitfinger %s\n",fitfinger.c_str());

				if (calc_md5==fitfinger){
					GPRINT(NORMAL_LEVEL,"fitfinger checked\n");
					remove(passport.source_file.c_str());
					//GPRINT(NORMAL_LEVEL,"m1\n");
					string finger_file=FIRMWARE_PATH+'/'+passport.passport["finger"].asString();
					//GPRINT(NORMAL_LEVEL,"m2\n");
					string root_finger_file=FIRMWARE_PATH+'/'+passport.passport["rootfinger"].asString();
					//GPRINT(NORMAL_LEVEL,"m3\n");
					string fit_finger_file=FIRMWARE_PATH+'/'+passport.passport["fitfinger"].asString();
					//GPRINT(NORMAL_LEVEL,"m4\n");
					string passport_file=FIRMWARE_PATH+'/'+string("passport");
					//GPRINT(NORMAL_LEVEL,"m5\n");
					remove((char*)finger_file.c_str());
					remove((char*)root_finger_file.c_str());
					remove((char*)fit_finger_file.c_str());
					remove((char*)passport_file.c_str());
					GPRINT(NORMAL_LEVEL,"update file removed\n");

					UpdateBackupSettings(passport.passport);
					UpdatePlaceStump();
					GPRINT(NORMAL_LEVEL,"delay 5 sec for debug (sync stdout)\n");
					sync();
					sleep(5);
					return NO_ERROR;
				}
				else
					GPRINT(NORMAL_LEVEL,"error checkit fitfinger, device is broken calk md5 %s!= passp md5 %s \n",(char*)calc_md5.c_str(),passport.passport["fitfinger"].asCString());
			}
			else{
				GPRINT(NORMAL_LEVEL,"error checkit rootfinger, device is broken calk md5 %s!= passp md5 %s \n",(char*)calc_md5.c_str(),passport.passport["rootfinger"].asCString());
			}
		}


	}
	else {
		GPRINT(NORMAL_LEVEL,"DO_Update_FIT_ROOTx2_RWx1_raw NAND unsupport\n");
		return NO_ERROR;
	}

	return ERROR;
	//dd if=%s of=%s bs=1M count=%s
}

eErrorTp UpdateEngine::DO_Update(Update_info & passport,u8 update_type,enrunfromSt runFrom,string firmware_path){
	eErrorTp err=ERROR;
	if (passport.passport["updtype"]=="FIT_ROOTx2_RWx1"){

		err=DO_Update_FIT_ROOTx2_RWx1(passport,update_type,runFrom,firmware_path);
		if (err==ERROR)
			SendSystemFromCnodaToUI_upd(UPDATE_EVENT_UPD_ERROR);
	}
	return err;
}

//CnT->FIRMWARE_PATH+'/'+CnT->FIRMWARE_NAME
//CnT->CACHE_PATH
eErrorTp UpdateEngine::RunUpdateChain(string firmware_path,string firmware_file,string cache_path){

	Update_info passport;
	string fw_file_raw=firmware_file;
	string fw_file_gzip=fw_file_raw+".gzip";
	string fw_file_select=fw_file_raw;
	eErrorTp err=ERROR;
	buffer raw_passport(PASSPORT_MAX_SIZE);
	u8 update_type=UPDATE_FW_TYPE_RAW;

	GPRINT(NORMAL_LEVEL,"RunUpdateChain firmware_path[%s] firmware_file[%s] cache_path[%s]\n",firmware_path.c_str(),firmware_file.c_str(),cache_path.c_str());
	if (existsSync(fw_file_raw)==NO_ERROR){
		//No comp update
		fw_file_select=fw_file_raw;
		update_type=UPDATE_FW_TYPE_RAW;
	}
	else
	if (existsSync(fw_file_gzip)==NO_ERROR){
		fw_file_select=fw_file_gzip;
		update_type=UPDATE_FW_TYPE_GZIP;
		//gzip update

	}
	else
		return err;

	SendSystemFromCnodaToUI_upd(UPDATE_EVENT_UPD_START_UPDATE);
	GPRINT(NORMAL_LEVEL,"Found update type [%s], filename %s\n",(update_type==UPDATE_FW_TYPE_RAW)?"raw":"gzip",(char*)fw_file_select.c_str());
	if (GetImagePassport(fw_file_select, PASSPORT_OFFSET_VAR1,(char*)raw_passport.p(),(u32)PASSPORT_MAX_SIZE)==NO_ERROR){
		Json::Reader reader;
		//Json::Value root;
		if (reader.parse( (char*)raw_passport.p(), passport.passport )){

			//passport.passport=root;
			passport.source_file=fw_file_select;
			if (existsSync(sf("%s/%s",(char*)cache_path.c_str(),FACTORY_MARKER))==NO_ERROR){
				GPRINT(NORMAL_LEVEL,"System run from factory rootfs %s\n",passport.passport["froot_ro"].asCString());

				err=DO_Update(passport,update_type,runfromFactory,firmware_path);
				if (err==ERROR) SendSystemFromCnodaToUI_upd(UPDATE_EVENT_UPD_END_UPDATE_ERR);
				else
					SendSystemFromCnodaToUI_upd(UPDATE_EVENT_UPD_END_UPDATE_OK);
			}
			else
			{
				if (existsSync(sf("%s/%s",(char*)cache_path.c_str(),UPDATER_MARKER))==NO_ERROR){
					GPRINT(NORMAL_LEVEL,"System run from updater, target factory rootfs %s\n",passport.passport["froot_ro"].asCString());
					err=DO_Update(passport,update_type,runfromUpdater,firmware_path);
					if (err==ERROR) SendSystemFromCnodaToUI_upd(UPDATE_EVENT_UPD_END_UPDATE_ERR);
					else
						SendSystemFromCnodaToUI_upd(UPDATE_EVENT_UPD_END_UPDATE_OK);
				}
				else
					GPRINT(NORMAL_LEVEL,"Cancel flush firmware, no found marker\n");
			}

		}
	}

	return err;
}

eErrorTp UpdateEngine::CheckAndFoundUpdate(
		Json::Value & upad_storages,
		vector <string> & mount_points,
		vector <Update_info> & updinfo_list,
		string dfolder
		)
{
	eErrorTp err=ERROR;
	for (u8 idx=0;idx<upad_storages.size();idx++) {

		string mount_dev="/dev/"+upad_storages[idx].asString();
		if (existsSync(mount_dev)==ERROR){
			GPRINT(NORMAL_LEVEL,"Skip %s, device not detect\n",mount_dev.c_str());
			continue;
		}

		GPRINT(NORMAL_LEVEL,"Detect storage device %s\n",mount_dev.c_str());
		string mp=string_format("%s/mp11p/%s",CACHE_PATH_DEF,upad_storages[idx].asCString());

		MkPath(mp.c_str(), 0xffffffff);

		if (Mount(mount_dev.c_str(),mp.c_str(),NULL,0,"")==NO_ERROR){
			mount_points.push_back(mp);
			GPRINT(NORMAL_LEVEL,"Mount %s to %s\n",(char*)mount_dev.c_str(),(char*)mp.c_str());
			if (upad_firmware_file=="update"){
				vector <string> upd_list;
				// SearchFilesByMask(string & dir,char * suffix,vector <string> & result)
				if (SearchFilesByMask((char*)mp.c_str(),".update",upd_list)==NO_ERROR)
					if (upd_list.size()!=0){
						for (u8 z=0;z<upd_list.size();z++){
							Update_info ui;

							string source_fw_path=string_format("%s/%s",(char*)mp.c_str(),(char*)upd_list[z].c_str());
						//string noremove="noremove"
							//GPRINT(NORMAL_LEVEL,"Detect %s update\n",source_fw_path.c_str());
							u32 result=0;
							SendSystemFromCnodaToUI_upd(string_format("{\"uprep\":{\"msg\":\"%s\",\"arg\":[\"%s\"]}}",UPDPREP_EVENT_UPD_CHECK,(char*)upd_list[z].c_str()));
							if ((result=Update_fw_check(source_fw_path,dfolder,ui))==0){
								SendSystemFromCnodaToUI_upd(string_format("{\"uprep\":{\"msg\":\"%s\",\"arg\":[\"%s\"]}}",UPDPREP_EVENT_UPD_CHECK_OK,(char*)upd_list[z].c_str()));
								//GPRINT(NORMAL_LEVEL,"New passport %s\n",ui.dump().c_str());
								err=NO_ERROR;
								updinfo_list.push_back(ui);
							}
							else
							{
								SendSystemFromCnodaToUI_upd(string_format("{\"uprep\":{\"msg\":\"%s\",\"arg\":[\"%s\"]}}",UPDPREP_EVENT_UPD_CHECK_ERR,(char*)upd_list[z].c_str()));
								GPRINT(NORMAL_LEVEL,"Detect fault update %s err 0x%08d\n",source_fw_path.c_str(),result);
							}

						}
					}
			}
			//if (Update_fw_check(source,dfolder,dimagename,remove_mod)==NO_ERROR)


		}
		else
			GPRINT(NORMAL_LEVEL,"Error Mount %s to %s\n",(char*)mount_dev.c_str(),(char*)mp.c_str());
	}
	return err;
}


eErrorTp UpdateEngine::UmountAllUpdateStrorages(vector <string> & mount_points)
{
	eErrorTp err=NO_ERROR;
	for (u8 z=0;z<mount_points.size();z++){
		GPRINT(NORMAL_LEVEL,"Umount %s\n",(char*)mount_points[z].c_str());
		if (UmountForce(mount_points[z].c_str())==ERROR){
				GPRINT(NORMAL_LEVEL,"Error Umount %s\n",(char*)mount_points[z].c_str());
				err=ERROR;
		}
	}
	return err;
}

eErrorTp UpdateEngine::GetLastVersionUpdate(vector <Update_info> & updinfo_list,u32 & updinfo_list_fidx){
	eErrorTp err=ERROR;
	if (updinfo_list.size()!=0){
			string upd_vers_max="";
			updinfo_list_fidx=0;
			for (u8 z=0;z<updinfo_list.size();z++){
				GPRINT(NORMAL_LEVEL,"Found correct update fw, passport [%s]\n",updinfo_list[z].dump().c_str());
				if (updinfo_list[z].passport["version"].asString()>upd_vers_max){
					upd_vers_max=updinfo_list[z].passport["version"].asString();
					updinfo_list_fidx=z;
					err=NO_ERROR;
				}
			}
			GPRINT(NORMAL_LEVEL,"Select update FW with version %s\n",upd_vers_max.c_str());
			SendSystemFromCnodaToUI_upd(string_format("{\"uprep\":{\"msg\":\"%s\",\"arg\":[\"%s\",\"%s\"]}}",UPDPREP_EVENT_UPD_SELECT_VERS,upd_vers_max.c_str(),updinfo_list[updinfo_list_fidx].source_file.c_str()));
	}
	return err;
}


eErrorTp UpdateEngine::CheckHWFromTarget(Update_info & source_update)
{

	buffer raw_passport(PASSPORT_MAX_SIZE);
	if (GetImagePassport(source_update.passport["ffit_ro"].asString(),(u32)PASSPORT_OFFSET_VAR1,(char*)raw_passport.p(),(u32)PASSPORT_MAX_SIZE)==NO_ERROR)
	{
		Json::Reader rd;
		Json::Value val;
		if (rd.parse( (char*)raw_passport.p(), val )){
			if (val.isMember("hw")&&source_update.passport.isMember("hw")){
				if (val["hw"].asString()==source_update.passport["hw"].asString()){
					GPRINT(NORMAL_LEVEL,"Target hw and Update hw identical\n");
					return NO_ERROR;
				}
				else
					GPRINT(NORMAL_LEVEL,"Target hw %s Update hw %s\n",val["hw"].asCString(),source_update.passport["hw"].asCString());
			}
			else
				GPRINT(NORMAL_LEVEL,"Target hw or Update hw not found in passport\n");
		}
		else
			GPRINT(NORMAL_LEVEL,"Incorrect passport for read hw\n");
	}
	else
		GPRINT(NORMAL_LEVEL,"Error read passport from target\n");

	return ERROR;
}
eErrorTp UpdateEngine::CheckByPassportFromTarget(Update_info & source_update)
{

	buffer raw_passport(PASSPORT_MAX_SIZE);
//	bool finalize=true;
	eErrorTp err=NO_ERROR;

	//return ERROR;
	GPRINT(NORMAL_LEVEL,"source_update.passport[ufit_ro] %s\n",source_update.passport["ufit_ro"].asCString());
	if (GetImagePassport(source_update.passport["ufit_ro"].asString(),(u32)PASSPORT_OFFSET_VAR1,(char*)raw_passport.p(),(u32)PASSPORT_MAX_SIZE)==NO_ERROR)
	{
		GPRINT(NORMAL_LEVEL,"Target passport %s\n",raw_passport.p());

		if (strcmp((char*)source_update.passport_str.c_str(),(char*)raw_passport.p())==0){
			GPRINT(NORMAL_LEVEL,"Identical passports from firmware and target, check checksum\n");
			u32 blocksize=UPDATE_BLOCK_SIZE_DEFAULT;
			if (source_update.passport.isMember("blocksize"))
				blocksize=stoul(source_update.passport["blocksize"].asString());
			GPRINT(NORMAL_LEVEL,"blocksize %d\n",blocksize);

			if (source_update.passport.isMember("fitsize")){
				string md5_fit=GetMD5_SumDD(source_update.passport["ufit_ro"].asString(),stoul(source_update.passport["fitsize"].asString()),blocksize);

				if (md5_fit==source_update.passport["fitfinger"].asString()){
					GPRINT(NORMAL_LEVEL,"Fit finger checked!!!\n");
					GPRINT(NORMAL_LEVEL,"System firmware identical, skip update\n");
					//finalize=false;
					//Не делать проверку rootfs, получается сложная и запутанная логика
					//Например нужно правильно обрабатывать сжатые и не сжатые образы, нужна доп. flash память для этого
					//Если нужно принудительное обновление, то нужно выставить upad_force=1,
					//Если нет, но очень нужна высокая надежность, то выход ramdisk

					/*string md5_root=GetMD5_SumDD(source_update.passport["uroot_ro"].asString(),stoul(source_update.passport["rootsize"].asString()),blocksize);
					if (md5_root==source_update.passport["rootfinger"].asString()){
						GPRINT(NORMAL_LEVEL,"Root finger checked!!!\n");
						GPRINT(NORMAL_LEVEL,"Firmware identical, skip update\n");
						finalize=false;
					}
					else{
						GPRINT(NORMAL_LEVEL,"Root finger is different %s!=%s\n",md5_root.c_str(),source_update.passport["rootfinger"].asCString());
						//Finalize_update(CnT->FIRMWARE_PATH,FIRMWARE_NAME,source_update);
					}*/
					err=NO_ERROR;
				}
				else{
					err=ERROR;
					GPRINT(NORMAL_LEVEL,"Fit finger is different %s!=%s\n",md5_fit.c_str(),source_update.passport["fitfinger"].asCString());
				}
			}
			else{
				GPRINT(NORMAL_LEVEL,"Reduced passport!!!\n");
				err=NO_ERROR;
			}
		}
		else
		{
			err=ERROR;
			GPRINT(NORMAL_LEVEL,"Passports is differents from firmware and target, try update\n");
		}
	}
	else
		err=ERROR;

	//if (finalize)
	//{

	//}
	return err;
}

eErrorTp UpdateEngine::CheckAndUpdate(void){


	//CnT->FIRMWARE_PATH+'/'+CnT->FIRMWARE_NAME
	//CnT->CACHE_PATH
	GPRINT(MEDIUM_LEVEL,"CheckAndUpdate\n");
	if (CnT->reboot_state)
		return NO_ERROR;
	if (RunUpdateChain(CnT->FIRMWARE_PATH,CnT->FIRMWARE_PATH+'/'+CnT->FIRMWARE_NAME,CnT->CACHE_PATH)==NO_ERROR)
	{
		SendSystemFromCnodaToUI_upd(UPDATE_EVENT_UPD_SUCCESS_DO_REBOOT);
		GPRINT(NORMAL_LEVEL,"update Success, reboot!!!\n");
		RebootSystem();
		return NO_ERROR;
	}
	return ERROR;

}
eErrorTp UpdateEngine::FindAndPrepareUpdate(void){
	vector <Update_info> updinfo_list;
	vector <string> mount_points;
	bool need_update=false;
	u32 updinfo_list_fidx;
	updinfo_list.clear();
	mount_points.clear();
	GPRINT(NORMAL_LEVEL,"Check storages for update\n");
	SendSystemFromCnodaToUI_upd(UPDPREP_EVENT_UPD_START_SEARCH);
	CheckAndFoundUpdate(upad_storages,mount_points,updinfo_list,CnT->FIRMWARE_PATH);

	if (updinfo_list.size()!=0){
		if (GetLastVersionUpdate(updinfo_list,updinfo_list_fidx)==NO_ERROR){
			//GPRINT(NORMAL_LEVEL,"GetLastVersionUpdate(updinfo_list,updinfo_list_fidx)==NO_ERROR\n");
			if (upad_force==1){
				SendSystemFromCnodaToUI_upd(string_format("{\"uprep\":{\"msg\":\"%s\",\"arg\":[\"%s\"]}}",UPDPREP_EVENT_UPD_FINALIZE,updinfo_list[updinfo_list_fidx].source_file.c_str()));
				if (Finalize_update(updinfo_list[updinfo_list_fidx])==NO_ERROR)
					need_update=true;
			}
			else{
				if (CheckHWFromTarget(updinfo_list[updinfo_list_fidx])==NO_ERROR){
					if (CheckByPassportFromTarget(updinfo_list[updinfo_list_fidx])==ERROR){
						//Если паспорта не совпали или fit отличаются, тогда обновление разрешено
						SendSystemFromCnodaToUI_upd(string_format("{\"uprep\":{\"msg\":\"%s\",\"arg\":[\"%s\"]}}",UPDPREP_EVENT_UPD_FINALIZE,updinfo_list[updinfo_list_fidx].source_file.c_str()));
						if (Finalize_update(updinfo_list[updinfo_list_fidx])==NO_ERROR)
							need_update=true;
					}
					else
						SendSystemFromCnodaToUI_upd(UPDPREP_EVENT_UPD_SKIP_IDENTICAL);
				}
				else{
					GPRINT(NORMAL_LEVEL,"Skip update, firmware is not for this hardware\n");
					SendSystemFromCnodaToUI_upd(UPDPREP_EVENT_UPD_SKIP_ERR_HW);
				}
			}
		}
		//else{
		//	SendSystemFromCnodaToUI_upd(UPDPREP_EVENT_UPD_HIDE_SYSTEM_BLOCK);
		//}
	}
	//Unmount all
	UmountAllUpdateStrorages(mount_points);
	SendSystemFromCnodaToUI_upd(UPDPREP_EVENT_UPD_END_SEARCH);
	if (existsSync(sf("%s/%s",(char*)CnT->CACHE_PATH.c_str(),FACTORY_MARKER))==NO_ERROR){
		//Если находимся в factory то нет смысла перезагружаться, идем в обновление
		GPRINT(NORMAL_LEVEL,"System run in factory img, force check prepaired update\n");
		CheckAndUpdate();
	}
	else{
		if (need_update){
			GPRINT(NORMAL_LEVEL,"System run in user img, need update, reboot\n");
			SendSystemFromCnodaToUI_upd(UPDPREP_EVENT_UPD_REBOOT);
			RebootSystem();
		}
		else{
			GPRINT(NORMAL_LEVEL,"System run in user img, no need update\n");
			//SendSystemFromCnodaToUI_upd(UPDPREP_EVENT_UPD_HIDE_SYSTEM_BLOCK);
		}
	}



	//if (need_update==false)


	return NO_ERROR;
}

void UpdateEngine::RemoveUpdateMarker(void){
	string upd=CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]+'/'+UPDATE_STUMP;
	remove(upd.c_str());
	upd=CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED]+'/'+UPDATE_STUMP;
	remove(upd.c_str());
	SendSystemFromCnodaToUI_upd(UPDPREP_EVENT_UPD_HIDE_SYSTEM_BLOCK);
}
eErrorTp UpdateEngine::CheckUpdateMarker(void){
	if ((existsSync(CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]+'/'+UPDATE_STUMP)==NO_ERROR)||(existsSync(CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED]+'/'+UPDATE_STUMP)==NO_ERROR))
	{
		//sleep(2);
		SendSystemFromCnodaToUI_upd(UPDPREP_EVENT_UPD_COMPLETE);
		return NO_ERROR;
	}
	return ERROR;
}
