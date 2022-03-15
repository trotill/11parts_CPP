/*
 * node_req.cxx
 *
 *  Created on: 19 дек. 2018 г.
 *      Author: root
 */
#include "node_req.h"
#include "update/update.h"
#include "custom_project/custom_main.h"
#include "proto/json_proto.h"

NodeReqT::NodeReqT (eDebugTp debug_level) : NodeT()
		{
			SourceStr="Node";
			ObjPref="Req";
			//SrvType=srvtWWW;
			this->debug_level=debug_level;

			InitBuf(&SrvReadBuf);
			AllocBuf(&SrvReadBuf,NOREREQ_STRING_BUF);
			ClearBuf(&SrvReadBuf);

			InitBuf(&SrvSendBuf);
			AllocBuf(&SrvSendBuf,NOREREQ_STRING_BUF);
			ClearBuf(&SrvSendBuf);

			InitBuf(&SrvBufLH);
			AllocBuf(&SrvBufLH,NOREREQ_STRING_BUF);
			ClearBuf(&SrvBufLH);
			for (u32 n=0;n<MAX_GROUP;n++)
				Group[n]=NULL;
			//Sock_cln=new Socket("192.168.50.103",10099,crpTCP_Client);
		};

NodeReqT::~NodeReqT (void)
		{
			GPRINT(NORMAL_LEVEL,"%s deleted\n",SourceStr.c_str());
			//delete(Sock_srv);
			//delete(Sock_cln);
			FreeBuf(&SrvReadBuf);
			FreeBuf(&SrvSendBuf);
			FreeBuf(&SrvBufLH);
		}

eErrorTp NodeReqT::RND_Parse(void)
{

	//int i=0;
	//stringstream ss;
	//u8 * tbuf;
	//u8 * cbuf;
	//u32 ptype;
	//u32 vers;
	//string str;
	//string resp_if;
	eErrorTp err=ERROR;

	//printf("RND.RawBuf.base_len %d\n",RND.RawBuf.base_len);
	if (RawBuf.base_len==0)
		return err;

	RawBuf.buf_base[RawBuf.base_len]=0;

	//cbuf=tbuf=(char*)(RND.RawBuf.buf_base);

	//printhex(RND.RawBuf.buf_base,RND.RawBuf.base_len,20);

	GPRINT(NORMAL_LEVEL,"Get %s len %d strlen %d\n",RawBuf.buf_base,RawBuf.base_len,(int)strlen((char*)RawBuf.buf_base));
	BufHandler matchjson;
	InitBuf(&matchjson);
	AllocBuf(&matchjson,RawBuf.base_len+1);
	u32 left=0,right=0;
	u32 newjson_ptr=0;
	BufHandler json_req;
	BufHandler json_resp;

	//GPRINT(NORMAL_LEVEL,"MM1\n");
	for (u32 j=0;j<RawBuf.base_len;j++) {
		if (RawBuf.buf_base[j]=='{')
		 left++;
		if (RawBuf.buf_base[j]=='}')
			right++;
		//GPRINT(NORMAL_LEVEL,"MM2\n");
		//printf("left %d right %d\n",left,right);
		if (left==right){
			//GPRINT(NORMAL_LEVEL,"MM3\n");
			//printf("newjson_ptr %d j %d\n",newjson_ptr,j);
			//printf("(j-newjson_ptr)+1 %d\n",(j-newjson_ptr)+1);

			memcpy(matchjson.buf_base,&RawBuf.buf_base[newjson_ptr],(j-newjson_ptr)+1);
			matchjson.len=matchjson.base_len=j-newjson_ptr+1;

			//GPRINT(NORMAL_LEVEL,"MM4\n");
			matchjson.buf_base[matchjson.base_len]=0;
			newjson_ptr=j+1;
			GPRINT(NORMAL_LEVEL,"match pack %s\n",matchjson.buf_base);
			//InitBuf(&json_req);
			InitBuf(&json_resp);
					 			//BufToBufHandler(&json_req, cbuf,strlen(cbuf), strlen(cbuf));
			AllocBuf(&json_resp,2048);
			CMD(&matchjson,&json_resp);
			//GPRINT(NORMAL_LEVEL,"MM5\n");
			//RND используется для двустороннего взаимодействия с jnoda и одностороннего с web_ui, web_srv
			//результат для web_ui и web_srv, отправляется через другой сокет
			//SockSendToLHBus(RND.pSockSrc,(char*)json_resp.buf_base, json_resp.base_len,&RND.pSockSrc.DestAddr);
			FreeBuf(&json_resp);
		}
	}




	FreeBuf(&matchjson);
	ClearBuf(&RawBuf);
	return err;
}

