/*
 * abstraction.cxx
 *
 *  Created on: 17 июл. 2019 г.
 *      Author: root
 */



#include "abstraction.h"


	 //eErrorTp SetValue(char * name, double & value);
	 //eErrorTp SetValue(char * name,float & value);
#if 0
eErrorTp DNKchainT::SetValue(char * name,int & value){

	JSON_SetValueOnStringPath(root,name,value);
	// JSON_SetValueOnStringPath(Json::Value & root,char * path,T value)
	return NO_ERROR;
}
#endif
	 //eErrorTp SetValue(char * name,string & value);


eErrorTp DNKchainT::DNKlink(Json::Value & link_root){

				for (const auto& key : link_root.getMemberNames()) {
					if (link_root[key].isObject()) {
						if (link_root[key].isMember("#")){
							string inherit=link_root[key]["#"]["inherit"].asString();
							//printf("Link %s\n",inherit.c_str());
							string inherit_path=json_skel_path+'/'+inherit;
							std::ifstream skel(inherit_path, std::ifstream::binary);
							string json((std::istreambuf_iterator<char>(skel)),std::istreambuf_iterator<char>());
							//printf("read %s %s",string(json_skel_path+inherit).c_str(),json.c_str());
							Json::Value inh_root;
							Json::Reader reader;


							if (reader.parse(json , inh_root )){
								DNKlink(inh_root);
								for (const auto& keym : inh_root.getMemberNames()) {
									//printf("%s %s\n",key.c_str(),keym.c_str());
									if (inh_root[keym].isObject())
										JSON_Merge(link_root[key][keym],inh_root[keym]);
									else
										link_root[key][keym]=inh_root[keym];
								}
							}
							else{
								gp.GPRINT(NORMAL_LEVEL,"DNKlink:Error parse inherit %s\n",inherit_path.c_str());
							}
							if (link_root[key]["#"].isMember("algo")){
								if (link_root[key]["#"]["algo"].isMember("cmd+=")){
									//Increment CMD algo, for array
									//u32 inc=stoul(link_root[key]["#"]["algo"]["cmd+="].asString());
									DNKlink_algo_IncCMD(link_root[key],stoul(link_root[key]["#"]["algo"]["cmd+="].asString(),0,16));
								}
							}
						}
						else
							DNKlink(link_root[key]);
					}
				}

		return NO_ERROR;
	}
eErrorTp DNKchainT::GetValue(char * name, double & result){
	  		Json::Value roott;
	  		eErrorTp err=ERROR;
	  		JSON_GetObjectOnStringPath(root,roott,name);
 	  		if (roott.isNull())
 	  			return err;

	  		if ((!roott.isObject())&&(!roott.isArray())){
	  			//try{
	  			if (roott.isDouble()){
	  				result=roott.asDouble();
	  			}
	  			else{
	  				result=stod(roott.asString());
	  			}
	  			err=NO_ERROR;
	  			//}catch(std::exception &ex){
	  				//err=ERROR;
	  				//gp.GPRINT(NORMAL_LEVEL,"GetValue double, exept %s\n",ex.what());
	  			//}
	  		}
	  		return err;
	  	}

