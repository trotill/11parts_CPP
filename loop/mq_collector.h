/*
 * mqtt_ttcollector.h
 *
 *  Created on: 1 мая 2020 г.
 *      Author: root
 */



#include "engine/thread.h"
#include "engine/ipc_socket.h"
#include "engine/node_req.h"
#include "engine/global.h"
#ifdef _MQTT
#include "engine/proto/mqtt.h"
#endif

#ifndef FROM_SW_TO_ALL_MQTT_TOPIC
	#define FROM_SW_TO_ALL_MQTT_TOPIC "SWtoSRVa"
#endif

#ifndef FROM_CNODA_TO_SW_MQTT_TOPIC
	#define FROM_CNODA_TO_SW_MQTT_TOPIC "SRVtoSW"
#endif


class Mq_collector  : public ThreadT {
	public:
	Mq_collector(eDebugTp debug_level);
	virtual ~Mq_collector (void){
		GPRINT(HARD_LEVEL,"Destroy Mqtt_ttcollector\n");
	}
	eErrorTp Action(char * json);
	eErrorTp Configure(Json::Value & msg);
	eErrorTp AddExchanger(Json::Value & msg);
	eErrorTp DelExchanger(Json::Value & msg);
	eErrorTp Send(Json::Value & msg);
	eErrorTp GotMessage();
	eErrorTp NecronLang(string & msg);
	private:
	buffer FifoPreBuf;
		virtual eErrorTp Loop(void* thisPtr);

#ifdef _MQTT
			u32 loopdelay=50;
			shared_ptr<mqtt_client> mq;
#else
			u32 loopdelay=100;
#endif
		bool mqtt_enabled=false;
		bool echo_enabled=false;

		string echo_topic=FROM_CNODA_TO_SW_MQTT_TOPIC;
		string rx_topic=FROM_SW_TO_ALL_MQTT_TOPIC;

		vector<string> mqtt_exchangers;
};

/* Example MQTT
Init
eErrorTp MQTT_setup(){
	Json::Value cfg;
	cfg["mqProto"]="mqtt";
	cfg["mqMQTT_url"]=mqtt_addr;
	cfg["mqMQTT_topics"][0]="rx_topic";
	SendActionLocal(srvmJSON_ACTION_FORMAT,"mqco","mq_configure",cfg);
	Json::Value exch;
	exch["mqProto"]="mqtt";
	exch["mqMQTT_Exchanger"]="felo";
	SendActionLocal(srvmJSON_ACTION_FORMAT,"mqco","mq_add_exchanger",exch);
	exch["mqMQTT_Exchanger"]="drlo";
	SendActionLocal(srvmJSON_ACTION_FORMAT,"mqco","mq_add_exchanger",exch);
	return NO_ERROR;
}

Send
eErrorTp SendMQ(char * topic,string & data){
	Json::Value send;
	send["mqProto"]="mqtt";
	send["mqMQTT_topics"][0]="tx_topic";
	send["mqMQTT_send"]=data;
	SendActionLocal(srvmJSON_ACTION_FORMAT,"mqco","mq_send",send);
	return NO_ERROR;
}

Read
while (GetUcastFifoMessage(&FifoPreBuf,Mheader)!=ERROR)
{

 GPRINT(PARANOIDAL_LEVEL,"Get MsgType %d\n",Mheader.MsgType);
	switch(Mheader.MsgType)
	{
		case srvmMQ_MESSAGE:
			if (reader.parse((char*)FifoPreBuf.buf_base , json )){
				GPRINT(NORMAL_LEVEL,"Get parse ok\n");
				RecvFromFE(json);
			}
			break;
	}
}

upstart
AddModule<Febe_loopT>(string("felo"),THREAD_RUN_TYPE_RESTART_ONMSG,srvmMQ_MQTT_READY);
*/

