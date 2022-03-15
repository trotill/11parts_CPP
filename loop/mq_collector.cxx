#include "mq_collector.h"

Mq_collector::Mq_collector(eDebugTp debug_level):ThreadT("MQ_collect","mqco",debug_level){
		GPRINT(HARD_LEVEL,"Create Mq_collector\n");
		 JSON_ReadConfigField(CnT->json_cfg,"mqco_loopdelay",loopdelay);

}

eErrorTp Mq_collector::Action(char * json){
	Json::Value action;
	GPRINT(HARD_LEVEL,"Get Action %s\n",json);
	if (JSON_ParseString(action,(char*)json)==NO_ERROR){
		//zr();
		if (action["d"].isMember("mq_send")){
			/*{
			 *  d:{
			 *    action:mq_send,
			 *    mq_send:{
			 *      mqProto:mqtt
			 *      mqMQTT_topics:["topic"],
			 *      mqMQTT_send:string data
			 *    }
			 *  }
			 *}
			 */
			//zr();
			Send(action["d"]["mq_send"]);
		}
		if (action["d"].isMember("mq_add_exchanger")){
			/*{
			 *  d:{
			 *    action:mq_add_exchanger,
			 *    mq_add_exchanger:{
			 *      mqProto:mqtt
			 *      mqMQTT_Exchanger:thread id,
			 *    }
			 *  }
			 *}
			 */
			AddExchanger(action["d"]["mq_add_exchanger"]);
		}
		if (action["d"].isMember("mq_del_exchanger")){
			/*{
			 *  d:{
			 *    action:mq_del_exchanger,
			 *    mq_del_exchanger:{
			 *      mqProto:mqtt
			 *      mqMQTT_Exchanger:thread id,
			 *    }
			 *  }
			 *}
			 */
			DelExchanger(action["d"]["mq_del_exchanger"]);
		}
		if (action["d"].isMember("mq_configure")){
			/*{
			 *  d:{
			 *    action:mq_configure,
			 *    mq_configure:{
			 *      mqProto:mqtt
			 *      mqMQTT_url:URL,
			 *      mqMQTT_topics:["topic"]
			 *    }
			 *  }
			 *}
			 */
			Configure(action["d"]["mq_configure"]);
		}
	}

	return NO_ERROR;
}

eErrorTp Mq_collector::Send(Json::Value & msg){

	//printJSON("Send",msg);
	if (msg["mqProto"]=="mqtt"){
		if (mqtt_enabled==false){
			return ERROR;
		}
		//zr();
		string topic=msg["mqMQTT_topics"][0].asString();
		u32 buflen=msg["mqMQTT_send"].asString().size();
		u8 * buf=(u8*)msg["mqMQTT_send"].asCString();
		//zr();
		//string buf=msg["mqMQTT_send"].asString();
		//u32 buflen=strlen((char*)buf)+1;
		//printf("Publish len %d\n%s\n",buflen,buf);
#ifdef _MQTT
		if (mq->publish_sync(buf,buflen,(char*)topic.c_str())==ERROR){
			if (mq->publish_sync(buf,buflen,(char*)topic.c_str())==ERROR){
				return ERROR;
			}
		}
		else
			return NO_ERROR;
#endif
	}

	return ERROR;
}
eErrorTp Mq_collector::Configure(Json::Value & sett){
	//Json::Value sett;
	//son::Reader rd;
	GPRINT(NORMAL_LEVEL,"Configure mqco collector %s\n",StyledWriteJSON(sett).c_str());

		//Json::Value dnksett;

	//sett=msg;
		//dnksett["settings"]["settings.reporters"]=settr;
		//CnT->dnk_var->Merge(dnksett);

	//zr();
	if (sett["mqProto"]=="mqtt"){
		//zr();
#ifdef _MQTT
		string url=sett["mqMQTT_url"].asString();
		//zr();
		string mqtt_id=RandomCStr("Mq_coll");
		//zr();
		string topic=sett["mqMQTT_topics"][0].asString();
		//zr();
		rx_topic=topic;
		if (mqtt_enabled)
			mq.reset();

		mq=make_shared<mqtt_client>(url,mqtt_id,MQTT_KEEP_ALIVE_INTERVAL,debug_level,topic);
		mq->subscribe();

		mqtt_enabled=true;
		GPRINT(NORMAL_LEVEL,"MQTT configured url %s id %s topic %s\n",(char*)url.c_str(),(char*)mqtt_id.c_str(),(char*)topic.c_str());
		SendSharedFifoMessage(srvmMQ_MQTT_READY,"all");
#endif
		return NO_ERROR;
	}



	return ERROR;
}

