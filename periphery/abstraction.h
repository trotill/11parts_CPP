/*
 * demagogue.h
 *
 *  Created on: 11 июл. 2019 г.
 *      Author: root
 */

#ifndef SRC_CUSTOM_PROJECT_ABSTRACTION_H_
#define SRC_CUSTOM_PROJECT_ABSTRACTION_H_

#include <engine/types.h>
#include <engine/basic.h>
#include <engine/proto/json_proto.h>
#include <engine/print.h>
#include <engine/settings_adm.h>

#include "engine/memadmin.h"
#include "engine/lib/11p_bin.h"
#include "engine/lib/11p_time.h"
#include "engine/lib/11p_string.h"
#include "engine/algo/base64.h"
//#include "custom_project/project_var.h"

typedef enum {enutCLASSIC} enDNK_MC_Protocol;
#define DNK_MAX_VALUE_SIZE 1000

#define DOUBLE_TYPE 0
#define STRING_TYPE 1
#define U32_TYPE 2
#define INT_TYPE 3
#define FLOAT_TYPE 4
#define JSON_TYPE 5
#define UNDEF_TYPE 6

class DNKtransferBaseT{
	public:
	virtual eErrorTp init(void){
			printf("DNKtransferBaseT Please override init call\n");
			return ERROR;
		}
	virtual eErrorTp send(u8 * buf,u32 bufsize){
			printf("DNKtransferBaseT Please override send call\n");
			return ERROR;
		}
	virtual ~DNKtransferBaseT(){};
};
class DNKchainT{
	public:
	eErrorTp DNKlink_algo_IncCMD(Json::Value & inc_root,u32 inc);
	eErrorTp AttachDNKTransfer(DNKtransferBaseT * trans);
	eErrorTp DNKlink(Json::Value & link_root);
	//json_skel_path - путь к dnk конфигам
	//json_skel_root - имя файла главного конфига DNK
	//trans - клас для отправки и приема данных, используется для синхронной отправке при получения и сохранении значений
	DNKchainT(eDebugTp log_level,string json_skel_path,string json_skel_root,eErrorTp & err){
		Json::Value root_wbook;
		Json::Reader reader;

		gp.debug_level=log_level;
		sadm.SetLogLevel(log_level);
		gp.SourceStr="dnk";
		gp.ObjPref=json_skel_root;

		this->json_skel_path=json_skel_path;
		err=ERROR;
		string glskel=json_skel_path+'/'+json_skel_root;
		try{

			std::ifstream global_skel(glskel, std::ifstream::binary);
			string json((std::istreambuf_iterator<char>(global_skel)),
					std::istreambuf_iterator<char>());
			bool parsingSuccessful = reader.parse(json , root );
			if (parsingSuccessful) {
				gp.GPRINT(NORMAL_LEVEL,"%s parsingSuccessful\n",glskel.c_str());
				//Json::Value & tmp_root=root;
				DNKlink(root);
				default_root=root;
				}
				else{
					gp.GPRINT(NORMAL_LEVEL,"%s parsingError\n",glskel.c_str());
					throw 1;
				}
		}catch(int err)
		{
			for (u32 n=0;n<10;n++){
				gp.GPRINT(NORMAL_LEVEL,"Error parse json skeleton %s!!!\n",glskel.c_str());
				sleep(1);
			}
			return;
			//exit(1);
		}
		err=NO_ERROR;

		return;
	}


	DNKchainT(eDebugTp log_level,Json::Value & config_data,string id,eErrorTp & err){
			Json::Value root_wbook;


			gp.debug_level=log_level;
			sadm.SetLogLevel(log_level);
			gp.SourceStr="dnk";
			gp.ObjPref=id;

			//Json::Value & tmp_root=root;
			root=config_data;
			err=DNKlink(root);
			default_root=root;
			return;
		}



	eErrorTp MC_ParsePackData_ConvValue(u8 * bin_value,u32 size,string algo,string & path,Json::Value & root_result);
	eErrorTp MC_ParsePackData(char * cmd_code,Json::Value & mc_cmd,u8 * buf,Json::Value & json_result,eErrorTp (*User_UnPackerValue)(u8 * bin_value,u32 size,string algo,string & path,Json::Value & main_root,Json::Value & root_result,void * cb_context),void * cb_context);
	eErrorTp MC_CreatePackData_ConvValue(u32 bitsize,string algo,Json::Value js_value, u8 * result);
	eErrorTp MC_Create_ProtoDB_Standart(char * target,Json::Value & mc_name,Json::Value & mc_cmd);
	u32 MC_CreatePackData(char * cmd_code,Json::Value & mc_cmd,u8 * buf, eErrorTp (*User_PackerValue)(u32,string,Json::Value,u8*,void * cb_context));

