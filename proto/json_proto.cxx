/*
 * json_proto.cxx
 *
 *  Created on: 27 февр. 2017 г.
 *      Author: root
 */

#include "json_proto.h"
#include "custom_project/project_var.h"
#include "engine/lib/11p_files.h"
#include "engine/settings_adm.h"


eErrorTp JSON_ParseFile(Json::Value & json,string filename)
{
	Json::Reader rd;
	string data;
	if (existsSync(filename)==NO_ERROR){
		ReadStringFile((char*)filename.c_str(),data);
		if (rd.parse(data.c_str(),json)){
			return NO_ERROR;
		}
		else
			return ERROR;
	}
	return ERROR;
}


Json::Value JSON_CreatePack(Json::Value d_obj,string sid){
	Json::Value rt_root;
	rt_root["t"][JSON_PACK_HEADER_PACK_TYPE]=1;
	rt_root["t"][JSON_PACK_HEADER_VERSION]=1;
	rt_root["d"]=d_obj;
	rt_root["sid"]=sid;
	return rt_root;
}

eErrorTp JSON_GetPackType(char * json,u32 & ptype,u32 & vers)
{
	Json::Value root;
	Json::Reader reader;
	eErrorTp err=ERROR;

	bool parsingSuccessful = reader.parse( json, root );
	if (parsingSuccessful)
	{
		if (root[JSON_HEADER_SYMB].isArray())
		{

			ptype=root[JSON_HEADER_SYMB][JSON_PACK_HEADER_PACK_TYPE].asInt();
			vers=root[JSON_HEADER_SYMB][JSON_PACK_HEADER_VERSION].asInt();
			err=NO_ERROR;
		}
	}
	return err;
}

eErrorTp JSON_GetFieldInDataMulti(char * json,char * fieldname,u8 num,string & data)
{
	char NStr[20];
	snprintf(NStr,sizeof(NStr),"%s%d",fieldname,num);
	return JSON_GetFieldInData(json,NStr,data);
}


eErrorTp JSON_GetFieldInData(Json::Value & json,char * fieldname,string & data)
{
	//Json::Value root;
	Json::Reader reader;
	eErrorTp err=ERROR;

	if (json[JSON_DATA_SYMB].isObject()&&(json[JSON_DATA_SYMB][fieldname].isNull()==false))
	{
		data=json[JSON_DATA_SYMB][fieldname].asString();
		err=NO_ERROR;
	}
	else{

		data=" ";
	}

	return err;
}

eErrorTp JSON_GetFieldInDataFromFile(string filename,string fieldname,string & data)
{
	Json::Value root;
	eErrorTp err=ERROR;
	SettingsAdm sm;

	 if (sm.LoadSetting(root,filename)==NO_ERROR){
		if (root[JSON_DATA_SYMB].isObject()&&(root[JSON_DATA_SYMB][fieldname].isNull()==false))
		{

			data=root[JSON_DATA_SYMB][fieldname].asString();
			err=NO_ERROR;
		}
		else
		{
			data=" ";
		}
	}


	return err;
}

eErrorTp JSON_GetFieldInData(char * json,char * fieldname,string & data)
{
	Json::Value root;
	Json::Reader reader;
	eErrorTp err=ERROR;

	bool parsingSuccessful = reader.parse( json, root );
	if (parsingSuccessful)
	{
		if (root[JSON_DATA_SYMB].isObject()&&(root[JSON_DATA_SYMB][fieldname].isNull()==false))
		{

			data=root[JSON_DATA_SYMB][fieldname].asString();
			err=NO_ERROR;
		}
		else
		{
			data=" ";
		}
	}


	return err;
}