eErrorTp Mq_collector::AddExchanger(Json::Value & msg){
	if (msg["mqProto"]=="mqtt"){

		string exchanger=msg["mqMQTT_Exchanger"].asString();
		GPRINT(NORMAL_LEVEL,"MQTT Try add exchanger %s\n",(char*)exchanger.c_str());
		if (std::find(mqtt_exchangers.begin(), mqtt_exchangers.end(), exchanger) == mqtt_exchangers.end()){
			mqtt_exchangers.push_back(msg["mqMQTT_Exchanger"].asString());
			GPRINT(NORMAL_LEVEL,"MQTT added exchanger %s\n",(char*)exchanger.c_str());
		}


	}
	return NO_ERROR;
}
eErrorTp Mq_collector::DelExchanger(Json::Value & msg){

	if (msg["mqProto"]=="mqtt"){
		string exchanger=msg["mqMQTT_Exchanger"].asString();
		GPRINT(NORMAL_LEVEL,"MQTT Try del exchanger %s\n",(char*)exchanger.c_str());
		auto it=std::find(mqtt_exchangers.begin(), mqtt_exchangers.end(), exchanger);
		if (it!= mqtt_exchangers.end()){
			mqtt_exchangers.erase(it);
			GPRINT(NORMAL_LEVEL,"MQTT deleted exchanger %s\n",(char*)exchanger.c_str());
		}
	}

	return NO_ERROR;
}

eErrorTp Mq_collector::NecronLang(string & msg){
	//GPRINT(NORMAL_LEVEL,"NecronLang CnT->version %s\n",CnT->version);
	if (CnT->version!="dbg")
		return NO_ERROR;

	//GPRINT(NORMAL_LEVEL,"workbook %s\n",msg.c_str());
	if ((msg.size()>13)&&(memcmp("{\"x\":{\"srclang\":",msg.c_str(),13)==0)){
		GPRINT(NORMAL_LEVEL,"Fill workbook\n");
		Json::Value root;
		if (JSON_ParseString(root,msg)!=ERROR){
			if (root["x"].isMember("srclang")&&root["x"].isMember("wordbook"))
			{
				//CnT->dnk_var->SetValue("x.srclang",root["x"]["srclang"]);
				//CnT->dnk_var->SetValue("x.wordbook",root["x"]["wordbook"]);
				//CnT->dnk_var->Dump("",false);
				SettingsAdm Sm;

				string selLng=root["x"]["srclang"].asString();
				CnT->lang["x"]["srclang"]=selLng;
				CnT->lang["x"]["deflang"]=root["x"]["deflang"];
				//printf("sel Lng %s lang %s\n",selLng.c_str(),FastWriteJSON(root["x"]["wordbook"]).c_str());
				CnT->lang["x"]["wordbook"][selLng]=root["x"]["wordbook"][selLng];
				//CnT->dnk_var->GetElementWithPath("",setting);
				Sm.SaveSetting(FastWriteJSON(CnT->lang),"settings.lang");
			}
		}
	}
	return NO_ERROR;
}

eErrorTp Mq_collector::GotMessage(){

	//string topic="";
	string mq_result="";
	if (mqtt_enabled==false){
		return ERROR;
	}
	else{
#ifdef _MQTT
		if (mq->receive(mq_result,rx_topic)!=ERROR){
			GPRINT(NORMAL_LEVEL,"MQTT got message %s\n",(char*)mq_result.c_str());
			for (u32 n=0;n<mqtt_exchangers.size();n++){
				GPRINT(NORMAL_LEVEL,"MQTT resend msg to exchanger %s\n",(char*)mqtt_exchangers[n].c_str());
				SendSharedFifoMessage(srvmMQ_MESSAGE,mqtt_exchangers[n], mq_result);
				NecronLang(mq_result);
			}
			if (echo_enabled){
				GPRINT(NORMAL_LEVEL,"MQTT echo msg to topic %s, %s\n",(char*)echo_topic.c_str(),(char*)mq_result.c_str());
				//mq->publish_sync((u8*)mq_result.c_str(),mq_result.size()+1,(char*)echo_topic.c_str());
			}
		}
#endif
	}
	return NO_ERROR;
}
eErrorTp Mq_collector::Loop(void* thisPtr){

	while(1){


		GotMessage();
		while (GetUcastFifoMessage(FifoPreBuf,Mheader)!=ERROR)
		{
			switch(Mheader.MsgType)
			{
					case srvmJSON_ACTION_FORMAT:
						//GPRINT(NORMAL_LEVEL,"srvmJSON_ACTION_FORMAT %s\n",FifoPreBuf.buf);
						Action((char*)FifoPreBuf.p());
					break;
			}
			if (TERMReq)
			{
				break;
			}
		}

		if (TERMReq)
		{
			break;
		}
		mdelay(loopdelay);
	}
	return NO_ERROR;
}
