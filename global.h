/*
 * global.h
 *
 *  Created on: 16 дек. 2018 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_GLOBAL_H_
#define SRVIOT_SRC_ENGINE_GLOBAL_H_

#include "basic.h"
#include "engine/lib/11p_files.h"
#include "engine/lib/11p_process.h"
#include "custom_project/project_var.h"
#include "engine/proto/json_proto.h"
#include "engine/update/update.h"
#include "engine/periphery/abstraction.h"
#ifdef _SECURE_ENABLE
	#include "engine/periphery/skeyV2.h"
#endif
//#include "guards.h"



#define RUN_MODE_NORMAL 0
#define RUN_MODE_SNIF 1
#define RUN_MODE_INPUT 2



#define JS_APPLY_SETTINGS "\"apply\""
#define JS_REQ_INFO "\"info\""
#define JS_INIT "\"init\""

#define CACHE_PATH_DEF "/var/run"
#define DEVID_PATH_DEF "/var/run/devid"
#define UNDELETE_FILE "undelete.set"
#define UPDATE_STUMP "updated"
#define FACTORY_MARKER "image_is_factory"
#define UPDATER_MARKER "image_is_updater"
#define ROLL_OUT_FACTORY_STUMP "rollOutToFactory"


//#define DEBUG_UPDATE
#define SETTING_STORAGE_FIRST 0
#define SETTING_STORAGE_RESERVED 1
#define TOTAL_RW_STORAGE_FOR_SETTING 2

#define SETTING_STORAGE_DEFAULT 2

#define RESERVED_PATH_NAME "reserved"
#define SETTING_STORAGE_COUNT 3

#define SETTING_FILE_NAME "settings"
#define SETTING_FILE_NAME_TEST "settings.test"
#define ACCOUNT_FILE_NAME "account"

#define MAX_CORE_SUPPORT 16

class CpuInfo{
	public:
	eErrorTp init(void);
	void dump(void){
		string res=StyledWriteJSON(cpu_result);

		printf("<CPU info>----------\n");
		printf("%s\n",(char*)res.c_str());
		printf("--------------------\n");
	}
	Json::Value cpu_result;
	private:
	u32 cpu_idx=0;
/*
	u8 cpu_idx=0;
	vector<string> model_name;
	vector<string> BogoMIPS;
	vector<string> Features;
	vector<string> CPU_mplementer;
	vector<string> CPU_architecture;
	vector<string> CPU_variant;
	vector<string> CPU_part;
	vector<string> CPU_revision;
	vector<string> Hardware;
	vector<string> Revision;
	vector<string> Serial;
	vector<string> model;
	vector<string> cpu_family;*/
	/*
	void dump(void){
			printf("--------------------\n");
			if (inited){
				printf("  cpu [%s] hw [%s] version_hw [%s] version_major [%s] swvers [%s]\n",
					(char*)cpu.c_str(),(char*)hw.c_str(),(char*)version_hw.c_str(),(char*)version_major.c_str(),(char*)swvers.c_str());
				printf("  swbuild [%s]\n",(char*)swbuild.c_str());
				printf("  hwvers [%s] swdate [%s] cpu_count [%s] hwmanuf [%s] hwmodel [%s]\n",
					(char*)hwvers.c_str(),(char*)swdate.c_str(),(char*)cpu_count.c_str(),(char*)hwmanuf.c_str(),(char*)hwmodel.c_str());
			}
			else{
				printf("Error dump distro info: not found disto cfg!!!\n");
			}
			printf("--------------------\n");
	}*/
};
class DistroInfo{
	public:
	eErrorTp init(void);
	void dump(void){
		printf("<Distro info>--------\n");
		if (inited){
			printf("  cpu [%s] hw [%s] version_hw [%s] version_major [%s] swvers [%s]\n",
				(char*)cpu.c_str(),(char*)hw.c_str(),(char*)version_hw.c_str(),(char*)version_major.c_str(),(char*)swvers.c_str());
			printf("  swbuild [%s]\n",(char*)swbuild.c_str());
			printf("  hwvers [%s] swdate [%s] cpu_count [%s] hwmanuf [%s] hwmodel [%s]\n",
				(char*)hwvers.c_str(),(char*)swdate.c_str(),(char*)cpu_count.c_str(),(char*)hwmanuf.c_str(),(char*)hwmodel.c_str());
		}
		else{
			printf("Error dump distro info: not found disto cfg!!!\n");
		}
		printf("--------------------\n");
	}
	string cpu;
	string hw;
	string version_hw;
	string version_major;
	string swvers;
	string swbuild;
	string hwvers;
	string swdate;
	string cpu_count;
	string hwmanuf;
	string hwmodel;
	bool webJnodaUsePrivate=false;//используется функционал для неудалаяемой конфигурации
	string sn;
	string version="";//GLOBAL VERSION
	Json::Value websrvCfg;
	bool inited=false;
};


