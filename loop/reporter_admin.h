/*
 * reporter_admin.h
 *
 *  Created on: 14 янв. 2020 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_LOOP_REPORTER_ADMIN_H_
#define SRVIOT_SRC_ENGINE_LOOP_REPORTER_ADMIN_H_

#include "engine/thread.h"
#include "custom_project/custom_project.h"

#define ACTION_GET_SMS "get_sms"
#define ACTION_GET_VOICE_CALL "get_gsm_call"

class Reporter_loop  : public ThreadT {
	public:
	Reporter_loop(eDebugTp debug_level);
	virtual ~Reporter_loop (void);
	eErrorTp Report (char * data);
	void Test(void);
	private:

	buffer FifoPreBuf;
		virtual eErrorTp Loop(void* thisPtr);
		vector<string> senders;
		eErrorTp Configure(char * settstr);
		eErrorTp ParseSMS_from_serialport_gsm(Json::Value & sms){
			Json::Value result;

			if (!sms.isObject()||(!sms.isMember("message"))||(!sms.isMember("dateTimeSent")))
				return ERROR;

			result["t"][0]=JSON_PACK_TYPE_SEND_EVENT;
			result["t"][1]=JSON_PACK_VERSION;
			result["d"]["action"]=ACTION_GET_SMS;
			result["d"][ACTION_GET_SMS]["text"]=sms["message"].asString();
			string sms_sender="+"+sms["sender"].asString();
			result["d"][ACTION_GET_SMS]["telnum"]=sms_sender;
			//printf("senders.size()\n",senders.size());
			if (senders.size()!=0){
				if (std::find(senders.begin(), senders.end(), sms_sender) == senders.end())
				{
				  return ERROR;
				}
			}

			result["d"][ACTION_GET_SMS]["imsi"]="";//strprs[1];
			result["d"][ACTION_GET_SMS]["send_date"]=sms["dateTimeSent"].asString();
			result["d"][ACTION_GET_SMS]["received_date"]="";

			SendSharedFifoMessage(srvmJSON_ACTION_FORMAT,"all", result);

			//printf("data %s\n From %s\n IMSI %s\n Sent %s\n Received %s\n",data.c_str(),telnum.c_str(),IMSI.c_str(),Sent.c_str(),Received.c_str());
			return NO_ERROR;
		}
		eErrorTp ParseVoiceCall_from_serialport_gsm(Json::Value & call){
					Json::Value result;

					if ((!call.isObject())||(!call.isMember("data"))||(!call["data"].isMember("number")))
						return ERROR;

					result["t"][0]=JSON_PACK_TYPE_SEND_EVENT;
					result["t"][1]=JSON_PACK_VERSION;
					result["d"]["action"]=ACTION_GET_VOICE_CALL;
					string call_tel=call["data"]["number"].asString();
					result["d"][ACTION_GET_VOICE_CALL]["tel"]=call_tel;
					if (senders.size()!=0){
						if (std::find(senders.begin(), senders.end(), call_tel) == senders.end())
						{
						  return ERROR;
						}
					}

					SendSharedFifoMessage(srvmJSON_ACTION_FORMAT,"all", result);

					//printf("data %s\n From %s\n IMSI %s\n Sent %s\n Received %s\n",data.c_str(),telnum.c_str(),IMSI.c_str(),Sent.c_str(),Received.c_str());
					return NO_ERROR;
		}

		eErrorTp ParseSMS_from_smsd(char * sms){
			Json::Value result;
			std::vector<std::string> sWords = split(sms, '\n');
			if (sWords.size()!=0)
			{
				result["t"][0]=JSON_PACK_TYPE_SEND_EVENT;
				result["t"][1]=JSON_PACK_VERSION;
				result["d"]["action"]=ACTION_GET_SMS;
				result["d"][ACTION_GET_SMS]["text"]=sWords[sWords.size()-1];


				//string telnum="";
				//string IMSI="";
				//string Sent="";
				//string Received="";

				for (u8 n=0;n<sWords.size()-1;n++){

					std::vector<std::string> strprs = split(sWords[n], ':');
					if (strprs.size()!=0){
						if (strprs[0]=="From"){
							result["d"][ACTION_GET_SMS]["telnum"]=strprs[1];
						}
						if (strprs[0]=="IMSI"){
							result["d"][ACTION_GET_SMS]["imsi"]=strprs[1];
						}
						if (strprs[0]=="Sent"){
							result["d"][ACTION_GET_SMS]["send_date"]=strprs[1];
						}
						if (strprs[0]=="Received"){
							result["d"][ACTION_GET_SMS]["received_date"]=strprs[1];
						}
					}
					//printf("strprs%d %s\n",n,strprs[n].c_str());
				}
				//if (result["d"].isMember("sms")){
					//string strres=FastWriteJSON(result);

					SendSharedFifoMessage(srvmJSON_ACTION_FORMAT,"all", result);
			}
			else
				GPRINT(NORMAL_LEVEL,"Error parse SMS with text \n%s\n",sms);
			//printf("data %s\n From %s\n IMSI %s\n Sent %s\n Received %s\n",data.c_str(),telnum.c_str(),IMSI.c_str(),Sent.c_str(),Received.c_str());
			return NO_ERROR;
		}
		eErrorTp Action(char * json){
			Json::Value action;
			GPRINT(MEDIUM_LEVEL,"Get Action %s\n",json);
			if (JSON_ParseString(action,(char*)json)==NO_ERROR){
				//zr();
				if (action["d"].isMember(smsd_GotSMS_action)&&action["d"][smsd_GotSMS_action].isMember("smsinfile")){
					string smsfile=action["d"]["smsd"]["smsinfile"].asString();
					string result;
					GPRINT(HARD_LEVEL,"Try read SMS %s\n",smsfile.c_str());
					if (ReadStringFile((char*)smsfile.c_str(),result)==NO_ERROR){

						GPRINT(HARD_LEVEL,"read SMS %s, ok\n%s\n",smsfile.c_str(),result.c_str());
						ParseSMS_from_smsd((char*)result.c_str());
					}
					else
						GPRINT(HARD_LEVEL,"read SMS %s, fault\n",smsfile.c_str());
				}
				//zr();
				if (action["d"].isMember("webevent")
						&&action["d"]["webevent"].isMember("req")
						&&action["d"]["webevent"].isMember("result")
						&&action["d"]["webevent"]["req"].asString()==serialport_gsm_GotSMS_action){
					//zr();


					Json::Value sms=action["d"]["webevent"]["result"];
					GPRINT(HARD_LEVEL,"read SMS ok\n%s\n",StyledWriteJSON(sms).c_str());
					ParseSMS_from_serialport_gsm(sms);

				}
				if (action["d"].isMember("webevent")
						&&action["d"]["webevent"].isMember("req")
						&&action["d"]["webevent"].isMember("result")
						&&action["d"]["webevent"]["req"].asString()==serialport_gsm_GotCall_action){
					Json::Value incall=action["d"]["webevent"]["result"];

					GPRINT(HARD_LEVEL,"abonent %s is call\n",incall["data"]["number"].asCString());
					ParseVoiceCall_from_serialport_gsm(incall);
				}
			}
			return NO_ERROR;
		}
		u32 loopdelay=100;//mS
		bool nodemailer_enable=false;
		string lang="en";
		bool smsd_enable=false;
		bool serialport_gsm_enable=false;
		bool report_enable=false;
		bool email_enable=false;
		bool sms_enable=false;
		string smsd_GotSMS_action="smsd";
		string serialport_gsm_GotSMS_action="spgSMS";
		string serialport_gsm_GotCall_action="spgIncall";
		void debugTests(){
			string badSms="{\"t\":[12,1],\"d\":{\"action\":\"webevent\",\"client\":\"\",\"webevent\":{\"req\":\"spgSMS\",\"result\":\"C\"}}}";
			GPRINT(NORMAL_LEVEL,"badSms test\n");
			Action(badSms.c_str());
			sleep(1);
		}
};



#endif /* SRVIOT_SRC_ENGINE_LOOP_REPORTER_ADMIN_H_ */
