/*
 * settings_adm.cxx
 *
 *  Created on: 15 янв. 2019 г.
 *      Author: root
 */


#include "engine/settings_adm.h"
#include "engine/global.h"
#include "engine/algo/MD5.h"

SettingsAdm::SettingsAdm (void)
	{
				SourceStr="Settings";
				ObjPref="Shared";

				//GPRINT(NORMAL_LEVEL,"%s created\n",SourceStr.c_str());

				//InitGset();
				debug_level=NORMAL_LEVEL;
	};

void SettingsAdm::SetLogLevel(eDebugTp dlevel)
	{
		debug_level=dlevel;
	}

eErrorTp SettingsAdm::LoadSetting(string & result,string setting)
	{
		//CnT->DISABLE_OUT - use in settings loader
		string fname=setting;

		string path=CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]+string("/")+fname+SETTING_EXTENSION;
		string path_crc=CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]+string("/")+fname+SETTING_EXTENSION_CRC;
		string rdata;
		bool useSign=false;
		for (string & signCfg : CnT->signedConfigs){
			if (signCfg==setting){
				useSign=true;
				break;
			}
		}

		//printf("LoadSetting [%s]\n",path.c_str());
		//printf("CheckJSONInStorage\n");
		if (useSign){
			checkSignSetting(setting);
		}

		if (CheckJSONInStorage(path, path_crc,rdata)==NO_ERROR)
		{
			//printf("CheckJSONInStorage1\n");
			result=rdata;
			return NO_ERROR;
		}
		else{
			//printf("SyncGsetOneSettingLazy\n");
			SyncGsetOneSettingLazy(setting);
			if (CheckJSONInStorage(path, path_crc,rdata)==ERROR)
			{
				result="{}";
				return ERROR;
			}
			else
				result=rdata;
		}

		return NO_ERROR;
	}

eErrorTp SettingsAdm::LoadSetting(Json::Value & JSON,string setting)
	{
		//CnT->DISABLE_OUT - use in settings loader
		string fname=setting;
		Json::Reader rd;
		string path=CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]+string("/")+fname+SETTING_EXTENSION;
		string path_crc=CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]+string("/")+fname+SETTING_EXTENSION_CRC;
		string rdata;
		//printf("LoadSetting [%s]\n",path.c_str());
		//printf("CheckJSONInStorage\n");
		if (CheckJSONInStorage(path, path_crc,rdata)==NO_ERROR)
		{
			//printf("CheckJSONInStorage1\n");
			if (rd.parse(rdata,JSON))
				return NO_ERROR;
			else
				return ERROR;
		}
		else{
			//printf("SyncGsetOneSettingLazy\n");
			SyncGsetOneSettingLazy(setting);
			if (CheckJSONInStorage(path, path_crc,rdata)==ERROR)
			{
				return ERROR;
			}
			else
				if (rd.parse(rdata,JSON))
					return NO_ERROR;
				else
					return ERROR;
		}

		return NO_ERROR;
	}

string SettingsAdm::GetSettingsName(char * virtdev)
	{
		string devid=string_format("%s/devid.%s",CnT->DEVID_PATH.c_str(),virtdev);

		ifstream file;

		string id="";
		string result;
		file.open(devid.c_str());

		if (file.is_open())
		{
			file >> id;
			//printf("Found devid %s\n",id.c_str());
			file.close();
			result=string_format("settings.%s.%s",virtdev,id.c_str());
		}
		else
		{

			result=string_format("settings.%s",virtdev);
		}

		//printf("GetSettingsName %s\n",result);
		return result;
	}

	//LoadJSONInStorage deprecated
eErrorTp SettingsAdm::LoadJSONInStorage(string & JSONstr,char * filename)
	{
		return LoadSetting(JSONstr,filename);
	}

eErrorTp SettingsAdm::ParseSettingInStorage(Json::Value & root,char * filename)
	{
		string JSONstr;
		if (LoadSetting(JSONstr,filename)!=ERROR)
		{
			Json::Reader reader;
			bool parsingSuccessful = reader.parse( JSONstr.c_str(), root );
			if (parsingSuccessful)
			{
				return NO_ERROR;
			}
			return ERROR;
		}
		return ERROR;
	}