class CnodaT{
	public:

	eErrorTp Init(int argc,char *argv[]);
	eErrorTp LoadLanguageWordbook();
	eErrorTp ReadConfigConf(string config);
	SettingsAdm sm;
	CnodaT(){
	};
	~CnodaT(){

	}
	eErrorTp LoadPrivateFromStorage(void){
		//PRIVATE_PATH
		Json::Value json;
		Json::Value rjson;
		if (JSON_ParseFile(json,PRIVATE_PATH)!=ERROR){
			rjson["private"]=json;
			dnk_var->Merge(rjson);
		}

		return ERROR;
	}

	eErrorTp LoadDNKFromStorage(void){
		string dnk_set;
		char dnk_set_name[]="settings.dnk";
		//SettingsAdm sm;
		if (sm.LoadSetting(dnk_set,dnk_set_name)!=ERROR){
			if (dnk_var->MergeWOCreate_cb((char*)dnk_set.c_str(),NULL,NULL)==ERROR){
			//if (Customer.dnk->Merge((char*)dnk_set.c_str())==ERROR){
					printf("\n\n-------\n-------\n-------\n-------\n-------\n");
					printf("Error load setting %s, format error, dnk unconf\n",dnk_set_name);
					printf("-------\n-------\n-------\n-------\n-------\n\n");
					return ERROR;
			}
			else{
				printf("DNK loaded!!!\n");
				return NO_ERROR;
			}
		}
		else{
			printf("Error load setting %s, file not found dnk unconf\n",dnk_set_name);
			return ERROR;
		}

	}
	void Step(void);
	void SigHandler(int snum);
	void SigActionConfig(void);
	void Help(void);
	eErrorTp ParseOpts(int argc, char *argv[]);
	eErrorTp GetFirstRunState(){
		string fpath=CACHE_PATH+"/cnoda_run";
		if (existsSync(fpath)==ERROR){
			first_run=true;
			string data="";
			return CreateFile((char*)fpath.c_str(),data);
		}
		else
			first_run=false;
		return NO_ERROR;
		//first_run=true;

	}
	u8 SWTerminateReq=0;
	u8 SWTerminate=0;
	bool cpu_and_distro_dump=false;
	u32 debug=0;
	u32 dnk_loglevel=NORMAL_LEVEL;
	u8 RunMode=0;//0 - normal, 1 -  rx snif mode, 2 -  tx input mode
	Mutex_t ConsLogMutex;

	Mutex_t SettingSave;
	u16 NODE_PORT_CPP=11100;
	u16 NODE_PORT_NODEJS=11200;//Jnoda.js port
	bool trigResetDeviceToFactory=false;//триггер запуска процедуру отката к заводским настр.
	shared_ptr <DNKchainT> dnk_var;
	Json::Value lang;
	Json::Value sast_db;

	u16 WDT_PORT_CPP=11300;
	u16 WDT_PORT_NODEJS=11400;//main.js port
	//u32 FactoryResetEnable=1;