u32 NodeReqT::FieldNameToID(u32 ptype,string & name)
{
	u32 ret=255;
	switch (ptype)
	{
		case JSON_PACK_TYPE_SET_SYSTEM:
			if (name=="changepasswd")
				ret=(u32)ChangePasswd;
			else
			if (name=="changepasswdV2")
				ret=(u32)ChangePasswdV2;
			else
			if (name=="chkpasswd")
				ret=(u32)CheckPasswd;
			else
			if (name=="settings")
				ret=(u32)Settings;
			else
			if (name=="reboot")
				ret=(u32)Reboot;
			else
			if (name=="update")
				ret=(u32)Update;
			else
			if (name=="resettofactory")
				ret=(u32)ResetToFactory;
			else
			if (name=="server")
				ret=(u32)Server;

		break;
	}

	return ret;
}

eErrorTp CorrectPageName(string savename,string & json_req){
	//settings.tap_network0.net.tap0
	int st_pos=savename.find("settings.");
	char buf[300];
	if (st_pos==0){
		string new_str=savename.substr(9);
		st_pos=new_str.find(".");
		if (st_pos>0){
			//Detect dyn device config, page name have id device,
			//Обнаружен конфиг дин устройства, т.к. содержится ID. Этот конфиг поступил с неверным полем page, т.к. page не имеет id устр.
			//Это не ошибка, такой подход может использовать Wizard. Т.е. например конфиг settings.tap_network0.net.tap0 сохранится с page tap_network0
			string page_name=new_str.substr(0,st_pos);
			JSON_ChangeFieldInData(json_req,"page",page_name.c_str());

		}
	}
	return NO_ERROR;
}


eErrorTp NodeReqT::ParseServerSOCKET_IO(Json::Value & root){

	//Json::Reader reader;
	//Json::FastWriter writer;
	//Json::Value root;
	Json::Value root_act;
	//printf("PARSE [%s]\n",json.c_str());
	//if (reader.parse(json,root)){

		string server=FastWriteJSON(root["d"]["data"]);
		//printf("server [%s]\n",server.c_str());

		root_act["sid"]=root["sid"].asString();
		root_act["action"]=root["d"]["action"].asString();
		string web_clnt_act=FastWriteJSON(root_act);

		//printf("web_clnt_act [%s]\n",web_clnt_act.c_str());
		shared_fifo->SendSharedFifoMessage(srvmSOCKET_IO,"all", server);
		shared_fifo->SendSharedFifoMessage(srvmWEB_CLIENT_ACT,"all", web_clnt_act);
		return NO_ERROR;
	//}
	//return ERROR;

}


