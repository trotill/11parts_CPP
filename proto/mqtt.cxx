/*
 * mqtt.cxx
 *
 *  Created on: 18 июл. 2019 г.
 *      Author: root
 */

#include "mqtt.h"

	char mqtt_password[100];
	char mqtt_login[100];
	mqtt_client::mqtt_client(string password,string login,string url,string id,int keepAliveInterval,eDebugTp level,string topic){
		//this->password=password;
		//this->login=login;

		//strncpy(mqtt_password,(char*)password.c_str(),sizeof(mqtt_password));
		//strncpy(mqtt_login,(char*)login.c_str(),sizeof(mqtt_login));
		mqtt_password=password;
		mqtt_login=login;
		if ((password.length()!=0)&&(login.length()!=0))
		{

			conn_opts.password=mqtt_password.c_str();///this->password.c_str();
			conn_opts.username=mqtt_login.c_str();//this->login.c_str();
			//printf("mqtt_client Set secure %s/%s\n",conn_opts.password,conn_opts.username);
		}
		mqtt_client_init(url,id,keepAliveInterval,level,topic);


	}

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
   // deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    int i;
    char* payloadptr;

    mqtt_client * parents=(mqtt_client*)context;

    //printf("get public %s\n",topicName);
   // if (parents->RecvFifo.fifo_item.count(topicName)>0)
    	//parents->RecvFifo.fifo_item[topicName]->FFifo.PushBuf((u8*)message->payload,message->payloadlen);
   // else
    //	printf("err public %s\n",topicName);

    parents->RecvFifo.PushBuf((u8*)message->payload,message->payloadlen);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}
void connlost(void *context, char *cause) {
	mqtt_client * parents=(mqtt_client*)context;
   // printf("\nConnection lost\n");
   // printf("     cause: %s\n", cause);
	parents->GPRINT(NORMAL_LEVEL,"Connection lost\n");
	parents->connst=false;
	//parents->mqtt_reinit();
	//MQTTClient_disconnect(parents->client, parents->timeout_ms);
}


