/*
 * settings_adm.h
 *
 *  Created on: 22 дек. 2018 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_SETTINGS_ADM_H_
#define SRVIOT_SRC_ENGINE_SETTINGS_ADM_H_


#include "engine/algo/crc.h"
#include "engine/print.h"
#include "engine/lib/11p_string.h"
#include "engine/proto/json_proto.h"

class SettingsAdm : public GprintT {
	public:
	SettingsAdm (void);
	void SetLogLevel(eDebugTp dlevel);
	eErrorTp LoadSetting(string & JSONstr,string setting);
	eErrorTp LoadSetting(Json::Value & JSON,string setting);
	string GetSettingsName(char * virtdev);
	eErrorTp LoadJSONInStorage(string & JSONstr,char * filename);
	eErrorTp ParseSettingInStorage(Json::Value & root,char * filename);
	eErrorTp GetSettingValue(Json::Value & root,string name,string & value);
	std::pair <eErrorTp,bool> ChangeReplaceSettingValueN(Json::Value & root,string name,string value);
	std::pair <eErrorTp,bool> ChangeSettingValueN(Json::Value & root,string name,string value);
	std::pair <eErrorTp,bool> ChangeSettingValueN(Json::Value & root,string name,u32 value);
	std::pair <eErrorTp,bool> ChangeSettingValueN(Json::Value & root,string name,int value);
	//eErrorTp ChangeSettingValue(Json::Value & root,string name,string value);
	//eErrorTp ChangeSettingValue(Json::Value & root,string name,u32 value);

	eErrorTp SaveSettingToStorage(Json::Value & root,string filename);
	eErrorTp CheckSaved(string filename,u32 crc32);
	eErrorTp SaveSetting(string JSONstr,string filename);
	eErrorTp CheckJSONInStorage(string & data_file, string & crc_file,string & rdata);
	eErrorTp RemoveFile(string & file);
	eErrorTp CopyFile(string & FileNameSrc, string & FileNameDst);
	eErrorTp RemoveFromSetting4x4(char * filename);
	eErrorTp SyncGsetOneSetting4x4(string & rawname);
	eErrorTp SyncGsetOneSettingLazy(string & rawname);
	eErrorTp SyncGsetSettings(u32 SyncType);
	eErrorTp InitGset(void);
	string calcDataSign(string & data);
	eErrorTp initSign(string & settFile,string & settDefFile,string & signFile, string & crcFile );
	eErrorTp checkSignSetting(string & settingName);
//if (std::is_same_v<type, Animal>)



	template <typename T>
	eErrorTp ChangeSettingValue(string setting_name,string value_name,T value)
	{
		Json::Value root;
		if (ParseSettingInStorage(root,(char*)setting_name.c_str())==NO_ERROR)
		{
			std::pair <eErrorTp,bool> res=ChangeSettingValueN(root,value_name.c_str(),value);
			printf("CHANGE %s %s errorst %d is save? %d\n",setting_name.c_str(),value_name.c_str(),res.first,res.second);
			if ((res.first==NO_ERROR)){
				if (res.second==true)
					return SaveSettingToStorage(root,setting_name.c_str());
				else
					return NO_ERROR;
			}

		}
		return ERROR;
	}
};

//eErrorTp ResetDeviceToFactory(void);



#endif /* SRVIOT_SRC_ENGINE_SETTINGS_ADM_H_ */