eErrorTp SettingsAdm::GetSettingValue(Json::Value & root,string name,string & value)
	{
	    if (root.isArray()){
	    	return ERROR;
	    }
		if (root[JSON_DATA_SYMB].isObject()&&(root[JSON_DATA_SYMB].isMember(name.c_str())))
		{
			value=root[JSON_DATA_SYMB][name.c_str()].asString();

		}
		else{
			return ERROR;
		}
		return NO_ERROR;
	}


std::pair <eErrorTp,bool> SettingsAdm::ChangeReplaceSettingValueN(Json::Value & root,string name,string value)
	{
		bool changed=false;
		if (root[JSON_DATA_SYMB].isObject())
		{
			if (root[JSON_DATA_SYMB].isMember(name.c_str())){
				 if ((root[JSON_DATA_SYMB][name.c_str()].isString())&&(root[JSON_DATA_SYMB][name.c_str()].asString()!=value)){
					 root[JSON_DATA_SYMB][name.c_str()]=value;
					 changed=true;
				 }
			}
			else{
				root[JSON_DATA_SYMB][name.c_str()]=value;
				changed=true;
			}
		}
		else
			return make_pair(ERROR,changed);
		return make_pair(NO_ERROR,changed);
	}

std::pair <eErrorTp,bool> SettingsAdm::ChangeSettingValueN(Json::Value & root,string name,string value)
	{
		bool changed=false;
		if (root[JSON_DATA_SYMB].isObject()&&
					root[JSON_DATA_SYMB].isMember(name.c_str()))
		{
			 if ((root[JSON_DATA_SYMB][name.c_str()].isString())&&(root[JSON_DATA_SYMB][name.c_str()].asString()!=value)){
				 root[JSON_DATA_SYMB][name.c_str()]=value;
				 changed=true;
			 }
		}
		else
			return ChangeReplaceSettingValueN(root,name,value);
		return make_pair(NO_ERROR,changed);
	}


std::pair <eErrorTp,bool> SettingsAdm::ChangeSettingValueN(Json::Value & root,string name,u32 value)
	{
		return ChangeSettingValueN(root,name,sf("%u",value));
	}

std::pair <eErrorTp,bool> SettingsAdm::ChangeSettingValueN(Json::Value & root,string name,int value)
	{
		//string val=;
		return ChangeSettingValueN(root,name,sf("%d",value));
	}


eErrorTp SettingsAdm::SaveSettingToStorage(Json::Value & root,string filename)
	{
		//Json::FastWriter writer;
		string json;

		json=FastWriteJSON(root);
		GPRINT(MEDIUM_LEVEL,"Save json %s\n",json.c_str());
		return SaveSetting(json,filename);

	}

eErrorTp SettingsAdm::CheckSaved(string filename,u32 crc32)
	{
		std::ifstream  file;
		u32 ncrc32=0;
		file.open(filename);
		if (file.is_open())
		{
			std::string tdata((std::istreambuf_iterator<char>(file)),
			                 std::istreambuf_iterator<char>());

			//	file.read(fbuf,10000);
				if ((!file)||file.fail())
				{
					file.close();
					//free(fbuf);
					printf("Error check file [%s]\n",filename.c_str());
					return ERROR;
				}
				//fsize=GetFileSize(filename.c_str());
				ncrc32=Crc32Buf((u8*)tdata.c_str(),tdata.size());
				file.close();
		}
		//free(fbuf);
		if (crc32!=ncrc32)
			return ERROR;
		else
			return NO_ERROR;
	}

