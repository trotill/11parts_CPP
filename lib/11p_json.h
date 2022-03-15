/*
 * 11p_json.h
 *
 *  Created on: 22 февр. 2021 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_LIB_11P_JSON_H_
#define SRVIOT_SRC_ENGINE_LIB_11P_JSON_H_


#include <engine/types.h>
#include <engine/basic.h>

#include "engine/memadmin.h"

#define JSONCPP_DEFAULT_PRECISION 10


eErrorTp JSON_ParseString(Json::Value & json,char * data);
eErrorTp JSON_ParseString(Json::Value & json,string & data);
Json::Value parseJSON(string data);
string FastWriteJSON(Json::Value & val);
string FastWriteJSON(Json::Value & val,u32 precision);
string JSON_stringify(Json::Value & data);
eErrorTp JSON_round(Json::Value & data,u8 precision);
string StyledWriteJSON(Json::Value & val,u32 precision);
string StyledWriteJSON(Json::Value & val);
string FastWriteJSON(rapidjson::Document * val);
string StyledWriteJSON(rapidjson::Document * val);

void printj(Json::Value & v);
void printJSON(string comment,Json::Value & val);

template <typename T>
Json::Value JSON_parse(T data){
	Json::Value json;
	JSON_ParseString(json,data);
	return json;
}

#endif /* SRVIOT_SRC_ENGINE_LIB_11P_JSON_H_ */
