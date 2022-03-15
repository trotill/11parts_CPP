/*
 * json_proto.h
 *
 *  Created on: 27 февр. 2017 г.
 *      Author: root
 */

#ifndef JSON_PROTO_H_
#define JSON_PROTO_H_

#include <engine/lib/11p_json.h>

/*
[{"t":[type,vers]},{data}]

type тип пакета:
       1 - set_system настройки системы (роутер+IoT)
       2 - set_sensors настройка датчиков/ИУ
       3 - get_sensors ответ датчиков
       4 - get_system, запрос настроек
       5 - request_system_info запрос информации (роутер+IoT)
       6 - response_system_info ответ на запрос информации
       7 - test_data_exist тест на наличие данных, если данных нет отправляется response_data_not_exist
       8 - response_data_not_exist отсутствие данных для сервера IoT
       9 - apply sennings router
vers - версия протокола
data
{ключ:значение,ключ:значение}
*/

#define JSON_PACK_TYPE_SET_SYSTEM 1
#define JSON_PACK_TYPE_TO_JNODA 2
#define JSON_PACK_TYPE_FROM_CNODA 3
#define JSON_PACK_TYPE_GET_SYSTEM  4
#define JSON_PACK_TYPE_REQ_SYS_INFO 5

#define JSON_PACK_TYPE_RESP_SYS_INFO  6
#define JSON_PACK_TYPE_TEST_DATA_EXIST  7
#define JSON_PACK_TYPE_RESP_DATA_NOT_EXIST  8
#define JSON_PACK_TYPE_REQ_EVENT  9

#define JSON_PACK_TYPE_INIT_ME 10
#define JSON_PACK_TYPE_WD_RESTART 11
#define JSON_PACK_TYPE_SEND_EVENT 12
//#define JSON_PACK_TYPE_AUTH 13
//#define JSON_PACK_TYPE_AUTH_CLEAR 14

//Сообщения из Cnoda в WEB клиент
#define JSON_PACK_TYPE_TO_UI 15

//Сообщения из Cnoda в WEB сервер
#define JSON_PACK_TYPE_TO_WEB_SRV 16
#define JSON_PACK_TYPE_TO_CNODA 17

#define TO_WEB_SRV_TYPE_AUTH "auth"
#define TO_WEB_SRV_TYPE_AUTH_CLEAR "authclear"

#define TO_WEB_SRV_TYPE_CNODA_READY "cnodaready"

#define JSON_PACK_HEADER_IDX 0
#define JSON_PACK_DATA_IDX 1

#define JSON_PACK_HEADER_PACK_TYPE 0//offset
#define JSON_PACK_HEADER_VERSION 1//offset

#define JSON_PACK_VERSION 1

#define JSON_HEADER_SYMB "t"
#define JSON_DATA_SYMB "d"
#define JSON_INFO_SYMB "i"
#define JSON_IFACE_NAME "if"
#define JSON_FIRMWARE_FIELD_NAME "fw"

#define JSON_EMPTY_RESP "empty\n"
#define JSON_S_LAN_MODE "lMode"
#define JSON_S_LAN_MAC "lMac"
#define JSON_S_LAN_DNS1 "lDns1"
#define JSON_S_LAN_DNS2 "lDns2"
#define JSON_S_LAN_DNSAUTO "lDnsa"
#define JSON_S_LAN_IP "lIp"
#define JSON_S_LAN_MASK "lMs"
#define JSON_S_LAN_GW "lGw"

#define JSON_S_WAN "wNat"

#define JSON_TXT_CONST_DHCP "DHCP"
#define JSON_TXT_CONST_STATICIP "Static IP"
#define JSON_TXT_CONST_LAN0 "LAN0"
#define JSON_TXT_CONST_LAN1 "LAN1"
#define JSON_TXT_CONST_WIFI "WIFI"
#define JSON_TXT_CONST_GSM  "GSM"

Json::Value JSON_CreatePack(Json::Value d_obj,string sid);
eErrorTp JSON_GetPackType(char * json,u32 & ptype,u32 & vers);
eErrorTp JSON_GetFieldInData(char * json,char * fieldname,string & data);
eErrorTp JSON_GetFieldInDataFromFile(string filename,string fieldname,string & data);
eErrorTp JSON_GetFieldInData(Json::Value & json,char * fieldname,string & data);
eErrorTp JSON_GetField(char * json,char * fieldname,string & data);
eErrorTp JSON_ChangeFieldInData(string & json,char * fieldname,string data);
eErrorTp JSON_GetFieldInDataInfo(char * json,char * fieldname,string & info);
eErrorTp JSON_TestDataInfoInResp(char * json,char * fdatafield,char * fdata,char * finfo);
eErrorTp JSON_GetFieldInDataMulti(char * json,char * fieldname,u8 num,string & data);
eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,bool & var);
eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,char * var);
eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,Json::Value & var);
eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,string & var);
eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,u16 & var);
eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,u32 & var);
eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,u64 & var);
eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,int & var);
eErrorTp JSON_ReadConfigField(Json::Value & root,char * fieldname,u8 & var);
//Get new object on path type "a.b.c"
eErrorTp JSON_GetObjectOnStringPath(Json::Value & src_root,Json::Value & new_root,char * path);
eErrorTp JSON_Merge(Json::Value & root_to,Json::Value & root_from);