eErrorTp SettingsAdm::SaveSetting(string JSONstr,string filename)
	{
		string fname=filename;
		bool useSign=false;
		//bool signError=false;
		for (string & signCfg : CnT->signedConfigs){
			if (signCfg==filename){
				useSign=true;
				if (checkSignSetting(fname)==ERROR){
					//В этом случае настройка откатилась к дефолтной, т.к. подписи нет или конфиг был изменен
					return ERROR;
				}
				//В этом случае подпись верная, значит можно сохранить конфиг
				break;
			}
		}
		string path1=CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]+string("/")+fname+SETTING_EXTENSION;
		string path2=CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED]+string("/")+fname+SETTING_EXTENSION;
		string path1crc=CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]+string("/")+fname+SETTING_EXTENSION_CRC;
		string path2crc=CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED]+string("/")+fname+SETTING_EXTENSION_CRC;
		std::ofstream  file1;
		std::ofstream  file2;
		std::ofstream  file1crc;
		std::ofstream  file2crc;
		string sign;
		mutex_lock(CnT->SettingSave);
		u32 crc32=Crc32Buf((u8*)JSONstr.c_str(),JSONstr.size());
		char bufcrc[20];
		snprintf(bufcrc,sizeof(bufcrc),"%08x",crc32);


		GPRINT(NORMAL_LEVEL,"Save file %s crc32[0x%s]\n",path1.c_str(),bufcrc);

		try
		{

		u32 saver_cntr=0;
		do
		{
			if (saver_cntr>0) {
				mdelay(50);
				printf("Repeat %d save %s\n",saver_cntr,path1.c_str());
			}

			file1.open(path1.c_str(),std::ofstream::out);
			if ((!file1)||file1.fail())
		    {
				file1.close();

				GPRINT(NORMAL_LEVEL,"Error create file1 [%s]\n",path1.c_str());
				throw 4;
			 }
			file1 << JSONstr;
			file1.flush();
			file1.close();
			if (useSign){
				sign=calcDataSign(JSONstr);
				WriteStringFile((char*)string(CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]+string("/")+fname+SETTING_EXTENSION_SIGN).c_str(),sign);
			}
			sync();
			saver_cntr++;
			if (saver_cntr>10){
				printf("Crit error file %s not save\n",path1.c_str());
				break;
			}
		}while(CheckSaved(path1.c_str(),crc32)!=NO_ERROR);

		saver_cntr=0;
		do
		{
			if (saver_cntr>0) {
				mdelay(50);
				printf("Repeat %d save %s\n",saver_cntr,path2.c_str());
			}
			file2.open(path2.c_str(),std::ofstream::out);
			 if ((!file2)||file2.fail())
			  {
					file2.close();
					GPRINT(NORMAL_LEVEL,"Error create file2 [%s]\n",path2.c_str());
					throw 3;
			  }
			 file2 << JSONstr;
			 file2.flush();
			 file2.close();
			if (useSign){
					WriteStringFile((char*)string(CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED]+string("/")+fname+SETTING_EXTENSION_SIGN).c_str(),sign);
			}
			 sync();
			 saver_cntr++;
			 if (saver_cntr>10) {
				 printf("Crit error file %s not save\n",path2.c_str());
				 break;
			 }
		}while(CheckSaved(path2.c_str(),crc32)!=NO_ERROR);

		file1crc.open(path1crc.c_str(),std::ofstream::out);
			if ((!file1)||file1.fail())
			 {
				file1crc.close();
				GPRINT(NORMAL_LEVEL,"Error create file1 [%s]\n",path1.c_str());
				throw 2;
			 }

		file1crc << bufcrc;
		file1crc.flush();
		file1crc.close();

		file2crc.open(path2crc.c_str(),std::ofstream::out);
			if ((!file1)||file1.fail())
			 {
				GPRINT(NORMAL_LEVEL,"Error create file1 [%s]\n",path1.c_str());
				throw 1;
			 }


		file2crc << bufcrc;
		file2crc.flush();
		file2crc.close();
		sync();
		}
		catch (int a)
		{
			switch(a)
			{
				case 1: file1crc.close();
				case 2: file2.close();
				case 3: file1.close();
				break;
			}
			sync();
			mutex_unlock(CnT->SettingSave);
			return ERROR;
		}
		sync();
		mutex_unlock(CnT->SettingSave);
		return NO_ERROR;
	}

