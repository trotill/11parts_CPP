/*
 * mqtt.h
 *
 *  Created on: 28 июн. 2019 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_PROTO_MQTT_H_
#define SRC_ENGINE_PROTO_MQTT_H_



#include <MQTTClient.h>
#include <engine/types.h>
#include <engine/basic.h>
#include <engine/fifo.h>
#include <engine/lib/11p_bin.h>

void delivered(void *context, MQTTClient_deliveryToken dt) ;
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void connlost(void *context, char *cause) ;
#define MQTT_MAX_PUBLICS 10


class mqtt_client_rx_fifo{
	public:
	string topic="";

};
class mqtt_client: public GprintT {
	public:
	eErrorTp mqtt_client_init(string url,string id,int keepAliveInterval,eDebugTp level,string topic){
		if (client!=NULL)
			return ERROR;

		url_arg=url;
		id_arg=id;
		keepAliveInterval_arg=keepAliveInterval;
		level_arg=level;
		subscribe_topic=topic;
		//multi_public_arg=multi_pub;


		debug_level=level;
		SourceStr="MQTT";
		ObjPref=id;
		mutex_init(Mutex);
		//multi_public=multi_pub;
		//conn_opts.MQTTVersion = MQTTVERSION_3_1_1;

		if (MQTTClient_create(&client, url.c_str(), id.c_str(),
			MQTTCLIENT_PERSISTENCE_NONE, NULL)==MQTTCLIENT_SUCCESS){
			conn_opts.keepAliveInterval = keepAliveInterval;
			conn_opts.cleansession = 1;
		}
		else
			client=NULL;

		if (client==NULL){
			return ERROR;
		}

		timeout_ms=conn_opts.keepAliveInterval*1000;

		MQTTClient_setCallbacks(client, this, connlost, msgarrvd, NULL);//delivered);
		GPRINT(NORMAL_LEVEL,"Create MQTT client url %s id %s\n",url.c_str(),id.c_str());
		//rc_buf=make_unique<char[]>(MQTT_TMP_BUF);
		return NO_ERROR;
	}
	mqtt_client(string url,string id,int keepAliveInterval,eDebugTp level,string topic){
		mqtt_client_init(url,id,keepAliveInterval,level,topic);
	}

	mqtt_client(string password,string login,string url,string id,int keepAliveInterval,eDebugTp level,string topic);

	eErrorTp ReInit(){
		GPRINT(NORMAL_LEVEL,"ReInit\n");
		mqtt_client_destroy();
		if (MQTTClient_create(&client, url_arg.c_str(), id_arg.c_str(),
					MQTTCLIENT_PERSISTENCE_NONE, NULL)!=MQTTCLIENT_SUCCESS){
			MQTTClient_destroy(&client);
			return ERROR;
		}
		else{
			MQTTClient_setCallbacks(client, this, connlost, msgarrvd, NULL);//delivered);
			GPRINT(NORMAL_LEVEL,"Create MQTT client url %s id %s\n",url_arg.c_str(),id_arg.c_str());
			return NO_ERROR;
		}
		return NO_ERROR;
	}
	void mqtt_client_destroy(){
		if (client!=NULL){
			GPRINT(NORMAL_LEVEL,"Destroy MQTT client\n");
			if (connst){
				MQTTClient_disconnect(client, timeout_ms);
			}
			MQTTClient_destroy(&client);
			client=NULL;
		}
	}

	~mqtt_client(){
		mqtt_client_destroy();
	}

#if 0
	eErrorTp CheckAndInit(){
		if (client==NULL){
			mqtt_client(url_arg,id_arg,keepAliveInterval_arg,level_arg,multi_public_arg);
			if (client!=NULL)
				return NO_ERROR;
			else
				return ERROR;
		}
		return NO_ERROR;
	}
#endif
	eErrorTp publish_sync(u8 * data,u32 datalen,char * topic){

	    //if (CheckAndInit()==ERROR){
	    //	return ERROR;
	    //}
		mutex_lock(Mutex);


		if (client==NULL){
			GPRINT(NORMAL_LEVEL,"Error:publish_sync, client context is null\n");
			mutex_unlock(Mutex);
			return ERROR;
		}

		if (connst==false){
			if (connect()==ERROR){
				mutex_unlock(Mutex);
				return ERROR;
			}
		}

		 if (MQTTClient_isConnected(client)==false){
			 disconnect();
			 if (connect()==ERROR){
				 printf("m2.4\n");
				 mutex_unlock(Mutex);
				 return ERROR;
			 }
		 }

		 pubmsg.payload = data;
		 pubmsg.payloadlen = datalen;
		 pubmsg.qos = qos;
		 pubmsg.retained = 0;
		 if (MQTTClient_publishMessage(client, topic, &pubmsg, &token)!=MQTTCLIENT_SUCCESS){
			 mutex_unlock(Mutex);
			 return ERROR;
		 }

		 if (MQTTClient_waitForCompletion(client, token, timeout_ms)!=MQTTCLIENT_SUCCESS){
			 mutex_unlock(Mutex);
			 return ERROR;
		 }

		 mutex_unlock(Mutex);
		 return NO_ERROR;
	}
	eErrorTp publish_sync(string data,string topic){
		return publish_sync((u8*)data.c_str(),(u32)data.size(),(char*)topic.c_str());
	}

	eErrorTp publish_sync(string data){
		return publish_sync((u8*)data.c_str(),(u32)data.size(),"cnoda");
	}

	eErrorTp subscribe(void){
		if (client==NULL){
				GPRINT(NORMAL_LEVEL,"Error:MQTT subscribe, client context is null\n");
				return ERROR;
		}

		if (subscribe_topic.length()==0){
			GPRINT(NORMAL_LEVEL,"Error: make correct topic name, topic length is 0!!!\n");
			return ERROR;
		}

		if (MQTTClient_isConnected(client)==false){
				disconnect();
				if (connect()==ERROR){
					GPRINT(NORMAL_LEVEL,"Error mqtt client unconnect %s id %s, from subscribe\n",subscribe_topic.c_str(),id_arg.c_str());
					return ERROR;
				}
		}

		if (subscribed)
			if(MQTTClient_unsubscribe(client,subscribe_topic.c_str())!=MQTTCLIENT_SUCCESS){
				GPRINT(NORMAL_LEVEL,"Error unsubscribe on %s id %s\n",subscribe_topic.c_str(),id_arg.c_str());
				//return ERROR;
			}

		if (MQTTClient_subscribe(client, (char*)subscribe_topic.c_str(), qos)!=MQTTCLIENT_SUCCESS){
			GPRINT(NORMAL_LEVEL,"Error subscribe on %s id %s\n",subscribe_topic.c_str(),id_arg.c_str());
		}
		else{
			subscribed=true;
			GPRINT(NORMAL_LEVEL,"Success subscribe on %s id %s\n",subscribe_topic.c_str(),id_arg.c_str());
			//RecvFifo.fifo_item.emplace(topic,make_shared<sFifoItem>(topic));
			//RecvFifo.fifo_item[topic]->FifoReady=true;
			GPRINT(NORMAL_LEVEL,"Fifo %s created id %s\n",(char*)subscribe_topic.c_str(),id_arg.c_str());
		}

#if 0
		if (topic.size()==0)
			topic=last_topic;
		else
			last_topic=topic;

		GPRINT(NORMAL_LEVEL,"Subscribe on topic %s id %s\n",topic.c_str(),id_arg.c_str());

	    if (CheckAndInit()==ERROR){
	    	return ERROR;
	    }

		if (connst==false)
			if (connect()==ERROR){
				GPRINT(NORMAL_LEVEL,"Error connect on %s id %s\n",topic.c_str(),id_arg.c_str());
				return ERROR;
			}

		if (MQTTClient_isConnected(client)==false){
				disconnect();
				if (connect()==ERROR){
					GPRINT(NORMAL_LEVEL,"Error mqtt client unconnect %s id %s\n",topic.c_str(),id_arg.c_str());
					return ERROR;
				}
		}

		u32 ps=sizeof(publics);
		bool found_copy=false;
		u8 new_item=0;
		for (u32 i=0;i<MQTT_MAX_PUBLICS;i++){
			if (publics[i]==topic){
				found_copy=true;
			}
			if (publics[i].size()==0){
				new_item=i;
				break;
			}
		}

		if ((found_copy==false)&&(topic.size()!=0)){
			if ((multi_public==false)&&(subscribed)){
				if(MQTTClient_unsubscribe(client,publics[0].c_str())!=MQTTCLIENT_SUCCESS){
					GPRINT(NORMAL_LEVEL,"Error unsubscribe on %s id %s\n",publics[0].c_str(),id_arg.c_str());
					return ERROR;
				}

				GPRINT(NORMAL_LEVEL,"unsubscribe on %s id %s\n",publics[0].c_str(),id_arg.c_str());
				RecvFifo.fifo_item.erase(publics[0].c_str());
				new_item=0;
			}

			if (MQTTClient_subscribe(client, (char*)topic.c_str(), qos)!=MQTTCLIENT_SUCCESS){
				GPRINT(NORMAL_LEVEL,"Error subscribe on %s id %s\n",topic.c_str(),id_arg.c_str());
				return ERROR;
			}
			if (multi_public==false)
				GPRINT(NORMAL_LEVEL,"resubscribe on %s id %s\n",topic.c_str(),id_arg.c_str());
			else
				GPRINT(NORMAL_LEVEL,"subscribe on %s id %s\n",topic.c_str(),id_arg.c_str());
			publics[new_item]=topic;

		}
		else{
			if (topic.size()==0){
				u16 max_pub=MQTT_MAX_PUBLICS;
				if (multi_public==false){
					max_pub=1;
				}

				for (u32 i=0;i<max_pub;i++){
					if (publics[i]==""){
						break;
					}
					MQTTClient_unsubscribe(client,publics[i].c_str());
					MQTTClient_subscribe(client, (char*)publics[i].c_str(), qos);
					GPRINT(NORMAL_LEVEL,"resubscribe on %s id %s\n",publics[i].c_str(),id_arg.c_str());

				}
			}
		}
		subscribed=true;

		if (topic.size()>0){
			if (RecvFifo.fifo_item.count(topic)>0){
				GPRINT(NORMAL_LEVEL,"Found created fifo %s, delete it, id %s\n",(char*)topic.c_str(),id_arg.c_str());
				RecvFifo.fifo_item.erase(topic);
			}

			RecvFifo.fifo_item.emplace(topic,make_shared<sFifoItem>(topic));
			RecvFifo.fifo_item[topic]->FifoReady=true;
			GPRINT(NORMAL_LEVEL,"Fifo %s created id %s\n",(char*)topic.c_str(),id_arg.c_str());
		}
#endif
		return NO_ERROR;
	}

	eErrorTp receive(buffer & result,char * topic){

		if (client==NULL){
			GPRINT(NORMAL_LEVEL,"Error:MQTT receive, client context is null\n");
			return ERROR;
		}
	    //if (CheckAndInit()==ERROR){
	    //	return ERROR;
	   // }

		//if (subscribed==false){
		//	GPRINT(NORMAL_LEVEL,"Default subscribe - 'cnoda'\n");
		//	subscribe("cnoda");
		//}

		if (MQTTClient_isConnected(client)==false){
			 GPRINT(NORMAL_LEVEL,"Error:MQTT reconnect id %s\n",id_arg.c_str());
			// disconnect();
			 connst=false;
			 if (ReInit()==ERROR)
				 return ERROR;
			 if (connect()==ERROR)
				return ERROR;

			 subscribe();
		}

		//result_len=0;


		if (RecvFifo.GetCountElements()==0)
			return ERROR;

		u32 size;
		u8 * buf=RecvFifo.GetLastBuf(size);
		///if (result.size()<size){
		//	result_len=0;
		//	GPRINT(NORMAL_LEVEL,"Error, big message %d\n",size);
		//	return ERROR;
		//}
		//else{
		result.create(size+1);
		memcpy(result.p(),buf,size);
		//result_len=size;
		//}

		return NO_ERROR;

	}

	eErrorTp receive(string & result,char * topic){

	   // if (CheckAndInit()==ERROR){
	   // 	return ERROR;
	    //}


		buffer buf;
		if (receive(buf,topic)!=ERROR)
		{
			result=(char*)buf.p();
			return NO_ERROR;
		}
		return ERROR;

	}
	eErrorTp receive(string & result,string & topic){
		return receive(result,(char*)topic.c_str());
	}
	// string test="Test context";
	// FifoT RecvFifo;
	 fragfifo RecvFifo;
	 bool connst=false;
	  u32 timeout_ms=0;
	  MQTTClient client=NULL;
	private:

	eErrorTp connect(){

		if (client==NULL){
				GPRINT(NORMAL_LEVEL,"Error:MQTT connect, client context is null\n");
				return ERROR;
		}
	  //  if (CheckAndInit()==ERROR){
	   // 	return ERROR;
	    //}

		if (connst){

			disconnect();
		}

		GPRINT(HARD_LEVEL,"Try MQTTClient_connect\n");
		if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
		{

			GPRINT(NORMAL_LEVEL,"Error:MQTTClient_connect\n");
			connst=false;
			return ERROR;
		}

		GPRINT(HARD_LEVEL,"MQTTClient_connect Ok\n");
		connst=true;
		return NO_ERROR;
	}
	eErrorTp disconnect(){
		if (client==NULL){
			GPRINT(NORMAL_LEVEL,"Error:MQTT disconnect, client context is null\n");
			return ERROR;
		}
	   // if (CheckAndInit()==ERROR){
	    //	return ERROR;
	   // }


		if (connst){
			connst=false;
			GPRINT(HARD_LEVEL,"Try MQTTClient_disconnect\n");
			MQTTClient_disconnect(client, timeout_ms);
			GPRINT(HARD_LEVEL,"MQTTClient_disconnect Ok\n");
			return NO_ERROR;
		}
		else{
			GPRINT(HARD_LEVEL,"skip disconnect\n");
		}
		return ERROR;
	}


	  bool subscribed=false;

	  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	  MQTTClient_message pubmsg = MQTTClient_message_initializer;
	  MQTTClient_deliveryToken token=0;

	  string publics[MQTT_MAX_PUBLICS]={""};
	//  std::unique_ptr<char[]> rc_buf;//(new char[10000]);
	  int rc=0;
	  u8 qos=1;
		//string url,string id,int keepAliveInterval,eDebugTp level,bool multi_public
	  string url_arg="127.0.0.1";
	  string id_arg="ddsdfew";
	  int keepAliveInterval_arg=10;
	  eDebugTp level_arg=NORMAL_LEVEL;
	  bool multi_public_arg=false;
	  bool multi_public=false;
	  string  mqtt_password;
	  string mqtt_login;
	  string subscribe_topic;
	  Mutex_t Mutex;
	  //string password;
	  //string login;

};





#endif /* SRC_ENGINE_PROTO_MQTT_H_ */
