/*
 * 11p_json.cxx
 *
 *  Created on: 22 февр. 2021 г.
 *      Author: root
 */

#include "11p_json.h"


eErrorTp JSON_ParseString(Json::Value & json,char * data)
{
	Json::Reader rd;

	if (rd.parse(data,json,false)){
		return NO_ERROR;
	}
	else
		return ERROR;

	return ERROR;
}

eErrorTp JSON_ParseString(Json::Value & json,string & data)
{
	Json::Reader rd;

	if (rd.parse(data.c_str(),json,false)){
		return NO_ERROR;
	}
	else
		return ERROR;

	return ERROR;
}



Json::Value parseJSON(string data){
	Json::Value result;
	JSON_ParseString(result,data);
	return result;
}



string FastWriteJSON(Json::Value & val){
	 Json::StreamWriterBuilder builder;
#ifdef JSONCPP_PRECISION
	 builder["precision"] = JSONCPP_PRECISION;
#else
	 builder["precision"] = JSONCPP_DEFAULT_PRECISION;
#endif
	 builder["commentStyle"] = "None";
	 builder["indentation"] = "";
	 return Json::writeString(builder, val);
}

string FastWriteJSON(Json::Value & val,u32 precision){
	 Json::StreamWriterBuilder builder;
	 builder["precision"] = precision;
	 builder["commentStyle"] = "None";
	 builder["indentation"] = "";
	 return Json::writeString(builder, val);
}


string JSON_stringify(Json::Value & data){
	return FastWriteJSON(data);
}

eErrorTp JSON_round(Json::Value & data,u8 precision){
	if (precision==0)
		return ERROR;

	if (data.isDouble()){
		u32 k=0;
		k=pow(10,precision);
		double val=round((double)data.asDouble()*k)/k;
		data=val;
		return NO_ERROR;
	}
	return ERROR;
}
string StyledWriteJSON(Json::Value & val,u32 precision){
	 Json::StreamWriterBuilder builder;
	 builder["precision"] = precision;
	 return Json::writeString(builder, val);
}

string StyledWriteJSON(Json::Value & val){
	 Json::StreamWriterBuilder builder;
#ifdef JSONCPP_PRECISION
	 builder["precision"] = JSONCPP_PRECISION;
#else
	 builder["precision"] = JSONCPP_DEFAULT_PRECISION;
#endif
	 return Json::writeString(builder, val);
}

void printJSON(string comment,Json::Value & val){
	printf("%s %s\n",comment.c_str(),StyledWriteJSON(val).c_str());
}
void printj(Json::Value & v){
	printJSON("",v);
}

string FastWriteJSON(rapidjson::Document * val){
	 //Json::StreamWriterBuilder builder;
	// builder["precision"] = 5;
	// return Json::writeString(builder, val);
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	val->Accept(writer);
	return buffer.GetString();
}

string StyledWriteJSON(rapidjson::Document * val){
	 //Json::StreamWriterBuilder builder;
	// builder["precision"] = 5;
	// return Json::writeString(builder, val);
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	val->Accept(writer);
	return buffer.GetString();
}