eErrorTp SettingsAdm::CheckJSONInStorage(string & data_file, string & crc_file,string & rdata)
	{
		std::ifstream  crcfile;
		std::stringstream ss;
		string crc32str="";
		u32 crc32cmp=0;
		//string rdata;
		std::ifstream  fil;
		u32 crc32;

		crcfile.open(crc_file);
		if (crcfile.is_open())
		{

				crcfile >> crc32str;

				ss << std::hex << crc32str;
				ss >> crc32cmp;
				//printf("%s crc32cmp %d\n",crc_file,crc32cmp);
				crcfile.close();
				GPRINT(HARD_LEVEL,"Found crc32 file for %s content %s/0x%04x\n",crc_file.c_str(),crc32str.c_str(),crc32cmp);
				if (crc32cmp==0)//incorrect value, corrupt crc file
				{
					RemoveFile(crc_file);
					RemoveFile(data_file);
					GPRINT(NORMAL_LEVEL,"corrupt crc file file %s\n",crc_file.c_str());
					GPRINT(NORMAL_LEVEL,"			    remove %s\n",data_file.c_str());
					return ERROR;
				}
		}
		else{
			GPRINT(HARD_LEVEL,"Not found crc32 file %s for config %s\n",crc_file.c_str(),data_file.c_str());
			crc32cmp=0;//not check file, so ok
		}

		fil.open(data_file);
		//if (CnT->DISABLE_OUT==0)

		if (fil.is_open())
		{
			rdata.assign((istreambuf_iterator<char>(fil)),istreambuf_iterator<char>());
			fil.close();
			if (rdata.size()==0)
			{
				GPRINT(NORMAL_LEVEL,"Remove 0 size file %s\n",data_file.c_str());
				RemoveFile(data_file);
			}

			GPRINT(HARD_LEVEL,"Read config %s size %d\n",data_file.c_str(),rdata.size());
			crc32=Crc32Buf((u8*)rdata.c_str(),rdata.size());
			GPRINT(HARD_LEVEL,"Calc crc32 0x%04x, saved crc32 0x%04x\n",crc32,crc32cmp);
			if ((crc32==crc32cmp)||(crc32cmp==0))
				return NO_ERROR;
		}
		return ERROR;
	}

eErrorTp SettingsAdm::RemoveFile(string & file)
	{
		std::remove(file.c_str());
		sync();
		return NO_ERROR;
	}

eErrorTp SettingsAdm::CopyFile(string & FileNameSrc, string & FileNameDst)
	{
		eErrorTp err=NO_ERROR;
		struct stat lst;

		if (lstat(FileNameSrc.c_str(),&lst)==-1)
		{
			GPRINT(NORMAL_LEVEL,"Error copy, source file [%s] not found\n",FileNameSrc.c_str());
			return ERROR;
		}

		std::ofstream  dst(FileNameDst.c_str(),std::ios::binary);
		if ((!dst)||dst.fail())
		{
			GPRINT(NORMAL_LEVEL,"Error create dst [%s]\n",FileNameDst.c_str());
			sync();
			return ERROR;
		}

		std::ifstream  src(FileNameSrc.c_str(), std::ios::binary);
		if ((!src)||src.fail())
		{
			 GPRINT(NORMAL_LEVEL,"Error src [%s] not copy to [%s]\n",FileNameSrc.c_str(),FileNameDst.c_str());
			 dst.close();
			 sync();
			 return ERROR;
		}

		dst << src.rdbuf();

		if (dst.fail()||src.fail())
		 {
			err=ERROR;
			GPRINT(NORMAL_LEVEL,"Error copy [%s] to [%s]\n",FileNameSrc.c_str(),FileNameDst.c_str());
		 }
		else
		{
			 //Print->GPRINT(MEDIUM_LEVEL,"Copy [%s] to [%s]\n",FileNameSrc,FileNameDst);
		}
		src.close();
		dst.close();
		sync();
		return err;
	}