eErrorTp DNKchainT::GetValue(char * name, Json::Value & result){
	  		Json::Value roott;
	  		eErrorTp err=ERROR;

	  		JSON_GetObjectOnStringPath(root,roott,name);
 	  		if (roott.isNull())
 	  			return err;
	  		//if ((!roott.isObject())&&(!roott.isArray())){
	  		result=roott;
	  		err=NO_ERROR;

	  		//}
	  		return err;
	  	}



	Json::Value  DNKchainT::GetValue(char * name){
	  		Json::Value roott;

	  		JSON_GetObjectOnStringPath(root,roott,name);

	  		//if ((!roott.isObject())&&(!roott.isArray())){
	  		return roott;
	  	}

	Json::Value  DNKchainT::GetValueS(string name){
	  		return GetValue((char*)name.c_str());
	  	}

	double  DNKchainT::GetValueDouble(char * name){
	  		Json::Value roott;

	  		JSON_GetObjectOnStringPath(root,roott,name);

	  		if ((!roott.isObject())&&(!roott.isArray())&&roott.isDouble())
	  			return roott.asDouble();

	  		return -1;
	  	}
	u32  DNKchainT::GetValueU32(char * name){
			Json::Value roott;

			JSON_GetObjectOnStringPath(root,roott,name);

			if ((!roott.isObject())&&(!roott.isArray())){
				if (roott.isUInt())
					return roott.asUInt();
				if (roott.isString()){
					string vs=roott.asString();
					if (regexMatch("^[0-9]+$",vs)==NO_ERROR){
						return stoul(vs);
					}
				}
			}
			return -1;
	}
    u64  DNKchainT::GetValueU64(char * name){
        Json::Value roott;

        JSON_GetObjectOnStringPath(root,roott,name);

        if ((!roott.isObject())&&(!roott.isArray())){
            if (roott.isUInt64())
                return roott.asUInt64();
            if (roott.isString()){
                string vs=roott.asString();
                if (regexMatch("^[0-9]+$",vs)==NO_ERROR){
                    return stoull(vs);
                }
            }
        }
        return -1;
    }
    s64  DNKchainT::GetValueS64(char * name){
        Json::Value roott;

        JSON_GetObjectOnStringPath(root,roott,name);

        if ((!roott.isObject())&&(!roott.isArray())){
            if (roott.isInt64())
                return roott.asInt64();
            if (roott.isString()){
                string vs=roott.asString();
                if (regexMatch("^[0-9]+$",vs)==NO_ERROR){
                    return stoll(vs);
                }
            }
        }
        return -1;
    }
	s32  DNKchainT::GetValueS32(char * name){
				Json::Value roott;

				JSON_GetObjectOnStringPath(root,roott,name);

				if ((!roott.isObject())&&(!roott.isArray())){
					if (roott.isUInt())
						return roott.asInt();
					if (roott.isString()){
						string vs=roott.asString();
						if (regexMatch("^[-]?[0-9]+$",vs)==NO_ERROR){
							return (s32)stol(vs);//int - 32b, long - 64b
						}
					}
				}
				return -1;
		}
	string  DNKchainT::GetValueString(char * name){
				Json::Value roott;

				JSON_GetObjectOnStringPath(root,roott,name);

				if ((!roott.isObject())&&(!roott.isArray())&&roott.isString())
					return roott.asString();

				return "";
		}
	 eErrorTp DNKchainT::GetValue(char * name,float & result){
	  		Json::Value roott;
	  		eErrorTp err=ERROR;

	  		JSON_GetObjectOnStringPath(root,roott,name);
 	  		if (roott.isNull())
 	  			return err;

	  		if ((!roott.isObject())&&(!roott.isArray())){
	  			//result=roott.asFloat();
	  			if (roott.isDouble())
	  				 result=roott.asFloat();
	  			else
	  				 result=stof(roott.asString());

	  			err=NO_ERROR;
	  		}
	  		return err;
	  	}


	 eErrorTp DNKchainT::GetValue(char * name,int & result){
	 	  		Json::Value roott;
	 	  		eErrorTp err=ERROR;
	 	  		JSON_GetObjectOnStringPath(root,roott,name);
	 	  		if (roott.isNull())
	 	  			return err;

	 	  		if ((!roott.isObject())&&(!roott.isArray())){
	 	  			if (roott.isInt())
	 	  				 result=roott.asInt();
	 	  			else
	 	  				 result=stoi(roott.asString());

	 	  			err=NO_ERROR;
	 	  		}
	 	  		return err;
	 	  	}
	 eErrorTp DNKchainT::GetValue(char * name,u32 & result){
	 	  		Json::Value roott;
	 	  		eErrorTp err=ERROR;

	 	  		JSON_GetObjectOnStringPath(root,roott,name);
	 	  		if (roott.isNull())
	 	  			return err;

	 	  		if ((!roott.isObject())&&(!roott.isArray())){
	 	  			if (roott.isUInt())
	 	  				result=roott.asUInt();
	 	  			else
	 	  				result=stol(roott.asString());

	 	  			err=NO_ERROR;
	 	  		}
	 	  		return err;
	 	  	}

	 eErrorTp DNKchainT::GetValue(char * name,string & result){
	 	  		Json::Value roott;
	 	  		eErrorTp err=ERROR;

	 	  		JSON_GetObjectOnStringPath(root,roott,name);
	 	  		if (roott.isNull())
	 	  			return err;

	 	  		if ((!roott.isObject())&&(!roott.isArray())){
	 	  			result=roott.asString();
	 	  			err=NO_ERROR;
	 	  		;
	 	  		}

	 	  		return err;
	 	  	}

		eErrorTp DNKchainT::AttachDNKTransfer(DNKtransferBaseT * trans){
			if (transfer!=NULL){
				gp.GPRINT(NORMAL_LEVEL,"Error:DNKtransfer attach fault\n");
				return ERROR;
			}
			else{
				transfer=trans;
				transfer->init();
				gp.GPRINT(NORMAL_LEVEL,"DNKtransfer attached\n");
				return NO_ERROR;
			}
		}

		void DNKchainT::Dump(char * path,bool show_spec=false){
			//Json::StyledWriter writer;
			Json::Value roott;

			JSON_GetObjectOnStringPath(root,roott,path);

			std::string outputConfig;
			if (show_spec==false){
				Json::Value nroott;
				JSON_DeepRemoveSpecObjs(roott,nroott);
				roott=nroott;
			}

			outputConfig = StyledWriteJSON(roott );
			gp.GPRINT(NORMAL_LEVEL," JSON DUMP path[%s]\n %s\n",path,outputConfig.c_str());


			//printf("str %s\n",root.get("x.ws.store3").asString().c_str());
		}

		eErrorTp DNKchainT::GetElementWithPath(char * path,string & outputConfig){

			//Json::FastWriter writer;
			Json::Value roott,setvroot,filtered_root;

			JSON_GetObjectOnStringPath(root,roott,path);
			JSON_DeepRemoveSpecObjs(roott,filtered_root);
			JSON_SetValueOnStringPath(setvroot,path,filtered_root);

			outputConfig  = FastWriteJSON(setvroot);//writer.write( setvroot );
			if (outputConfig=="null\n"){
				outputConfig="{}";
				return ERROR;
			}

			return NO_ERROR;
		}

		eErrorTp DNKchainT::GetElement(char * path,string & outputConfig){

				//Json::FastWriter writer;
				Json::Value roott,filtered_root;
				JSON_GetObjectOnStringPath(root,roott,path);
				JSON_DeepRemoveSpecObjs(roott,filtered_root);
				outputConfig  = FastWriteJSON(filtered_root);//writer.write( filtered_root );
				if (outputConfig=="null\n"){
					outputConfig="{}";
					return ERROR;
				}

				return NO_ERROR;
			}
		eErrorTp DNKchainT::GetElement(char * path,Json::Value & outputConfig){
						//Json::FastWriter writer;
						Json::Value roott;
						JSON_GetObjectOnStringPath(root,roott,path);
						JSON_DeepRemoveSpecObjs(roott,outputConfig);;
						return NO_ERROR;
				}

		eErrorTp DNKchainT::GetElement(string path,Json::Value & outputConfig){
						return GetElement((char*)path.c_str(),outputConfig);
		}
			eErrorTp DNKchainT::Merge(Json::Value & json_from){
					return JSON_Merge(root,json_from);
				}

			eErrorTp DNKchainT::Merge(char * json){
				Json::Reader reader;
				Json::Value  roott;
				if (reader.parse(json , roott )){
						gp.GPRINT(NORMAL_LEVEL,"parse Ok\n");
						JSON_Merge(root,roott);
						//root["x"]=roott["x"];
				}
				else
				{
					gp.GPRINT(NORMAL_LEVEL,"Merge error parse\n");
				}

				return NO_ERROR;
			}
			//Смержить без создания новых обьектов, если в приемнике не будет обьекта, функция вернет ошибку
			eErrorTp DNKchainT::MergeWOCreate_cb(char * json,int (*cb)(string merge_vname,string merge_value,void * cb_context),void * cb_context){
				//Obsolete Version, will remove 2021
				Json::Reader reader;
				Json::Value  roott;
				string path="";
				if (reader.parse(json , roott )){
						gp.GPRINT(NORMAL_LEVEL,"parse Ok\n");
						if (JSON_MergeWOCreate_cb(root,roott,cb,cb_context,path)==ERROR){
							return ERROR;
						}
						//root["x"]=roott["x"];
				}
				else
				{
					gp.GPRINT(NORMAL_LEVEL,"Merge error parse\n");
					return ERROR;
				}

				return NO_ERROR;
			}

			eErrorTp DNKchainT::Merge(char * json,int (*cb)(string merge_vname,Json::Value & merge_value,void * cb_context),void * cb_context){
				//New Version
				Json::Reader reader;
				Json::Value  roott;
				string path="";
				if (reader.parse(json , roott )){
						gp.GPRINT(NORMAL_LEVEL,"parse Ok\n");
						if (JSON_MergeWOCreate_cb(root,roott,cb,cb_context,path)==ERROR){
							return ERROR;
						}
						//root["x"]=roott["x"];
				}
				else
				{
					gp.GPRINT(NORMAL_LEVEL,"Merge error parse\n");
					return ERROR;
				}

				return NO_ERROR;
			}

			eErrorTp DNKchainT::SendPartDNK(char * path)
			{
				string result="{}";
				GetElementWithPath(path,result);
				if (transfer!=NULL)
					return transfer->send((u8*)result.c_str(),result.size());
				else {
					gp.GPRINT(NORMAL_LEVEL,"Error:transfer==NULL, please check it\n");
					return ERROR;
				}
			}