eErrorTp NodeReqT::jnodaCMD_System_Settings_pre_def(string savename,string & json){

	 return NO_ERROR;
}
eErrorTp NodeReqT::jnodaCMD_System_Settings_post_def(string savename,string & json){

	 return NO_ERROR;
}
eErrorTp NodeReqT::CMD_System(u32 ptype,Json::Value & jsonReq,Json::Value & jsonResp)
{
	string data="";
	string type="";

	//

	//if (JSON_GetFieldInData((char*)json_req.c_str(),"type",data)==ERROR)

	if (!jsonReq["d"].isMember("type"))
	{
		//json_resp="request_err";
		jsonResp["type"]=0;
		jsonResp["data"]["msg"]="UNDEF_ERROR";
		printf("!!!CMD_System format error %s\n",StyledWriteJSON(jsonReq).c_str());
		return ERROR;
	}
	eErrorTp err=ERROR;
	string resp;
	type=jsonReq["d"]["type"].asString();
	jsonResp["type"]=type;
	jsonResp["data"]["msg"]="ok";
	switch (FieldNameToID(ptype,type))
	{
		case Server:{
			err=ParseServerSOCKET_IO(jsonReq);
			//..SendSharedFifoMessage(srvmUpdateSNMP,srvtHelper, NULL,0);
		}
		break;
		case ChangePasswdV2:{
			err=auth_ChangePasswordV2(this,Group,jsonReq,resp);
			//Ответ, подтверждения, что настройка дошла
			jsonResp["data"]["msg"]=resp;
			jsonResp["data"]["page"]="secure";
			jsonResp["type"]="settings";//не трогать, если будет другое знач. подтверждение не будет обработано
		}
		break;
		case ChangePasswd:{
				//string page=jsonReq["d"]["page"].asString();
				err=auth_ChangePassword(this,Group,jsonReq,resp);
				//Ответ, подтверждения, что настройка дошла
				jsonResp["data"]["msg"]=resp;
				jsonResp["data"]["page"]="secure";
				jsonResp["type"]="settings";//не трогать, если будет другое знач. подтверждение не будет обработано

			}
			break;
		//case ResetToFactory:{
		//	err=Customer.ResetDeviceToFactory();
		//}break;
		case Settings:
			{
				string savename;
				string page=jsonReq["d"]["page"].asString();
				//JSON_GetFieldInData((char*)json_req.c_str(),"page",page);

				printf("Settings\n");

				jsonResp["data"]["page"]=page;
				jsonResp["data"]["msg"]="ok";
				if (jsonReq["d"].isMember("__errMsg")){//Если есть это поле, значит конфиг не сохранять, он с ошибкой, но Cnoda должна это через себя пропустить
					jsonResp["data"]["msg"]=jsonReq["d"]["__errMsg"];
					return NO_ERROR;
				}
				string jsonReqStr=FastWriteJSON(jsonReq);
				switch (FieldNameToID(ptype,page))
				{
					case Reboot:{
						RebootSystem();
						err=NO_ERROR;
					}
					break;

					case Update:{
						//Первая отправка подтверждения ок, что команда дошла
						string sid;
						sid=jsonReq["sid"].asString();
						shared_fifo->SendEventFromCnodaToUI(FastWriteJSON(jsonResp),sid);
						err=UpdateImageStd(jsonReqStr,resp);
						jsonResp["data"]["msg"]=resp;
						//Вторая отправка сообщения о ходе обновления и об ошибке
					}
					break;

					default:{
						savename =Sm.GetSettingsName((char*)page.c_str());//<< SETTING_FILE_NAME << "." << page;
						CorrectPageName(savename,jsonReqStr);
						printf("savename %s data %s\n",savename.c_str(),jsonReqStr.c_str());
						if (jnodaCMD_System_Settings_pre_def(savename,jsonReqStr)==NO_ERROR){
							if (jsonReq["d"].isMember("__mapType")){
								if ((jsonReq["d"]["__mapType"]=="sec")||(jsonReq["d"]["__mapType"]=="user_ro")){
									GPRINT(NORMAL_LEVEL,"Skip setting save, det __mapType %s\n",jsonReq["d"]["__mapType"].asCString());
								}
								else
									Sm.SaveSetting(jsonReqStr,savename);
							}
							else
								Sm.SaveSetting(jsonReqStr,savename);
							//Json::Value preSaveJson;
							//if (JSON_ParseString(preSaveJson,json_req)==NO_ERROR){//очистка sid
							//	preSaveJson["sid"]="empty";
							//	string njson=FastWriteJSON(preSaveJson);
							//	printf("RePack setting %s new %s\n",savename.c_str(),njson.c_str());
								//Sm.SaveSetting(FastWriteJSON(preSaveJson),savename);
							//}


						}
						if (page=="reporters"){
							shared_fifo->SendSharedFifoMessage(srvmReportSettings,"relo",jsonReqStr);
							//SendSharedFifoMessage(srvmUpdateSNMP,srvtHelper, NULL,0);
						}

						if (Customer.jnodaCMD_System_Settings(savename,jsonReqStr)==NO_ERROR){
							jnodaCMD_System_Settings_post_def(savename,jsonReqStr);
						}

						//SendSharedFifoMessage(srvmUpdateSNMP,srvtHelper, NULL,0);
						err=NO_ERROR;
					}

				}
				GPRINT(NORMAL_LEVEL,"!!!Del my:Save/Apply settings savename %s json [%s]\n",savename.c_str(),jsonReqStr.c_str());

			}
			break;
		default:
			err=ERROR;
	}
	//
	return err;
}

