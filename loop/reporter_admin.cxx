/*
 * reporter_admin.cxx
 *
 *  Created on: 14 янв. 2020 г.
 *      Author: root
 */
#include "reporter_admin.h"


void Reporter_loop::Test(void){
	printf("Send mail with files\n");
	vector<string> sendfiles;
	sendfiles.push_back("/www/pages/necron/Projects/flytty/styles/logo_11p_2.gif");
	sendfiles.push_back("/www/pages/necron/Projects/flytty/styles/onoff_orig.png");
	vector<string> email_addr;
	email_addr.push_back("monkeyhouse@mail.ru");
	SendReport_eMail(email_addr,"Mail test","Go Go",sendfiles,false);
	printf("Send text mail\n");
	SendReport_eMail(email_addr,"Mail test",false);
	printf("Send html mail\n");
	SendReport_eMail(email_addr,"<div style='font-size: 20pt;border-style: solid;border-radius: 10px;text-align: center;padding: 10px;'>Mail test</div>",true);

	vector<string> tel_num;
	tel_num.push_back("+79997087733");
	SendReport_SMS(tel_num,"SMS test");
}

Reporter_loop::Reporter_loop(eDebugTp debug_level):ThreadT("Reporter","relo",debug_level){
		GPRINT(HARD_LEVEL,"Create Reporter\n");
		JSON_ReadConfigField(CnT->json_cfg,"relo_loopdelay",loopdelay);
		SettingsAdm sa;
		string settstr;
		if (sa.LoadSetting(settstr,"settings.reporters")==NO_ERROR){
			//zr();
			Configure((char*)settstr.c_str());
		}

	}

Reporter_loop::~Reporter_loop (void){
		GPRINT(HARD_LEVEL,"Destroy Factory_reset\n");
	}

eErrorTp Reporter_loop::Report (char * data){
	Json::Value root;
	Json::Reader rd;
	GPRINT(MEDIUM_LEVEL,"Got Report %d byte\n",strlen(data));
	GPRINT(HARD_LEVEL,"Report data %s\n",data);
	if (rd.parse(data,root)){
		if (root["type"]=="email"){
			zr();
			if (nodemailer_enable){
				Json::Value mail;
				zr();
				if (root.isMember("recipients")){
					if (root["recipients"].isArray()){
						mail["nodemailer"]["recipients"]=root["recipients"];
					}
					else{
						mail["nodemailer"]["recipients"][0]=root["recipients"];
					}
				}

				if (root.isMember("file")){
					u8 n=0;
					for(const auto& key: root["file"]) {
						mail["nodemailer"]["attachments"][n]=root["file"][n];
						n++;
					}
				}

				if (root.isMember("subject")){
					mail["nodemailer"]["subject"]=root["subject"];
				}
				if (root.isMember("html")){
					mail["nodemailer"]["html"]=root["html"];
				}
				if (root.isMember("text")){
					mail["nodemailer"]["text"]=root["text"];
				}

				SendEventToJNODA(FastWriteJSON(mail));
			}
		}
		if (root["type"]=="sms"){
			if (smsd_enable){
				Json::Value smsd;
				//printf("tojnoda %s\n",data);
				if (root.isMember("recipients")){
					if (root["recipients"].isArray()){
						smsd["smsd"]["recipients"]=root["recipients"];
					}
					else{
						smsd["smsd"]["recipients"][0]=root["recipients"];
					}
				}

				if (root.isMember("text")){
					smsd["smsd"]["text"]=root["text"];
				}

				SendEventToJNODA(FastWriteJSON(smsd));
			}
			if (serialport_gsm_enable){
				Json::Value SerialPortGSM;
				//printf("tojnoda %s\n",data);
				if (root.isMember("recipients")){
					if (root["recipients"].isArray()){
						SerialPortGSM["SerialPortGSM"]["recipients"]=root["recipients"];
					}
					else{
						SerialPortGSM["SerialPortGSM"]["recipients"][0]=root["recipients"];
					}
				}

				if (root.isMember("text")){
					SerialPortGSM["SerialPortGSM"]["text"]=root["text"];
				}

				SendEventToJNODA(FastWriteJSON(SerialPortGSM));
			}
		}
	}
	return NO_ERROR;
}

eErrorTp Reporter_loop::Configure(char * settstr){
	Json::Value sett;
	Json::Value settr;
	Json::Reader rd;
	GPRINT(NORMAL_LEVEL,"Configure reporters %s\n",settstr);
	if (rd.parse(settstr,settr)){
		//Json::Value dnksett;

		sett=settr["d"];
		//dnksett["settings"]["settings.reporters"]=settr;
		//CnT->dnk_var->Merge(dnksett);

		if (sett.isMember("repLang")){
			lang=sett["repLang"].asString();//SetValueSAST((char*)action_var.c_str(),);
			SetValueSAST("settings.reporters","repLang",lang);
		}

		if (sett["repEnable"]=="true"){
			///senders
			u32 ssize=sett["repSMS_sms_senders"].size();
			for (u32 n=0;n<ssize;n++){
				senders.push_back(sett["repSMS_sms_senders"][n][0].asString());
			}
			report_enable=true;
			if (sett["repEmailEnable"]=="true"){
				email_enable=true;
				if (sett["repEmail_nm"]=="true"){
					nodemailer_enable=true;
				}
				else
					nodemailer_enable=false;

			}
			else
				email_enable=false;

			if (sett["repSMSEnable"]=="true"){
				sms_enable=true;
				if (sett["repSMS_smsd"]=="true"){
					smsd_enable=true;
				}
				else
					smsd_enable=false;

				if (sett["repSMS_SerialPortGSM"]=="true"){
					serialport_gsm_enable=true;
				}
				else
					serialport_gsm_enable=false;
			}
			else
				sms_enable=false;

			//serialport_gsm_enable
		}
		else
			report_enable=false;
	}

	return NO_ERROR;
}

eErrorTp Reporter_loop::Loop(void* thisPtr){


	//string json="{\"t\":[12,1],\"d\":{\"action\":\"smsd\",\"smsd\":{\"smsinfile\":\"/sms\"}}}";

	//shared_fifo->SendSharedFifoMessage(srvmReportAction,"relo", json);

	while(1){
			//GPRINT(NORMAL_LEVEL,"LOOP %d\n",report_enable);
		//debugTests();
			while (GetUcastFifoMessage(FifoPreBuf,Mheader)!=ERROR)
			{
				switch(Mheader.MsgType)
				{
					case srvmJNODA_READY:
							jnoda_ready=true;
					break;
					case srvmReport:
						printf("srvmReport report_enable %d\n",report_enable);
						if (report_enable){

							GPRINT(MEDIUM_LEVEL,"got Report message [size - %d byte]\n",strlen((char*)FifoPreBuf.p()));
							Report((char*)FifoPreBuf.p());
						}
					break;
					case srvmReportSettings:
						Configure((char*)FifoPreBuf.p());
						break;
					case srvmJSON_ACTION_FORMAT:
						GPRINT(HARD_LEVEL,"srvmReportAction %s\n",FifoPreBuf.p());
						if (Action((char*)FifoPreBuf.p())==ERROR){
							GPRINT(NORMAL_LEVEL,"Action error!!! %s\n",FifoPreBuf.p());
						}
						break;
					//default:
					//GPRINT(MEDIUM_LEVEL,"undef msg type [%d]\n",Mheader.MsgType);
				}
			}

			if (TERMReq){
				break;
			}


			mdelay(loopdelay);
		}

		return NO_ERROR;
	}