	//eDebugTp GLOBAL_LOG_LEVEL=NORMAL_LEVEL;
	//u8 DISABLE_OUT=0;
	string DNK_VAL_CFG="dnk_variables.json";
	string NECRON_PATH="/www/pages/necron";


	//string SHARED_LANG_LIB=NECRON_PATH+"/ui/visual/shared/liblng";
	//string PROJECT_LANG_LIB=NECRON_PATH+"/ui/visual/shared/liblng";
	string CNODA_PATH=NECRON_PATH+"/Cnoda";
	string SERVER_CONFIG_FILE=CNODA_PATH+"/Cnoda.json";
	string SETTING_STORAGE_MOUNTP[SETTING_STORAGE_COUNT]={"/www/pages/sys/","/www/pages/sys_ex/","/etc/necron/"};

	string NODE_GROUP_IP="239.100.100.1";
	string DOWNLOAD_PATH="/www/pages/download";
	string FIRMWARE_PATH="/www/pages/update";
	string FIRMWARE_NAME="firmware";
	string UPLOAD_CACHE=string(CACHE_PATH_DEF)+"/cache";
	string CACHE_PATH=CACHE_PATH_DEF;
	string CACHE_NECRON_PATH=CACHE_PATH+"/necron";
	string DEVID_PATH=DEVID_PATH_DEF;
	string PRIVATE_PATH=SETTING_STORAGE_MOUNTP[0]+"/private.set";
	string SAFE_LOGGER_CONFIG=CNODA_PATH+"/Cnoda.json";
	string deviceUID="";
	vector<string> signedConfigs;
	sPrjVar pvar;
	bool authorised=false;
	bool cnodakgUnlock=false;
	string licKey="";
	DistroInfo dinfo;
	CpuInfo cpuinfo;
	bool first_run=true;//если есть файл /var/run/cnoda_run, значит был старт cnoda
	bool reboot_state=false;
	Json::Value json_cfg;
	bool reqPrivate=false;
	string version="";

	//shared_ptr<ThreadCollector> tc;
};

extern shared_ptr<CnodaT> CnT;
void ResetDeviceToFactory(void);

template <typename T>
eErrorTp GetValueSAST(char * dnk_path,T & value){
	return CnT->dnk_var->GetValue(dnk_path,value);
}
template <typename T>
eErrorTp GetValueSAST(string setting_name,string field_name,T & value){
	string path=CnT->dnk_var->SAST_GetDNKPathBySettingField(CnT->sast_db,setting_name,field_name);
	if (path.length()!=0)
		return CnT->dnk_var->GetValue((char*)path.c_str(),value);
	else
		return ERROR;
}
//		dnk_var->SAST_GetDNKPathBySettingField(sast_db,string setting_name,string field_name)
//dnk_var->SetValue(name,value);

template <typename T>
void DumpDnkVar(T * dnk_path){
	 CnT->dnk_var->Dump((char*)dnk_path,false);
}



template <typename T>
eErrorTp SetValueSAST(char * dnk_path,T value){
	return CnT->dnk_var->SetValue(dnk_path,value);
}

template <typename T>
eErrorTp setDNK(string dnk_path,T value){
	return SetValueSAST((char*)dnk_path.c_str(),value);
}

template <typename T>
eErrorTp SetValueSAST(string setting_name,string field_name,T value){
	string path=CnT->dnk_var->SAST_GetDNKPathBySettingField(CnT->sast_db,setting_name,field_name);
	zr();
	if (path.length()!=0){
		printf("set patch [%s]\n",path.c_str());
		return CnT->dnk_var->SetValue((char*)path.c_str(),value);
	}
	else
		return ERROR;
}

eErrorTp emit(u8 msg,string str);
void DumpSAST(void);


#endif /* SRVIOT_SRC_ENGINE_GLOBAL_H_ */