eErrorTp repSendTestSMS(string & json){
	Json::Value root;
	Json::Value sett;
	JSON_ParseString(root,json);
	sett=root["d"]["repSendTestSMS"]["reporters"];
	//zr();
	//printf("sett %s\n",StyledWriteJSON(sett).c_str());
	if (sett["repSMSEnable"]=="true"){
		//zr();
		vector<string> tel_num;
		string test_string=sett["repSendTestSMS_Text"].asString();
		for (u32 n=0;n<sett["repSMS_sms_recipients"].size();n++)
			tel_num.push_back(sett["repSMS_sms_recipients"][n][0].asString());

		//zr();
		shared_fifo->SendReport_SMS(tel_num,test_string);
	}
	//zr();
	return NO_ERROR;
}

eErrorTp repSendTestMail(string & json){
	Json::Value root;
	Json::Value sett;
	vector<string> email_addr;
	JSON_ParseString(root,json);
	sett=root["d"]["repSendTestMail"];
	if (sett["repEmailEnable"].asString()=="true"){
		string test_string=sett["repSendTestMail_Text"].asString();
		for (u32 n=0;n<sett["repEmail_recipients"].size();n++)
			email_addr.push_back(sett["repEmail_recipients"][n][0].asString());

		if (sett["repEmail_recipients"].size()>0)
			shared_fifo->SendReport_eMail(email_addr,test_string,false);
	}
	return NO_ERROR;
}

eErrorTp NodeReqT::ReconfDeviceAction(Json::Value & root){
	printf("ReconfDeviceAction %s\n",StyledWriteJSON(root).c_str());
	if (root["d"][reconf_device_action].isMember("cfg_admin")){
		zr();
		Json::Value act=root["d"][reconf_device_action]["cfg_admin"];
		string file_name;//=act["cadmSetCfg"].asString();

		 if (act["cadmSetCfg"].isArray())
			 file_name=act["cadmSetCfg"][0].asString();
		 else
			 file_name=act["cadmSetCfg"].asString();

		act["sid"]=root["sid"];
		string path=string_format("%s/%s",CnT->DOWNLOAD_PATH.c_str(),file_name.c_str());

		if (SearchFile(path)==NO_ERROR){
					// printf("DO UPDATE\n");
			printf("ReconfDeviceAction got path %s\n",path.c_str());
			shared_fifo->SendSharedFifoMessage(srvmSAST_ROLLOUT_ARCHIVE_CONFIGS,"sast", act);
		}
		else
		{
			printf("ReconfDeviceAction not found path %s\n",path.c_str());
			string json_resp="file_err";
			string sid=root["sid"].asString();
			//JSON_GetField((char*)json.c_str(),"sid",sid);

			Json::Value jsonResp;
			jsonResp["data"]["msg"]=json_resp;
			shared_fifo->SendEventFromCnodaToUI(FastWriteJSON(jsonResp),sid);
		}
	}
	return NO_ERROR;
}

