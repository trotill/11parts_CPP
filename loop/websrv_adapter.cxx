/*
 * jnoda_adapter.cxx
 *
 *  Created on: 15 дек. 2018 г.
 *      Author: root
 */


#include "loop/websrv_adapter.h"

Websrv_adapter::Websrv_adapter(eDebugTp debug_level):ThreadT("Websrv_ad","wead",debug_level),IPC_SocketT(debug_level){
		GPRINT(HARD_LEVEL,"Create Websrv_adapter\n");
		 WDT_PORT_CPP=CnT->WDT_PORT_CPP;
		 WDT_PORT_NODEJS=CnT->WDT_PORT_NODEJS;
		 NODE_GROUP_IP=CnT->NODE_GROUP_IP;
		 JSON_ReadConfigField(CnT->json_cfg,"wead_loopdelay",loopdelay);
		 //ips=make_shared<IPC_SocketT>(debug_level);
		  InitLHBus(WDT_PORT_CPP,WDT_PORT_NODEJS,(char*)NODE_GROUP_IP.c_str());
	}

eErrorTp Websrv_adapter::DelayedSendUI_JSON_BUF(u32 & web_ui_client_cnt,
		vector<string> & js_ui_cache,
		u32 & time1s_cache_delay,
		u32 & time1s_cache_delay_counter
		){

	if ((web_ui_client_cnt>0)&&(js_ui_cache.size()>0)&&(time1s_cache_delay_counter>time1s_cache_delay)){
					time1s_cache_delay_counter=0;
					//printf("run js_ui_cache delay\n");
				}
				if (web_ui_client_cnt==0)
				{
					//printf("stop js_ui_cache delay\n");
					time1s_cache_delay_counter=time1s_cache_delay+1;
				}
				if (time1s_cache_delay_counter==time1s_cache_delay){
					time1s_cache_delay_counter=time1s_cache_delay+1;
					//printf("js_ui_cache.size() %d\n",js_ui_cache.size());
					for (u32 z=0;z<js_ui_cache.size();z++){
						//printf("pop %s\n",(char*)js_ui_cache[z].c_str());
						GPRINT(MEDIUM_LEVEL,"Send POP UI %s\n",(char*)js_ui_cache[z].c_str());
						SockSendToLH_Bus((char*)js_ui_cache[z].c_str(), js_ui_cache[z].size());
					}
					//printf("js_ui_cache.clear()\n");
					js_ui_cache.clear();
				}

				if (time1s_cache_delay_counter<time1s_cache_delay)
					time1s_cache_delay_counter++;
	return NO_ERROR;
}

eErrorTp Websrv_adapter::Loop(void* thisPtr){
	u32 recv;
		sInterThrMsgHeader Mheader;
		string str;
		string data;
		string ConnectedIp="";
		u16 ConnectedPort;
		u32 ptype;
		u32 vers;

		u32 size;
		u32 web_ui_client_cnt=0;
		vector<string> js_ui_cache;
		u32 time1s_cache_delay=(1000/loopdelay)*2;
		u32 time1s_cache_delay_counter=time1s_cache_delay+1;
		GPRINT(NORMAL_LEVEL,"Cnoda first run status [%s]\n",(CnT->first_run)?"first start":"repeated start");
		while(1){
			if (skey==false){
				SendSystemFromCnodaToUI(LICENSE_ERROR_DEVICE_IS_BROKEN);
					sleep(5);
					//ips->SockSendToLH_Bus((char*)FifoPreBuf.buf, FifoPreBuf.base_len);
			}

			DelayedSendUI_JSON_BUF(web_ui_client_cnt,js_ui_cache,time1s_cache_delay,time1s_cache_delay_counter);

			while (GetUcastFifoMessage(&FifoPreBuf,Mheader)!=ERROR)
					{

					 GPRINT(PARANOIDAL_LEVEL,"Get MsgType %d\n",Mheader.MsgType);
						switch(Mheader.MsgType)
						{
							case srvmJNODA_READY:
								jnoda_ready=true;
											//printf("SRV_HELPER JNODA_READY\n");
								break;
							case srvmSOCKET_IO:
								{
									//printf("srvmSOCKET_IO\n");
									Json::Reader reader;
									Json::Value root;
									if (reader.parse((char*)FifoPreBuf.buf,root)){
									//	printf("json dump %s\n",FifoPreBuf.buf);
										web_ui_client_cnt=root["UserCount"].asInt();
										GPRINT(NORMAL_LEVEL,"Connected clients %d\n",web_ui_client_cnt);
										//sleep(2);
									}
									else
									{
										GPRINT(NORMAL_LEVEL,"error parse %s\n",FifoPreBuf.buf);
									}

								}
								break;
							case srvmSendUI_JSON_BUF:
								//printf("srvmSendUI_JSON_BUF\n");
								FifoPreBuf.buf[FifoPreBuf.base_len]=0;
								if (web_ui_client_cnt==0){
									//pop_front
									if (js_ui_cache.size()>=MAX_ITEM_WEB_UI_JSON){
										pop_front(js_ui_cache);
										GPRINT(MEDIUM_LEVEL,"POP UI\n");
									}
									if (js_ui_cache.size()<MAX_ITEM_WEB_UI_JSON){
										//printf("push %s\n",(char*)FifoPreBuf.buf);
										js_ui_cache.push_back(string((char*)FifoPreBuf.buf));
										GPRINT(MEDIUM_LEVEL,"PUSH UI [%s]\n",FifoPreBuf.buf);
									}
									//printf("js_ui_cache.size() %d\n",js_ui_cache.size());
								}
								else{
									//printf("SockSendToLH_Bus %s\n",(char*)FifoPreBuf.buf);
									SockSendToLH_Bus((char*)FifoPreBuf.buf, FifoPreBuf.base_len);
									GPRINT(MEDIUM_LEVEL,"Resend UI [%s]\n",FifoPreBuf.buf);
								}
								break;
							case srvmSendToLH:

								FifoPreBuf.buf[FifoPreBuf.base_len]=0;
								SockSendToLH_Bus((char*)FifoPreBuf.buf, FifoPreBuf.base_len);

								GPRINT(HARD_LEVEL,"Resend [%s]\n",FifoPreBuf.buf);
								break;
							default:
								GPRINT(MEDIUM_LEVEL,"undef msg type [%d]\n",Mheader.MsgType);
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