eErrorTp SettingsAdm::RemoveFromSetting4x4(char * filename)
	{
		string first=string_format("%s/%s",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST].c_str(),filename);
		string second=string_format("%s/%s",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED].c_str(),filename);
		string first_rsv=string_format("%s/%s/%s",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST].c_str(),RESERVED_PATH_NAME,filename);
		string second_rsv=string_format("%s/%s/%s",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED].c_str(),RESERVED_PATH_NAME,filename);
		RemoveFile(first);
		RemoveFile(second);
		RemoveFile(first_rsv);
		RemoveFile(second_rsv);
		GPRINT(NORMAL_LEVEL,"Remove %s\n",first.c_str());
		GPRINT(NORMAL_LEVEL,"Remove %s\n",second.c_str());
		GPRINT(NORMAL_LEVEL,"Remove %s\n",first_rsv.c_str());
		GPRINT(NORMAL_LEVEL,"Remove %s\n",second_rsv.c_str());
		return NO_ERROR;
	}

eErrorTp SettingsAdm::SyncGsetOneSetting4x4(string & rawname)
	{
		enum {enSYS,enSYS_EXT,enSYS_RSV,enSYS_EXT_RSV,enSYS_MAX};
		string sett[enSYS_MAX];
		string sett_crc[enSYS_MAX];
		eErrorTp check[enSYS_MAX];
		struct stat lst;

		string fact_sett=string_format("%s/%s.set",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_DEFAULT].c_str(),rawname.c_str());
		string fact_sett_crc=string_format("%s/%s.crc",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_DEFAULT].c_str(),rawname.c_str());
		//string fact_sett="";
		//string fact_sett_crc="";

		sett[enSYS]=string_format("%s/%s.set",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST].c_str(),rawname.c_str());
		sett_crc[enSYS]=string_format("%s/%s.crc",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST].c_str(),rawname.c_str());

		sett[enSYS_RSV]=string_format("%s/%s/%s.set",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST].c_str(),RESERVED_PATH_NAME,rawname.c_str());
		sett_crc[enSYS_RSV]=string_format("%s/%s/%s.crc",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST].c_str(),RESERVED_PATH_NAME,rawname.c_str());

		sett[enSYS_EXT_RSV]=string_format("%s/%s/%s.set",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED].c_str(),RESERVED_PATH_NAME,rawname.c_str());
		sett_crc[enSYS_EXT_RSV]=string_format("%s/%s/%s.crc",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED].c_str(),RESERVED_PATH_NAME,rawname.c_str());

		sett[enSYS_EXT]=string_format("%s/%s.set",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED].c_str(),rawname.c_str());
		sett_crc[enSYS_EXT]=string_format("%s/%s.crc",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED].c_str(),rawname.c_str());

		u8 checked_item=0xff;
		string rdata;
		for (u32 i=0;i<enSYS_MAX;i++)
		{
			check[i]=CheckJSONInStorage(sett[i],sett_crc[i],rdata);

			if ((check[i]==NO_ERROR)&&(checked_item==0xff))
				checked_item=i;
			//printf("item %d\n",check[i]);
		}


		if (checked_item!=0xff)
		{
			for (u32 i=0;i<enSYS_MAX;i++)
			{
				if (check[i]==ERROR)
				{
							CopyFile(sett[checked_item],sett[i]);
							if (lstat(sett_crc[checked_item].c_str(),&lst)!=-1)
							{
								if (lst.st_size==0)
								{

									RemoveFile(sett_crc[checked_item]);

								}
								else
									CopyFile(sett_crc[checked_item],sett_crc[i]);
							}
							else
							{
								for (u32 j=0;j<enSYS_MAX;j++)
								{
									RemoveFile(sett_crc[j]);
								}
							}
							GPRINT(NORMAL_LEVEL,"Restore %s to %s\n",sett[checked_item].c_str(),sett[i].c_str());
							GPRINT(NORMAL_LEVEL,"    	 %s to %s\n",sett_crc[checked_item].c_str(),sett_crc[i].c_str());
						//}
					///}
				}
			}
		}
		else
		{
			for (u32 i=0;i<enSYS_MAX;i++)
			{
				CopyFile(fact_sett,sett[i]);
				CopyFile(fact_sett_crc,sett_crc[i]);
				GPRINT(NORMAL_LEVEL,"Restore %s to %s\n",fact_sett.c_str(),sett[i].c_str());
				GPRINT(NORMAL_LEVEL,"    	 %s to %s\n",fact_sett_crc.c_str(),sett_crc[i].c_str());
			}
			if (CheckJSONInStorage(sett[enSYS],sett_crc[enSYS],rdata)==ERROR)
			{
				GPRINT(NORMAL_LEVEL,"Factory not restored, critical error, storage is corrupted, go to service\n");
				return ERROR;
			}

		}
		return NO_ERROR;
	}