eErrorTp NodeReqT::CMD_Event(string json)
{
	string action="";

	string result="";
		    	printf("JSON_PACK_TYPE_SEND_EVENT %s\n",json.c_str());

	//JSON_GetFieldInData((char*)json.c_str(),"action",action);
	Json::Value root;
	if (JSON_ParseString(root,json)==NO_ERROR){
		if (root.isMember("d")&&root["d"].isMember("action")){
			action=root["d"]["action"].asString();

			 if (action=="extStore"){
				 shared_fifo->SendSharedFifoMessage(srvmEXT_STOR,"gulo", root["d"]["extStore"]);
			 }
			 if (action=="jnodaReady"){

				 //JSON_GetFieldInData((char*)json.c_str(),"jnoda",result);
				 //zr();
				// if(root["d"]["jnodaReady"].isMember("message")&&(root["d"]["jnodaReady"]["message"].asString()=="TO_CNODA")){
					// zr();
					 printf("GOT EVENT\n\nJNODA_READY\n\n");
					 shared_fifo->SendSharedFifoMessage(srvmJNODA_READY,"all", NULL,0);
					 //zr();
					 //SendSharedFifoMessage(eMsgTypeTp Msg,string Dest, u8 * Data,u32 DataLen)
					 //shared_fifo->SendSharedFifoMessage(srvmJNODA_READY,"wead", NULL,0);
					 //shared_fifo->SendSharedFifoMessage(srvmJNODA_READY,"sast", NULL,0);
					// shared_fifo->SendMcastFifoMessage(NULL,0);
					 //SendSharedFifoMessage(this,srvmJNODA_READY,srvtPARAMS, NULL,0);
					 //printf("result==ready\n");
				// }
				// zr();
			 }

			 if (action=="repSendTestMail")
				 repSendTestMail(json);

#ifdef _SECURE_ENABLE
			 if ((action=="cnodakg")&&(CnT->authorised)){
				 Json::Value cnodakgRoot;
				 if (root["d"]["cnodakg"].isMember("passwd")){
					 string cnodakgPasswd=root["d"]["cnodakg"]["passwd"].asString();
					 if (cnodakgPasswd==_SECURE_SNP){
							epsec::secure sec;
							sec.logPull();
							cnodakgRoot["cnodakgV1"]["stat"]="ready";
							cnodakgRoot["cnodakgV1"]["log"]=sec.logGet();
							CnT->cnodakgUnlock=true;
							shared_fifo->SendSystemFromCnodaToUI(FastWriteJSON(cnodakgRoot));

					 }
				 }
				 if (root["d"]["cnodakg"].isMember("uid")){
					 string uid=root["d"]["cnodakg"]["uid"].asString();
					 if (CnT->cnodakgUnlock){
						 string cmd=sf("%s/cnodakg %s %s",CnT->DOWNLOAD_PATH.c_str(),_SECURE_SNP,uid.c_str());
						 string result=BashResult(cmd);
						 epsec::secure sec;
						 sec.logPull();
						 string log=sec.logGet();
						 Json::Value rootLog=parseJSON(log);
						 Json::Value shortRootLog;
						 shortRootLog["d"]["count"]=rootLog["d"]["count"];
						 shortRootLog["d"]["genHistory"][0]=rootLog["d"]["genHistory"][rootLog["d"]["genHistory"].size()-1];

						 cnodakgRoot["cnodakgV1"]["stat"]="licKey";
						 cnodakgRoot["cnodakgV1"]["value"]=result;
						 cnodakgRoot["cnodakgV1"]["log"]=FastWriteJSON(shortRootLog);

						 printf("cmd [%s] licKey [%s]",cmd.c_str(),result.c_str());
						 shared_fifo->SendSystemFromCnodaToUI(FastWriteJSON(cnodakgRoot));
					 }
				 }
			 }
#endif
			 if (action=="activation"){
#ifdef _SECURE_ENABLE
				 if (root["d"]["activation"].isMember("key")){//установка ключа устройства
					 sleep(3);
					 string actKey=root["d"]["activation"]["key"].asString();
					 epsec::secure sec;
					 sec.setDeviceUID(CnT->deviceUID);
					 zr();
					 if (CnT->authorised==false){
						 zr();
						if (sec.checkKey(actKey)==NO_ERROR){
							zr();
							sec.signAuth(actKey);
							if (sec.checkAuthSign(actKey)==NO_ERROR){
								zr();
								string encLicKey=sec.encryptLicenseData(actKey);
								if (CnT->sm.SaveSetting(encLicKey,"license")==NO_ERROR){
								//if (sec.setLicKey(actKey)==NO_ERROR){
									CnT->authorised=true;
									shared_fifo->activationMessageActivated();
								}
							}
						}
					 }
				 }
#endif
				 if (root["d"]["activation"].isMember("priv")){//установка приватных данных устройства
					 string priv=root["d"]["activation"]["priv"].asString();
					Json::Value actPriv;
					actPriv["activation"]["stat"]="privSaved";
					//forceLogoutRoot["forceLogout"]["group"]="";
					CnT->sm.SaveSetting(priv,"private");
					shared_fifo->SendSystemFromCnodaToUI(FastWriteJSON(actPriv));
					CnT->reqPrivate=false;
				 }
			 }

			 if (action=="repSendTestSMS"){
				// printf("");
				 printf("repSendTestSMS %s\n",action.c_str());
				 repSendTestSMS(json);
			 }

			 if (action==smsd_GotSMS_action){
				 shared_fifo->SendSharedFifoMessage(srvmJSON_ACTION_FORMAT,"relo", json);
			 }

			 if (action==reconf_device_action){
				 ReconfDeviceAction(root);
			 }
			 if (action=="webevent"){//){
				 //zr();
				 if (root["d"]["webevent"].isMember("req")){
					 if (root["d"]["webevent"]["req"].asString()==serialport_gsm_GotSMS_action){
						 shared_fifo->SendSharedFifoMessage(srvmJSON_ACTION_FORMAT,"relo", json);
					 }
					 if (root["d"]["webevent"]["req"].asString()==serialport_gsm_GotCall_action){
						 shared_fifo->SendSharedFifoMessage(srvmJSON_ACTION_FORMAT,"relo", json);
					 }
				 }
				 //zr();
			 }
			 //if (action=="set_date_time"){

				 //shared_fifo->SendSharedFifoMessage(srvmSET_DATE_TIME,"all", NULL,0);
			 //}
			 //zr();
			Customer.jnodaCMD_Event(action,json);
			//zr();
		}
	}
	else{
		printf("Error parse %s\n!!!",json.c_str());
	}
	return NO_ERROR;
}