eErrorTp JSON_MergeWOCreate_cb(Json::Value & root_to,Json::Value & root_from,int (*cb)(string merge_vname,string merge_value,void * cb_context),void * cb_context,string path);
eErrorTp JSON_MergeWOCreate_cb(Json::Value & root_to,Json::Value & root_from,int (*cb)(string merge_vname,Json::Value & merge_value,void * cb_context),void * cb_context,string path);
//Глубокое удаление спец. обьектов # и $, во всем json.
eErrorTp JSON_DeepRemoveSpecObjs(Json::Value & obj,Json::Value & new_obj);
//eErrorTp JSON_SetValueOnStringPath(Json::Value & root,char * path,string value);

template <typename T>
eErrorTp JSON_SetValueOnStringPath(Json::Value & root,char * path,T value)
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
	 	 case 1:root[f[0]]=value; break;
	 	 case 2:root[f[0]][f[1]]=value;break;
	 	 case 3:root[f[0]][f[1]][f[2]]=value;break;
	 	 case 4:root[f[0]][f[1]][f[2]][f[3]]=value;break;
	 	 case 5:root[f[0]][f[1]][f[2]][f[3]][f[4]]=value;break;
	 	 case 6:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]]=value;break;
	 	 case 7:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]]=value;break;
	 	 case 8:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]]=value;break;
	 	 case 9:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]]=value;break;
	 	 case 10:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]]=value;break;
	 	 case 11:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]]=value;break;
	 	 case 12:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]]=value;break;
	 	 case 13:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]]=value;break;
	 	 case 14:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]]=value;break;
	 	 case 15:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]]=value;break;
	 	 case 16:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]]=value;break;
	 	 case 17:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]]=value;break;
	 	 case 18:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]][f[17]]=value;break;
	 	 case 19:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]][f[17]][f[18]]=value;break;
	 	 case 20:root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]][f[17]][f[18]][f[19]]=value;break;
	 	 default:
	 		root[f[0]][f[1]][f[2]][f[3]][f[4]][f[5]][f[6]][f[7]][f[8]][f[9]][f[10]][f[11]][f[12]][f[13]][f[14]][f[15]][f[16]][f[17]][f[18]][f[19]]=value;
	 		printf("ERROR JSON_GetObjectOnStringPath !!!\n");
	 		return ERROR;
	 }
	// new_root=new_root.get(path);
	 //new_root=src_root["x"]["ws"]["store3"]["mod"]["mod_cooler_public_t"]["fans_pause_time"];
	return NO_ERROR;
}

eErrorTp JSON_RemoveValueOnStringPath(Json::Value & root,char * path);


eErrorTp ConvertJsonToTable(Json::Value & json,Json::Value & table,string p);
eErrorTp ConvertJsonArrayToTable(Json::Value & json,Json::Value & table,string p);
//string FastWriteJSON(Json::Value & val);
//string FastWriteJSON(Json::Value & val,u32 precision);
//string JSON_stringify(Json::Value & data);
//eErrorTp JSON_round(Json::Value & data,u8 precision);
//string FastWriteJSON(rapidjson::Document * val);
//string StyledWriteJSON(Json::Value & val);
//string StyledWriteJSON(Json::Value & val,u32 precision);
//void printJSON(string comment,Json::Value & val);
//string StyledWriteJSON(rapidjson::Document * val);
//void printj(Json::Value & v);

//Json::Value parseJSON(string data);
eErrorTp JSON_ParseFile(Json::Value & json,string filename);
//eErrorTp JSON_ParseString(Json::Value & json,string & data);
//eErrorTp JSON_ParseString(Json::Value & json,char * data);

void MergeRapidObjects(rapidjson::Value &dstObject, rapidjson::Value &srcObject, rapidjson::Document::AllocatorType &allocator);
eErrorTp JSON_InitArrayOnStringPath_rpd(rapidjson::Document & root,char * path,char * value);
eErrorTp JSON_AddValueOnStringPath(string & outs,string & path,string & value);

typedef enum {ChangePasswd,ChangePasswdV2,CheckPasswd,Settings,Reboot,Update,Display,ResetToFactory,Server}esCMD;

#endif /* JSON_PROTO_H_ */