eErrorTp SettingsAdm::SyncGsetOneSettingLazy(string & rawname)
	{
		string sett="";
		string sett_crc="";
		string sett_rsv="";
		string sett_rsv_crc="";
		string ex_sett="";
		string ex_sett_crc="";
		string ex_sett_rsv="";
		string ex_sett_rsv_crc="";
		string fact_sett="";
		string fact_sett_crc="";
		string rdata;
		sett_rsv=string_format("%s/%s/%s.set",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST].c_str(),RESERVED_PATH_NAME,rawname.c_str());
		sett=string_format("%s/%s.set",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST].c_str(),rawname.c_str());

		//printf("1 %s\n",CnT->SETTING_STORAGE_MOUNTP[0].c_str());
		//printf("2 %s\n",CnT->SETTING_STORAGE_MOUNTP[1].c_str());
		//printf("3 %s\n",CnT->SETTING_STORAGE_MOUNTP[2].c_str());
		//printf("sett %s\n",sett.c_str());

		if (equalFiles(sett,sett_rsv)==NO_ERROR)
		{
				GPRINT(HARD_LEVEL,"file %s == %s\n",sett.c_str(),sett_rsv.c_str());
		}
		else
		{
			sett_crc=string_format("%s/%s.crc",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST].c_str(),rawname.c_str());
			sett_rsv_crc=string_format("%s/%s/%s.crc",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST].c_str(),RESERVED_PATH_NAME,rawname.c_str());

			if (CheckJSONInStorage(sett,sett_crc,rdata)==ERROR)
				{
				RemoveFile(sett);
				RemoveFile(sett_crc);
				GPRINT(NORMAL_LEVEL,"Incorrect setting %s\n",sett.c_str());
				ex_sett_rsv=string_format("%s/%s/%s.set",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED].c_str(),RESERVED_PATH_NAME,rawname.c_str());
				ex_sett_rsv_crc=string_format("%s/%s/%s.crc",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED].c_str(),RESERVED_PATH_NAME,rawname.c_str());
				ex_sett=string_format("%s/%s.set",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED].c_str(),rawname.c_str());
				ex_sett_crc=string_format("%s/%s.crc",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED].c_str(),rawname.c_str());

				if (CheckJSONInStorage(ex_sett,ex_sett_crc,rdata)==ERROR)
					{
					RemoveFile(ex_sett);
					RemoveFile(ex_sett_crc);
					GPRINT(NORMAL_LEVEL,"Incorrect extend setting %s\n",ex_sett.c_str());
					if (CheckJSONInStorage(sett_rsv,sett_rsv_crc,rdata)==ERROR)
					{
						RemoveFile(sett_rsv);
						RemoveFile(sett_rsv_crc);
						GPRINT(NORMAL_LEVEL,"Incorrect reserved setting %s\n",sett_rsv.c_str());
						if (CheckJSONInStorage(ex_sett_rsv,ex_sett_rsv_crc,rdata)==ERROR)
							{
							RemoveFile(ex_sett_rsv);
							RemoveFile(ex_sett_rsv_crc);
							GPRINT(NORMAL_LEVEL,"Incorrect all settings %s\n",rawname.c_str());
							GPRINT(NORMAL_LEVEL,"Restore factory %s\n",rawname.c_str());
							fact_sett=string_format("%s/%s.set",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_DEFAULT].c_str(),rawname.c_str());
							fact_sett_crc=string_format("%s/%s.crc",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_DEFAULT].c_str(),rawname.c_str());
							CopyFile(fact_sett,sett);
							CopyFile(fact_sett_crc,sett_crc);
							CopyFile(fact_sett,sett_rsv);
							CopyFile(fact_sett_crc,sett_rsv_crc);

							CopyFile(fact_sett,ex_sett);
							CopyFile(fact_sett_crc,ex_sett_crc);
							CopyFile(fact_sett,ex_sett_rsv);
							CopyFile(fact_sett_crc,ex_sett_rsv_crc);
							if (CheckJSONInStorage(sett,sett_crc,rdata)==ERROR)
							{
								RemoveFile(sett);
								RemoveFile(sett_crc);
								GPRINT(NORMAL_LEVEL,"Factory not restored, critical error, storage is corrupted, go to service\n");
							}
							else
							{
								GPRINT(NORMAL_LEVEL,"Factory restored, ok\n");
							}
						}
						else
						{
							GPRINT(NORMAL_LEVEL,"Correct extend rsv setting %s\n",ex_sett_rsv.c_str());
							GPRINT(NORMAL_LEVEL,"restore  %s to %s\n",ex_sett_rsv.c_str(),sett.c_str());
							GPRINT(NORMAL_LEVEL,"    copy %s to %s\n",ex_sett_rsv_crc.c_str(),sett_crc.c_str());
							CopyFile(ex_sett_rsv,sett);
							CopyFile(ex_sett_rsv_crc,sett_crc);
							GPRINT(NORMAL_LEVEL,"restore extend  %s to %s\n",ex_sett_rsv.c_str(),ex_sett.c_str());
							GPRINT(NORMAL_LEVEL,"    copy %s to %s\n",ex_sett_rsv_crc.c_str(),ex_sett_crc.c_str());
							CopyFile(ex_sett_rsv,ex_sett);
							CopyFile(ex_sett_rsv_crc,ex_sett_crc);
							GPRINT(NORMAL_LEVEL,"restore extend  %s to %s\n",ex_sett_rsv.c_str(),sett_rsv.c_str());
							GPRINT(NORMAL_LEVEL,"    copy %s to %s\n",ex_sett_rsv_crc.c_str(),sett_rsv_crc.c_str());
							CopyFile(ex_sett_rsv,sett_rsv);
							CopyFile(ex_sett_rsv_crc,sett_rsv_crc);

						}
					}
					else
					{
					GPRINT(NORMAL_LEVEL,"Correct rsv setting %s\n",sett_rsv.c_str());
					GPRINT(NORMAL_LEVEL,"restore  %s to %s\n",sett_rsv.c_str(),sett.c_str());
					GPRINT(NORMAL_LEVEL,"    copy %s to %s\n",sett_rsv_crc.c_str(),sett_crc.c_str());
					CopyFile(sett_rsv,sett);
					CopyFile(sett_rsv_crc,sett_crc);
					GPRINT(NORMAL_LEVEL,"restore extend  %s to %s\n",sett_rsv.c_str(),ex_sett.c_str());
					GPRINT(NORMAL_LEVEL,"    copy %s to %s\n",sett_rsv_crc.c_str(),ex_sett_crc.c_str());
					CopyFile(ex_sett_rsv,ex_sett);
					CopyFile(ex_sett_rsv_crc,ex_sett_crc);
					}

				}
				else
				{
				GPRINT(NORMAL_LEVEL,"Correct extend setting %s\n",ex_sett.c_str());
				GPRINT(NORMAL_LEVEL,"restore  %s to %s\n",ex_sett.c_str(),sett.c_str());
				GPRINT(NORMAL_LEVEL,"    copy %s to %s\n",ex_sett_crc.c_str(),sett_crc.c_str());
				CopyFile(ex_sett,sett);
				CopyFile(ex_sett_crc,sett_crc);
				}
			}
			else
			{
			GPRINT(NORMAL_LEVEL,"Correct CRC copy %s to %s\n",sett.c_str(),sett_rsv.c_str());
			GPRINT(NORMAL_LEVEL,"            copy %s to %s\n",sett_crc.c_str(),sett_rsv_crc.c_str());
			CopyFile(sett,sett_rsv);
			CopyFile(sett_crc,sett_rsv_crc);
			}

		}
	return NO_ERROR;
	}