eErrorTp NodeReqT::CMD(BufHandler * json_req,BufHandler * json_resp)
{
	u32 ptype;
	u32 vers;
	ClearBuf(json_resp);
	Json::Value jsonResponse;

	JSON_GetPackType((char*)json_req->buf_base,ptype,vers);
	jsonResponse["ptype"]=ptype;
	switch(ptype)
		{
	    case JSON_PACK_TYPE_SEND_EVENT:
 			CMD_Event(string((char*)json_req->buf_base));
	    break;
		case JSON_PACK_TYPE_INIT_ME:
		{
			DeInit("{}");
			Configure();
			JnodaRun=true;
		}
		break;
 		case JSON_PACK_TYPE_SET_SYSTEM:
 		{
 			//Json::Value jsonResponse;
 			//string request=;
 			Json::Value jsonReq;
 			if (JSON_ParseString(jsonReq,(char*)json_req->buf_base)!=ERROR){

 				if (jsonReq.isMember("d"))

					{
 						bool block=false;
						if (jsonReq["d"].isMember("action"))//action не бывает у set_system
						{
							string actStr=jsonReq["d"]["action"].asString();
							if (jsonReq["d"].isMember(actStr)){
								block=true;
								GPRINT(NORMAL_LEVEL,"Skip %s, incorrect pack\n",json_req->buf_base);
							}
						}
						if (block==false)
						{
							GPRINT(HARD_LEVEL,"CMD_System %s\n",json_req->buf_base);
							if (CMD_System(ptype,jsonReq,jsonResponse)==NO_ERROR){
								if (!jsonResponse.isMember("data"))
									jsonResponse["data"]["msg"]="ok";
							}

							//strcpy((char*)json_resp->buf_base,(char*)response.c_str());
							//json_resp->base_len=strlen((char*)json_resp->buf_base);
							string sid=jsonReq["sid"].asString();
							//JSON_GetField((char*)json_req->buf_base,"sid",sid);
							shared_fifo->SendEventFromCnodaToUI(FastWriteJSON(jsonResponse),sid);
						}
					}
					else
						return ERROR;


 			}

 		}
		break;

	 	case JSON_PACK_TYPE_GET_SYSTEM:
	 	break;
	 	case JSON_PACK_TYPE_REQ_SYS_INFO:
	 		ReqNODE(ptype, vers,json_req,json_resp);
	 		jsonResponse["data"]["msg"]=json_resp;
	 		shared_fifo->SendEventFromCnodaToUI(FastWriteJSON(jsonResponse));
		break;
	 //	case JSON_PACK_TYPE_REQ_EVENT:
	 //		ReqEvent(vers,json_req,json_resp);
	 //	break;
 		case JSON_PACK_TYPE_RESP_SYS_INFO:
 			break;
 		case JSON_PACK_TYPE_TEST_DATA_EXIST:
 			break;
 		case JSON_PACK_TYPE_RESP_DATA_NOT_EXIST:
 			break;

 		//case JSON_PACK_TYPE_SP_RESTART:
 		case JSON_PACK_TYPE_WD_RESTART:
 		{
 			sInterThrMsgHeader Mheader;
 			Mheader.MsgType=srvmSendToLH;
 			//printf("base len %d free_size %d, json_req->buf_base %s\n",json_req->base_len,json_req->free_size,json_req->buf_base);
 			//printf("json_req->buf %s json_req->len %d\n",json_req->buf,json_req->len);

 			shared_fifo->SendUcastFifoMessage(json_req,Mheader, "wead");
 			//strcpy((char*)json_resp->buf_base,(char*)"wait");
 			//json_resp->base_len=strlen((char*)json_resp->buf_base);
 			//printf("@@@@@@@@@@@@@@@ WAIT @@@@@@@@@@\n");

 			jsonResponse["data"]["msg"]="wait";

 			shared_fifo->SendEventFromCnodaToUI(FastWriteJSON(jsonResponse));
 			//sleep(5);
 		}
 		break;
 		default:
 			printf("Undef json pack [%d]\n",ptype);
		}
	return NO_ERROR;

}

