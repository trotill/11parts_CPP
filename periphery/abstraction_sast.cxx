/*
 * abstraction_sast.cxx
 *
 *  Created on: 20 янв. 2020 г.
 *      Author: root
 */



#include "abstraction.h"

	eErrorTp DNKchainT::SAST_Create_FilesDB_Standart(Json::Value & sast_db){
			string varname="";

			if (GSAST_Json_Standart(root,sast_db,varname)==NO_ERROR){
				 SAST_Sync_FilesDB_Standart(sast_db);
				 gp.GPRINT(HARD_LEVEL,"Sast_db \n%s\n",StyledWriteJSON(sast_db ).c_str());
				 ClearAllChangeMarkers();
				 return NO_ERROR;
			}
			else
				return ERROR;
		}
	eErrorTp DNKchainT::SAST_Sync_FilesDB_Standart(Json::Value & sast_db){

		Json::Value settings_data;
		for (const auto& virt_name : sast_db.getMemberNames()) {
			for (const auto& sett_name : sast_db[virt_name].getMemberNames()) {
				if (!settings_data.isMember(sett_name)){
					Json::Value JSON;
					if (sadm.LoadSetting(JSON,sett_name)!=ERROR)
						settings_data[sett_name] =JSON;
					//else
						//settings_data[sett_name][sast_db[virt_name][sett_name].asCString()]="0";
				}
				if (settings_data.isMember(sett_name)){
					string valname=sast_db[virt_name][sett_name].asCString();
					string value;
					//zr();
					if (sadm.GetSettingValue(settings_data[sett_name],valname,value)==NO_ERROR){
						//zr();
						//printf("sadm.GetSettingValue valname %s value %s\n",valname.c_str(),value.c_str());
						SetValue((char*)virt_name.c_str(),value);
					}
					else{
						string dnk_value;
						//zr();
						//printf("sadm.GetSettingValue valname %s dnk_value %s\n",valname.c_str(),dnk_value.c_str());
						GetValue((char*)virt_name.c_str(),dnk_value);
						settings_data[sett_name]["d"][valname]=dnk_value;
						gp.GPRINT(NORMAL_LEVEL,"Not found field %s in setting file %s, sync file from dnk value %s\n",valname.c_str(),sett_name.c_str(),dnk_value.c_str());
						sadm.SaveSettingToStorage(settings_data[sett_name],sett_name);
					}
					//zr();
				}

			}
		}

		//exit(1);
		return NO_ERROR;
	}

	eErrorTp DNKchainT::SAST_SaveChanged_Settings(Json::Value & sast_db){
		Json::Value result;
		Json::Value settings;
		if (SAST_Get_Settings(sast_db,result,true)!=ERROR){
			for (const auto& sett_file : result.getMemberNames()){
				for (const auto& valname : result[sett_file].getMemberNames()){
					string val=result[sett_file][valname].asString();
					string JSONstr;
					gp.GPRINT(HARD_LEVEL,"SAST_SaveChanged_Settings file[%s] field[%s] val[%s]\n",sett_file.c_str(),valname.c_str(),val.c_str());
					if (settings.isMember(sett_file)==false)
						sadm.LoadSetting(settings[sett_file],sett_file);
						//Json::Value sett;
						//sett["d"][valname]=val;
						//settings[sett_file]["d"][valname]=val;
						//sadm.SaveSettingToStorage(settings[sett_file],sett_file);
					//}
					//else
					settings[sett_file]["d"][valname]=val;
				}
			}
		}

		cout << StyledWriteJSON(settings) <<endl;
		for (const auto& sett_file : settings.getMemberNames()){
			gp.GPRINT(HARD_LEVEL,"Write setting to file %s\n",sett_file.c_str());
			sadm.SaveSettingToStorage(settings[sett_file],sett_file);
		}

		return NO_ERROR;
	}
	eErrorTp DNKchainT::SAST_Get_Settings(Json::Value & sast_db,Json::Value &result,bool only_changed){
			for (const auto& key : sast_db.getMemberNames()) {
				for (const auto& sett_file : sast_db[key].getMemberNames()){
					string value;

					if (GetValue((char*)key.c_str(),value)!=ERROR){
						if (only_changed){
							//printf("mark %s\n",key.c_str());
							auto res=GetChangeMarker((char*)key.c_str());

							if ((res.first!=ERROR)&&(res.second)){
								//printf("mark %d %d %s %s\n",res.first,res.second,sett_file.c_str(),sast_db[key][sett_file].asCString());
								result[sett_file][sast_db[key][sett_file].asCString()]=value;
								ClearChangeMarker((char*)key.c_str());
							}
						}
						else
							result[sett_file][sast_db[key][sett_file].asCString()]=value;

					}
				}
			}

			if (gp.debug_level>=HARD_LEVEL){
				gp.GPRINT(HARD_LEVEL,"SAST_Get_Settings result\n[%s]\n",StyledWriteJSON(result).c_str());
			}

			return NO_ERROR;
		}
	string DNKchainT::SAST_GetDNKPathBySettingField(Json::Value & sast_db,string setting_name,string field_name){
		for (const auto & key : sast_db.getMemberNames()){
			//printf("SAST::key %s\n",key.c_str());
			//zr();
			if (sast_db[key].isMember(setting_name)){
				//zr();
				if (sast_db[key][setting_name].asString()==field_name){
					//printf("return key %s\n",key.c_str());
					return key;
				}
			}
		}
		return "";
		//return
	}
	eErrorTp DNKchainT::GSAST_Json_Standart(Json::Value root_src,Json::Value & sast_db,string varname){
				for (const auto& key : root_src.getMemberNames()) {
					if (root_src[key].isObject()) {
						if ((key=="$")&&(root_src["$"].isMember("sast"))){
							for (const auto& keyf : root_src["$"]["sast"].getMemberNames()) {
								//keyf - virt var name
								//for (const auto& sett_file : root_src["$"]["sast"][keyf].getMemberNames()) {
								//	sast_db[sett_file][]
								//}
								string vn;
								vn=varname+'.'+keyf;

								sast_db[vn]=root_src["$"]["sast"][keyf];
							}
						}
						else{
							string nname;
							if (varname.size()>0)
								nname=varname+"."+key;
							else
								nname=key;
							GSAST_Json_Standart(root_src[key],sast_db,nname);
						}
					}
				}
				return NO_ERROR;
			}

	eErrorTp DNKchainT::SetChangeMarker(char * name){
			changed_val[name]=true;
			gp.GPRINT(HARD_LEVEL,"SetChangeMarker name %s\n",name);
			return NO_ERROR;
		}
		pair<eErrorTp,bool> DNKchainT::GetChangeMarker(char * name){
			eErrorTp err=ERROR;
			bool val=false;
			if (changed_val.isMember(name)){
				val=changed_val[name].asBool();
				err=NO_ERROR;
			}
			gp.GPRINT(HARD_LEVEL,"GetChangeMarker name %s\n",name);
			return make_pair(err,val);
		}
		eErrorTp DNKchainT::ClearChangeMarker(char * name){
			if (changed_val.isMember(name)){
					changed_val[name]=false;
				    return NO_ERROR;
			}
			gp.GPRINT(HARD_LEVEL,"ClearChangeMarker %s\n",name);
			return ERROR;
		}
		eErrorTp DNKchainT::ClearAllChangeMarkers(void){
			for (const auto& key : changed_val.getMemberNames()) {
				changed_val[key]=false;
			}
			gp.GPRINT(HARD_LEVEL,"ClearAllChangeMarkers\n");
			return NO_ERROR;
		}
		void DNKchainT::ShowChangeMarkers(void){
			gp.GPRINT(NORMAL_LEVEL," Changed markers\n %s\n",StyledWriteJSON(changed_val ).c_str());
		}