eErrorTp JSON_ChangeFieldInData(string & json,char * fieldname,string data)
{
	Json::Value root;
	Json::Reader reader;
	eErrorTp err=NO_ERROR;

	bool parsingSuccessful = reader.parse( json.c_str(), root );
	if (parsingSuccessful)
	{
		if (root[JSON_DATA_SYMB].isObject()&&(root[JSON_DATA_SYMB][fieldname].isNull()==false))
		{

			root[JSON_DATA_SYMB][fieldname]=data;
			//Json::FastWriter writer;
			json=FastWriteJSON(root);//writer.write(root);
			printf("Correct field %s to %s\n",fieldname,data.c_str());

			//err=NO_ERROR;
		}
	}


	return err;
}

eErrorTp JSON_GetField(char * json,char * fieldname,string & data)
{
	Json::Value root;
	Json::Reader reader;
	eErrorTp err=ERROR;

	bool parsingSuccessful = reader.parse( json, root );
	if (parsingSuccessful)
	{
		if (root[fieldname].isNull()==false)
		{
			data=root[fieldname].asString();
			err=NO_ERROR;
		}
	}
	return err;
}

eErrorTp JSON_GetFieldInDataInfo(char * json,char * fieldname,string & info)
{
	Json::Value root;
		Json::Reader reader;
		eErrorTp err=ERROR;

		bool parsingSuccessful = reader.parse( json, root );
		if (parsingSuccessful)
		{
			if (root[JSON_DATA_SYMB].isObject()&&(root[JSON_DATA_SYMB][JSON_INFO_SYMB].isNull()==false))
			{
				info=root[JSON_DATA_SYMB][JSON_INFO_SYMB].asString();
				err=NO_ERROR;
			}
		}
		return err;
}

eErrorTp JSON_TestDataInfoInResp(char * json,char * fdatafield,char * fdata,char * finfo)
{
	Json::Value root;
	Json::Reader reader;
	eErrorTp err=ERROR;

	string jsdata;

	bool parsingSuccessful = reader.parse( json, root );
	if (parsingSuccessful)
	{
		if (root[JSON_DATA_SYMB].isObject()&&
				(root[JSON_DATA_SYMB][fdatafield].isNull()==false)&&
				(root[JSON_INFO_SYMB].isNull()==false)&&
				(root[JSON_INFO_SYMB][0].isNull()==false)&&
				(root[JSON_INFO_SYMB][0][finfo].isNull()==false))
		{
			jsdata=root[JSON_DATA_SYMB][fdatafield].asString();
			if (strcmp(jsdata.c_str(),fdata)==0)
			{

				err=NO_ERROR;
			}
		}
	}
	return err;
}


eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,char * var)
{
	if (root[fieldname].isNull()==false)
	{
		strcpy(var,root[fieldname].asString().c_str());
		return NO_ERROR;
	}
	return ERROR;
}

eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,Json::Value & var)
{
	if (root[fieldname].isNull()==false)
	{
		var=root[fieldname];
		//printf("name %s val %s\n",fieldname,var.c_str());
		return NO_ERROR;
	}
	return ERROR;
}

eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,string & var)
{
	if (root[fieldname].isNull()==false)
	{
		var=root[fieldname].asString();
		//printf("name %s val %s\n",fieldname,var.c_str());
		return NO_ERROR;
	}
	return ERROR;
}


eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,bool & var)
{
	if (root[fieldname].isNull()==false)
	{
		var=(bool)stoul(root[fieldname].asString());
		return NO_ERROR;
	}
	return ERROR;
}

eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,u8 & var)
{
	if (root[fieldname].isNull()==false)
	{
		var=(u8)stoul(root[fieldname].asString());
		return NO_ERROR;
	}
	return ERROR;
}

eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,u16 & var)
{
	if (root[fieldname].isNull()==false)
	{
		var=(u16)stoul(root[fieldname].asString());
		return NO_ERROR;
	}
	return ERROR;
}

eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,u32 & var)
{

	if (root[fieldname].isNull()==false)
	{
		var=stoul(root[fieldname].asString());
		return NO_ERROR;
	}
	return ERROR;
}

eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,u64 & var)
{

	if (root[fieldname].isNull()==false)
	{
		var=stoull(root[fieldname].asString());
		return NO_ERROR;
	}
	return ERROR;
}

eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,int & var)
{

	if (root[fieldname].isNull()==false)
	{
		var=stoi(root[fieldname].asString());
		return NO_ERROR;
	}
	return ERROR;
}

eErrorTp JSON_RemoveValueOnStringPath(Json::Value & root,char * path)
{
    istringstream iss(path);
    string object;

    string f[20];
    u32 n=0;
    while ( getline( iss, object, '.' ) ) {
        f[n++]=object;
    }
    // if (path[strlen(path)-1]=="]"){

    // }
    // cout << "path " << path << " value " << value << endl;
    switch (n)
    {
        case 1:root[f[0]].clear(); break;
        case 2:root[f[0]][f[1]].clear();break;
        case 3:root[f[0]][f[1]][f[2]].clear();break;
        case 4:root[f[0]][f[1]][f[2]][f[3]].clear();break;
        case 5:root[f[0]][f[1]][f[2]][f[3]][f[4]].clear();break;
        case 6:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]].clear();break;
        case 7:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]].clear();break;
        case 8:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]].clear();break;
        case 9:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]].clear();break;
        case 10:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]].clear();break;
        case 11:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]].clear();break;
        case 12:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]].clear();break;
        case 13:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]].clear();break;
        case 14:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]].clear();break;
        case 15:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]].clear();break;
        case 16:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]].clear();break;
        case 17:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]].clear();break;
        case 18:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]][f[17]].clear();break;
        case 19:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]][f[17]][f[18]].clear();break;
        case 20:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]][f[17]][f[18]][f[19]].clear();break;
        default:
            root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]][f[17]][f[18]][f[19]].clear();
            printf("ERROR JSON_GetObjectOnStringPath !!!\n");
            return ERROR;
    }
    // new_root=new_root.get(path);
    //new_root=src_root["x"]["ws"]["store3"]["mod"]["mod_cooler_public_t"]["fans_pause_time"];
    return NO_ERROR;
}

//Get new object on path type "a.b.c"
eErrorTp JSON_GetObjectOnStringPath(Json::Value & src_root,Json::Value & new_root,char * path)
{
	istringstream iss(path);
	string object;
#if 0
	new_root=src_root;
	 while ( getline( iss, object, '.' ) ) {
	    printf( "%s\n", object.c_str() );
	    new_root=new_root[object];
	  }
#endif

	 string f[20];
	 u32 n=0;
	 while ( getline( iss, object, '.' ) ) {
		 f[n++]=object;
		// printf("get %s\n",object.c_str());
	 }

	 switch (n)
	 {
	 	 case 0:new_root=src_root; break;
	 	 case 1:new_root=src_root[f[0]]; break;
	 	 case 2:new_root=src_root[f[0]][f[1]];break;
	 	 case 3:new_root=src_root[f[0]][f[1]][f[2]];break;
	 	 case 4:new_root=src_root[f[0]][f[1]][f[2]][f[3]];break;
	 	 case 5:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]];break;
	 	 case 6:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]];break;
	 	 case 7:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]];break;
	 	 case 8:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]];break;
	 	 case 9:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]];break;
	 	 case 10:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]];break;
	 	 case 11:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]];break;
	 	 case 12:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]];break;
	 	 case 13:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]];break;
	 	 case 14:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]];break;
	 	 case 15:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]];break;
	 	 case 16:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]];break;
	 	 case 17:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]];break;
	 	 case 18:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]][f[17]];break;
	 	 case 19:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]][f[17]][f[18]];break;
	 	 case 20:new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]][f[17]][f[18]][f[19]];break;
	 	 default:
	 		new_root=src_root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]][f[17]][f[18]][f[19]];
	 		printf("ERROR JSON_GetObjectOnStringPath !!!\n");
	 		return ERROR;
	 }
	// new_root=new_root.get(path);
	 //new_root=src_root["x"]["ws"]["store3"]["mod"]["mod_cooler_public_t"]["fans_pause_time"];
	return NO_ERROR;
}