	eErrorTp SetChangeMarker(char * name);
	pair<eErrorTp,bool> GetChangeMarker(char * name);
	eErrorTp ClearChangeMarker(char * name);
	eErrorTp ClearAllChangeMarkers(void);
	void ShowChangeMarkers(void);
	eErrorTp SAST_Get_Settings(Json::Value & sast_db,Json::Value &result,bool only_changed);
	eErrorTp SAST_SaveChanged_Settings(Json::Value & sast_db);
	eErrorTp SAST_Sync_FilesDB_Standart(Json::Value & sast_db);
	eErrorTp SAST_Create_FilesDB_Standart(Json::Value & sast_db);
	string SAST_GetDNKPathBySettingField(Json::Value & sast_db,string setting_name,string field_name);
	eErrorTp RemoveValue(char * name) {
		gp.GPRINT(HARD_LEVEL,"RemoveValue name %s\n",name);
		Json::Value  gvalue;
		if (GetValue(name,gvalue)==NO_ERROR)
			return JSON_RemoveValueOnStringPath(root,name);
		else
			return ERROR;
	}

	template<typename T>
	eErrorTp SetValue(char * name,T & value){

		gp.GPRINT(HARD_LEVEL,"SetValue name %s\n",name);
		Json::Value  gvalue;

		GetValue(name,gvalue);
		if (gvalue!=value){
			JSON_SetValueOnStringPath(root,name,value);
			SetChangeMarker(name);
			return NO_ERROR;
		}
		// JSON_SetValueOnStringPath(Json::Value & root,char * path,T value)
		return ERROR;
	}
	template<typename T>
	eErrorTp SetValueRef(string name,T & value){
		return SetValue((char*)name.c_str(),value);
	}
	template<typename T>
	eErrorTp SetValue(string name,T value){
		T tmp=value;
		return SetValue((char*)name.c_str(),tmp);
	}

	template<typename T>
	eErrorTp SetAndSendValue(char * name,T & value){
		SetValue(name,value);
		SendPartDNK(name);
		return NO_ERROR;
	}

	template<typename T>
	eErrorTp SetValueC(char * name,T value){
		T val=value;
		return SetValue(name,val);
	}

	template<typename T>
	eErrorTp SetAndSendValueC(char * name,T value){
		T val=value;
		return SetAndSendValue(name,val);
	}

	void Dump(char * path,bool show_spec=false);
	eErrorTp SendPartDNK(char * path);
	eErrorTp GetElementWithPath(char * path,string & outputConfig);
	 Json::Value GetValue(char * name);
	Json::Value GetValueS(string name);
	 double  GetValueDouble(char * name);
	 u32  GetValueU32(char * name);
	 s32  GetValueS32(char * name);
     s64  GetValueS64(char * name);
     u64  GetValueU64(char * name);
	 string GetValueString(char * name);
	 eErrorTp GetValue(char * name, double & result);
	 eErrorTp GetValue(char * name,float & result);
	 eErrorTp GetValue(char * name,int & result);
	 eErrorTp GetValue(char * name,u32 & result);
	 eErrorTp GetValue(char * name,string & result);
	 eErrorTp GetValue(char * name, Json::Value & result);
	 eErrorTp GetElement(char * path,string & outputConfig);
	 eErrorTp GetElement(char * path,Json::Value & outputConfig);
	 eErrorTp GetElement(string path,Json::Value & outputConfig);
	 eErrorTp Merge(Json::Value & json_from);
	 eErrorTp Merge(char * json);
	 eErrorTp MergeWOCreate_cb(char * json,int (*cb)(string merge_vname,string merge_value,void * cb_context),void * cb_context);
	 eErrorTp Merge(char * json,int (*cb)(string merge_vname,Json::Value & merge_value,void * cb_context),void * cb_context);
	 Json::Value default_root;
	protected:
		Json::Value root;

		DNKtransferBaseT * transfer=NULL;
		Json::Value changed_val;
		u8 MC_ParsePackData_value_tmp[DNK_MAX_VALUE_SIZE];
		u8 MC_CreatePackData_value_tmp[DNK_MAX_VALUE_SIZE];
		SettingsAdm sadm;
		GprintT gp;
		Json::Value stm_to_json;
		Json::Value json_to_stm;
		string json_skel_path;
		eErrorTp GSAST_Json_Standart(Json::Value root_src,Json::Value & sast_db,string varname);
		void GMC_Json_Standart(char * target,Json::Value root_src,Json::Value & mc_name,Json::Value & mc_cmd,string varname);



};



#endif /* SRC_CUSTOM_PROJECT_ABSTRACTION_H_ */