eErrorTp NodeReqT::ReqNODE(u32 pack, u32 vers,BufHandler * json_req,BufHandler * json_resp)
{
	ClearBuf(json_resp);

	 		string req;
	 		string iface;
	 		string reqp;
	 		string wait_t;
	 		string data;
	 		char * resp;

	 		if ((JSON_GetFieldInData((char*)json_req->buf_base,(char*)"req",req)==NO_ERROR)
	 				&&(JSON_GetFieldInData((char*)json_req->buf_base,(char*)"iface",iface)==NO_ERROR)
	 				&&(JSON_GetFieldInData((char*)json_req->buf_base,(char*)"wait_t",wait_t)==NO_ERROR))
	 		{

	 			reqp='\"'+req+'\"';

	 			ClearBuf(json_resp);

	 			if (Cache.RequestFilter(iface,(char*)req.c_str(),stoul(wait_t))==NO_ERROR)
	 				ReqNetInfo(iface,reqp);
	 			if (Cache.FromCashe(iface,req,data)==NO_ERROR)
	 			{
	 				printf("mach resp [%s]\n",data.c_str());

	 				CopyToBuf(json_resp,(u8*)data.c_str(),data.size()+1);
	 			}
	 			else
	 			{
	 				snprintf((char*)json_resp->buf_base,json_resp->free_size,JSON_EMPTY_RESP);
	 				AddCStringToBuf(json_resp);
	 				AddZeroToBuf(json_resp);
	 			}
	 		}
	 		else
	 		{
	 			GPRINT(NORMAL_LEVEL,"Error JSON parse [%s]\n",json_req->buf_base);
				snprintf((char*)json_resp->buf_base,json_resp->free_size,JSON_EMPTY_RESP);
		 				AddCStringToBuf(json_resp);
		 				AddZeroToBuf(json_resp);
		 				printf("no recv data [%s] len %lu\n",(char*)json_resp->buf_base,strlen((char*)json_resp->buf_base));
	 		}

	 		return NO_ERROR;

}