eErrorTp JSON_Merge(Json::Value & root_to,Json::Value & root_from){
			for (const auto& key : root_from.getMemberNames()) {
					if (root_to[key].isObject()) {
						JSON_Merge(root_to[key],root_from[key]);
					} else {
						root_to[key] = root_from[key];
					}
			}
			return NO_ERROR;
}

eErrorTp JSON_MergeWOCreate_cb(Json::Value & root_to,Json::Value & root_from,int (*cb)(string merge_vname,Json::Value & merge_value,void * cb_context),void * cb_context,string path){
			//New version!!!
			string p=path;
			for (const auto& key : root_from.getMemberNames()) {
					if (root_from[key].isObject()) {
						if (JSON_MergeWOCreate_cb(root_to[key],root_from[key],cb,cb_context,p+'.'+key)==ERROR)
							return ERROR;
					} else {
						if (root_to.isMember(key)&&root_from.isMember(key)) {
							root_to[key] = root_from[key];
							//printf("merge %s\n",key.c_str());
							if (!root_from[key].isArray())
							{	//printf("key %s\n",(string(&p.c_str()[1])+'.'+key).c_str());
								if (cb!=NULL)
									cb(string(&p.c_str()[1])+'.'+key,root_from[key],cb_context);


							}
							else{
								//Json::FastWriter writer;
								if (cb!=NULL)
									cb(string(&p.c_str()[1])+'.'+key,root_from[key],cb_context);

							}
						}
						else{
							printf("JSON_Merge_cb undef key %s\n",key.c_str());
							if (root_to.isMember(key)==false)
								root_to[key]=root_from[key];

							continue;
						}
					}
			}

	return NO_ERROR;
}

eErrorTp JSON_MergeWOCreate_cb(Json::Value & root_to,Json::Value & root_from,int (*cb)(string merge_vname,string merge_value,void * cb_context),void * cb_context,string path){
			//Obsolete version, merge all as string!!!
			printf("JSON_MergeWOCreate_cb obsolete version, merge all as string!!!\n");
			string p=path;
			for (const auto& key : root_from.getMemberNames()) {
					if (root_from[key].isObject()) {
						if (JSON_MergeWOCreate_cb(root_to[key],root_from[key],cb,cb_context,p+'.'+key)==ERROR)
							return ERROR;
					} else {
						if (root_to.isMember(key)&&root_from.isMember(key)) {
							root_to[key] = root_from[key];
							//printf("merge %s\n",key.c_str());
							if (!root_from[key].isArray())
							{	//printf("key %s\n",(string(&p.c_str()[1])+'.'+key).c_str());
								if (cb!=NULL)
									cb(string(&p.c_str()[1])+'.'+key,root_from[key].asString(),cb_context);


							}
							else{
								//Json::FastWriter writer;
								if (cb!=NULL)
									cb(string(&p.c_str()[1])+'.'+key,FastWriteJSON(root_from[key])/*writer.write(root_from[key])*/,cb_context);

							}
						}
						else{
							printf("JSON_Merge_cb undef key %s\n",key.c_str());
							if (root_to.isMember(key)==false)
								root_to[key]=root_from[key];

							continue;
						}
					}
			}

	return NO_ERROR;
}

eErrorTp JSON_DeepRemoveSpecObjs(Json::Value & obj,Json::Value & new_obj){
	if (obj.isObject()){
		for (const auto& key : obj.getMemberNames()) {
			if (obj[key].isObject()) {
				if ((key!="#")&&(key!="$"))
					JSON_DeepRemoveSpecObjs(obj[key],new_obj[key]);
			}
			else{
				new_obj[key]=obj[key];
			}
		}
	}
	else
		new_obj=obj;

	return NO_ERROR;
}