eErrorTp SettingsAdm::SyncGsetSettings(u32 SyncType)
			{
				DIR *dp;
				struct dirent *entry;
				struct stat statbuf;
				u32 numberOfEntries=0;
				string fname;

				string set;
				string JSONstr;
				u32 len;

					if ((dp = opendir(CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST].c_str())) != NULL) {
								while((entry = readdir(dp)) != NULL) {

									if (strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
										continue;

									fname=entry->d_name;
									//lstat(fname.c_str(), &statbuf);
									if ((fname.compare(RESERVED_PATH_NAME)!=0)&&((GetExtensionStr(fname,set))==NO_ERROR)&&(set.compare("set")==0))
									{
										size_t lastindex = fname.find_last_of(".");
										string rawname = fname.substr(0, lastindex);

										if (SyncType==0)
											SyncGsetOneSetting4x4(rawname);
										else
											SyncGsetOneSettingLazy(rawname);

									}

								}
								closedir(dp);
					}
				//}

				return NO_ERROR;

			}

string SettingsAdm::calcDataSign(string & data){
	string str=data+CnT->deviceUID+"gorchakovFounder";
	str[0]=str[0]^0xff;
	string result= md5(str);
	printf("data %s\nsign %s\n",data.c_str(),result.c_str());

	return result;
}

