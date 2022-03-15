/*
 * custom.h
 *
 *  Created on: 18 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_CUSTOM_H_
#define SRC_ENGINE_CUSTOM_H_

#include "print.h"
#include "global.h"
#include "settings_adm.h"

class CustomBaseT{
	public:
	CustomBaseT(){
		SharedPrint.debug_level=MEDIUM_LEVEL;
		SharedPrint.SourceStr="User";
		SharedPrint.ObjPref="u";
	}
	virtual ~CustomBaseT(){};
	virtual eErrorTp Init_SAST(SettingsAdm * Sm){return ERROR;};
	virtual eErrorTp SyncSettings_SAST(SettingsAdm * Sm){return ERROR;};
	virtual eErrorTp jnodaCMD_System_Settings(string savename,string json){return ERROR;};
	virtual eErrorTp jnodaCMD_Event(string & action,string & json){return ERROR;};

	 eErrorTp SetValue(string name,string data);
	 eErrorTp SetValue(string name,u32 data)
	{
		return SetValue(name,sf("%u",data));
	}

	 eErrorTp SetValue(string name,int data)
	{
		return SetValue(name,sf("%d",data));
	}
	GprintT SharedPrint;
	sSaved OldSaved;
	vector<string> signedConfigs;//список конфигов которые обязаны иметь подпись, пример settings.vpn
	//если подпись отсутствует, конфиг восстанавливается на дефолтный
	//список задается в Customer Init_SAST

	SettingsAdm sm;
};



#endif /* SRC_ENGINE_CUSTOM_H_ */