eErrorTp ConvertJsonArrayToTable(Json::Value & json,Json::Value & table,string p){
	string z;

		//printf("p %s isObject %d !isArray %d\n",p.c_str(),json.isObject(),!json.isArray());
		if (json.isObject()&&(!json.isArray())){
			//zr();
			for (const auto & key:json.getMemberNames()){
				//printf("key %s\n",key.c_str());
				if (p.size()!=0){
					z=p+'.'+key;
				}
				else
					z=key;
			//	printf("z %s isObject %d !isArray %d\n",z.c_str(),json[key].isObject(),!json[key].isArray());
				if (json[key].isObject()&&(!json[key].isArray())){
					//zr();
					ConvertJsonArrayToTable(json[key],table,z);
				}
				else{

					table[z]=json[key];
				}
			}
		}
		else{
			//zr();
			table=json;
		}

	return NO_ERROR;
}

eErrorTp ConvertJsonToTable(Json::Value & json,Json::Value & table,string p){
	string z;
	if (json.isArray()){
	//	zr();
		for (u32 n=0;n<json.size();n++){
			//if (p.size()!=0)
			string key;
			if (p.size()==0){
				key=IntToStr(n);
			}
			else
				key=p+'.'+IntToStr(n);

			ConvertJsonToTable(json[n],table[key],z);
		}
	}
	else
	{
		if (json.isObject()){
			for (const auto & key:json.getMemberNames()){
				//printf("key %s\n",key.c_str());
				if (p.size()!=0){
					z=p+'.'+key;
				}
				else
					z=key;
				//printf("z %s\n",z.c_str());
				if (json[key].isObject()){
					ConvertJsonToTable(json[key],table,z);
				}
				else{
					if (json[key].isArray()){
						//printf("key %s is array\n",key.c_str());
						ConvertJsonToTable(json[key],table,z);
					}
					else
						table[z]=json[key];
				}
			}
		}
		else{
			//zr();
			table=json;
		}
	}
	return NO_ERROR;
}






void MergeRapidObjects(rapidjson::Value &dstObject, rapidjson::Value &srcObject, rapidjson::Document::AllocatorType &allocator)
{
    for (auto srcIt = srcObject.MemberBegin(); srcIt != srcObject.MemberEnd(); ++srcIt)
    {
        auto dstIt = dstObject.FindMember(srcIt->name);
        if (dstIt != dstObject.MemberEnd())
        {
            assert(srcIt->value.GetType() == dstIt->value.GetType());
            if (srcIt->value.IsArray())
            {
                for (auto arrayIt = srcIt->value.Begin(); arrayIt != srcIt->value.End(); ++arrayIt)
                {
                    dstIt->value.PushBack(*arrayIt, allocator);
                }
            }
            else if (srcIt->value.IsObject())
            {
            	MergeRapidObjects(dstIt->value, srcIt->value, allocator);
            }
            else
            {
                dstIt->value = srcIt->value;
            }
        }
        else
        {
            dstObject.AddMember(srcIt->name, srcIt->value, allocator);
        }
    }
}

eErrorTp JSON_AddValueOnStringPath(string & outs,string & path,string & value)
{
	istringstream iss(path);
	string object;

	 u32 n=0;
	 outs="{";

	 while ( getline( iss, object, '.' ) ) {
		 outs+='"'+object+"\":{";
		 n++;
	 }
	 outs.pop_back();
	 outs+=value;
	 for (u32 z=n;z>0;z--){
		 //printf("z %d\n",z);
		 outs+='}';
	 }

	// root.SetObject();
	 //root.Parse(outs.c_str());
	 //printf("outs %s\n",outs.c_str());
	 return NO_ERROR;
}


eErrorTp JSON_InitArrayOnStringPath_rpd(rapidjson::Document & root,char * path,char * value)
{
	istringstream iss(path);
	string object;

	 u32 n=0;
	 string outs="{";

	 while ( getline( iss, object, '.' ) ) {
		 outs+='"'+object+"\":{";
		 n++;
	 }
	 outs.pop_back();
	 outs+=value;
	 for (u32 z=n;z>0;z--){
		 //printf("z %d\n",z);
		 outs+='}';
	 }

	 root.SetObject();
	 root.Parse(outs.c_str());
	 //printf("outs %s\n",outs.c_str());
	 return NO_ERROR;
}