eErrorTp SettingsAdm::initSign(string & settFile,string & settDefFile,string & signFile, string & crcFile ){
		remove(settFile.c_str());
		remove(crcFile.c_str());
		if (CopyFile(settDefFile,settFile)==NO_ERROR){
			string data;
			ReadStringFile((char*)settDefFile.c_str(),data);
			string signData=calcDataSign(data);
			WriteStringFile((char*)signFile.c_str(),signData);
		}
		else
			return ERROR;
		return NO_ERROR;
	}

eErrorTp SettingsAdm::checkSignSetting(string & settingName){
	string settFile=sf("%s/%s%s",CnT->SETTING_STORAGE_MOUNTP[0].c_str(),settingName.c_str(),SETTING_EXTENSION);
	string crcFile=sf("%s/%s%s",CnT->SETTING_STORAGE_MOUNTP[0].c_str(),settingName.c_str(),SETTING_EXTENSION_CRC);
	string settDefFile=sf("%s/%s%s",CnT->SETTING_STORAGE_MOUNTP[2].c_str(),settingName.c_str(),SETTING_EXTENSION);
	string signFile=sf("%s/%s%s",CnT->SETTING_STORAGE_MOUNTP[0].c_str(),settingName.c_str(),SETTING_EXTENSION_SIGN);
	if (existsSync(signFile)==NO_ERROR){
		zr();
		string data;
		ReadStringFile((char*)settFile.c_str(),data);
		string signDataCalc=calcDataSign(data);
		string signDataFromFile;
		ReadStringFile((char*)signFile.c_str(),signDataFromFile);
		if (signDataFromFile!=signDataCalc){
			zr();
			initSign(settFile,settDefFile,signFile,crcFile);
			return ERROR;
		}
	}
	else{
		zr();
		initSign(settFile,settDefFile,signFile,crcFile);
		return ERROR;
	}
	//exit(1);
	return NO_ERROR;
}

eErrorTp SettingsAdm::InitGset(void)
			{
				string path1=string_format("%s/%s",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST].c_str(),RESERVED_PATH_NAME);
				string path2=string_format("%s/%s",CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED].c_str(),RESERVED_PATH_NAME);

				if (SearchFile(path1.c_str())==ERROR)
				{
					GPRINT(NORMAL_LEVEL,"Mkpath reserved %s\n",path1.c_str());
					MkPath(path1.c_str(), 0xffffffff);
				}
				if (SearchFile(path2.c_str())==ERROR)
				{
					GPRINT(NORMAL_LEVEL,"Mkpath reserved %s\n",path1.c_str());
					MkPath(path2.c_str(), 0xffffffff);
				}

				SyncGsetSettings(0);

				return NO_ERROR;
			}