CasherT::CasherT():max_cashe(MAX_CACHE_ELEMENTS)
	{
		Cache = new sRecCache*[max_cashe];
		for (u32 i=0;i<max_cashe;i++)
		{

			Cache[i]=NULL;
		}
	}

CasherT::CasherT(u32 max):max_cashe(max)
	{
		Cache = new sRecCache*[max_cashe];
		for (u32 i=0;i<max_cashe;i++)
		{

			Cache[i]=NULL;
		}
	}

CasherT::~CasherT()
	{

		for (u32 i=0;i<max_cashe;i++)
				{
					if (Cache[i]!=NULL)
						delete Cache[i];
				}

		delete Cache;
	}

eErrorTp CasherT::RequestFilter(string & iface,char * req,u32 FltTime)
	{
		eErrorTp err=ERROR;
		u32 i;
		string reqs=req;
		TIME_T src_time;
		u32 Flt=FltTime/1000;
		for (i=0;i<max_cashe;i++)
		{
			if (Cache[i]!=NULL)
			{
				//printf("(Cashe[i]!=NULL) %s/%s %s/%s\n",Cashe[i]->iface.c_str(),iface.c_str(),Cashe[i]->req.c_str(),reqs.c_str());
			if ((Cache[i]->iface==iface)&(Cache[i]->req==reqs))
			{
				src_time=TIME((u32*)NULL);
				if ((src_time-Cache[i]->SendTime)>=Flt)
				{
					//printf("[%d]/[%d] [%d] req %s/%s\n",src_time,Cashe[i]->SendTime,src_time-Cashe[i]->SendTime,Cashe[i]->iface.c_str(),reqs.c_str());
					Cache[i]->SendTime=src_time;
					err=NO_ERROR;
				}
				else
				{
					printf("skip req %s/%s\n",Cache[i]->iface.c_str(),reqs.c_str());
				}
				break;
			}
			}

		}
		if (i==max_cashe)
			err=NO_ERROR;

		return err;
	}

eErrorTp CasherT::ToCashe(string & iface,char * req,char * data)
	{
		u32 i;
		string reqs=req;
		for (i=0;i<max_cashe;i++)
		{
			if (Cache[i]!=NULL)
			{
				if ((Cache[i]->iface==iface)&(Cache[i]->req==reqs))
				{
					Cache[i]->data=data;
					//printf("Edit cashe %s/%s\n",Cashe[i]->iface.c_str(),Cashe[i]->req.c_str());
					break;
				}
			}
			else
			{
				Cache[i]=new sRecCache;
				Cache[i]->data=data;
				Cache[i]->iface=iface;
				Cache[i]->req=req;
				Cache[i]->FiltTime=0;
				Cache[i]->SendTime=0;
				//printf("Add cashe %d %s/%s\n",i,Cashe[i]->iface.c_str(),Cashe[i]->req.c_str());
				break;
			}
		}
		if (i==max_cashe)
		{
			printf("Cache overload, total elements %d\n",MAX_CACHE_ELEMENTS);
			return ERROR;
		}
		return NO_ERROR;
	}

eErrorTp CasherT::FromCashe(string & iface,string & req,string & data)
		{
		data="";
		eErrorTp err=ERROR;
		for (u32 i=0;i<max_cashe;i++)
				{
					if (Cache[i]!=NULL)
					{
						if ((Cache[i]->iface==iface)&(Cache[i]->req==req))
						{
							data=Cache[i]->data+'\n';
							err=NO_ERROR;
							break;
						}
					}
					else
					  break;
				}
		  return err;
		}










