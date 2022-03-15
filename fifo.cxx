/*
 * fifi.cxx
 *
 *  Created on: 20 дек. 2018 г.
 *      Author: root
 */

#include "fifo.h"
#include "engine/proto/json_proto.h"
#include "engine/lib/11p_string.h"
#include "engine/global.h"
#include "engine/algo/base64.h"
#include "engine/gzip/compress.hpp"

shared_ptr<CnodaT> CnT;
static FifoT FifoTObj;

FifoEngineT::FifoEngineT (string printname,string objname,eDebugTp debug_level)
			{
				Json::Value objnameJson;
				Json::Reader rd;

				if (rd.parse(objname,objnameJson)){
					for (u32 n=0;n<objnameJson.size();n++){
						oname.emplace_back(objnameJson[n].asString());
					}
				}
				else
					oname.emplace_back(objname);

				this->SourceStr=printname;
				this->ObjPref=oname[0];//objname;
				//this->SrvType=srvtUNDEF;
				this->debug_level=debug_level;

				this->objname=oname[0];//objname;

				mutex_lock(FifoTObj.mutex);

				for (auto & onameElement:oname){
					auto it= FifoTObj.fifo_item.find(onameElement);
					if (it == FifoTObj.fifo_item.end()){
						GPRINT(NORMAL_LEVEL,"Found created fifo %s, delete it\n",(char*)onameElement.c_str());
						FifoTObj.fifo_item.erase(onameElement);
					}
					FifoTObj.fifo_item.emplace(onameElement,make_shared<sFifoItem>(onameElement));
					FifoTObj.fifo_item[onameElement]->FifoReady=true;
					GPRINT(NORMAL_LEVEL,"Fifo %s created\n",(char*)onameElement.c_str());
				}
				//FifoTObj.thread_fifo[k].FifoReady=true;

				mutex_unlock(FifoTObj.mutex);
				//sFifo.FFifo[SrvType]=&Fifo;
			};

 	 FifoEngineT::~FifoEngineT (void){
			GPRINT(NORMAL_LEVEL,"%s stoped\n",(char*)SourceStr.c_str());
			mutex_lock(FifoTObj.mutex);
			for (auto & onameElement:oname){
				FifoTObj.fifo_item[onameElement]->FifoReady=false;
				u32 size;
				u8 * gbuf=NULL;
				do {
					gbuf=FifoTObj.fifo_item[onameElement]->FFifo.GetLastBuf(size);//fifo->FFifo.GetLastBuf(size);
				} while(gbuf!=NULL);

				// for(auto it = c.begin(); it != c.end(); )

				FifoTObj.fifo_item.erase(onameElement);
			}
			mutex_unlock(FifoTObj.mutex);
		}

 	 	 eErrorTp FifoEngineT::GetUcastFifoMessage(buffer & buf, sInterThrMsgHeader & MType)
 	 	 {
 	 		 return GetUcastFifoMessage(buf,MType.MsgType,objname);
 	 	 }
 	 	 eErrorTp FifoEngineT::GetUcastFifoMessage(buffer & buf, u8 & MsgType, string fifoName)
 			{
 				u32 size;
 				u8 * gbuf;
 				eErrorTp err=ERROR;
 				//printf("0m buf=0x%x buf->buf_base=0x%x\n",buf,buf->buf_base);

 				//memset(&Mheader,0,sizeof(sInterThrMsgHeader));
 				mutex_lock(FifoTObj.mutex);
 				try{
 					bool foundOname=false;
 					for (auto & fname:oname){
 						if (fname==fifoName){
 							foundOname=true;
 						}
 					}
 					if (!foundOname)
 						throw ERROR;

 					gbuf=FifoTObj.fifo_item[fifoName]->FFifo.GetLastBuf(size);

 					if (gbuf==NULL)
 						throw ERROR;

 					if (size==0)
 					{
 						GPRINT(NORMAL_LEVEL,"Get pack witch 0 size %s\n",__func__);
 						throw ERROR;
 					}
 					u32 hlen=sizeof(sInterThrMsgHeader);

 					if (size<hlen) {
 						GPRINT(NORMAL_LEVEL,"Critical error size<hlen%s pack size %d hlen size %d\n",__func__,size,sizeof(sInterThrMsgHeader));
 						throw ERROR;
 					}


 					buf.create(size-hlen);
 					buf.p(0,&gbuf[hlen],size-hlen);

 					sInterThrMsgHeader Mheader;
 					memcpy(&Mheader,gbuf,hlen);
 					MsgType=Mheader.MsgType;

 					//printhex((u8*)&Mheader,hlen,16);
 					err=NO_ERROR;
 				}
 				catch(eErrorTp e){
 					err=e;
 				}
 				mutex_unlock(FifoTObj.mutex);

 				return err;
 			}

		eErrorTp FifoEngineT::GetUcastFifoMessage(BufHandler * buf, sInterThrMsgHeader & Mheader)
		{
			u32 size;
			u8 * gbuf;
			eErrorTp err=ERROR;
			//printf("Used obsolete GetUcastFifoMessage");
			//printf("0m buf=0x%x buf->buf_base=0x%x\n",buf,buf->buf_base);
			ClearBuf(buf);
			memset(&Mheader,0,sizeof(sInterThrMsgHeader));
			mutex_lock(FifoTObj.mutex);
			try{
				gbuf=FifoTObj.fifo_item[objname]->FFifo.GetLastBuf(size);

			//printf("1m\n");
				if (gbuf==NULL)
					throw ERROR;

				if (size==0)
				{
					GPRINT(NORMAL_LEVEL,"Get pack witch 0 size %s\n",__func__);
					throw ERROR;
				}
				u32 hlen=sizeof(sInterThrMsgHeader);

				if (size<hlen)
					GPRINT(NORMAL_LEVEL,"Critical error size<hlen%s pack size %d hlen size %d\n",__func__,size,sizeof(sInterThrMsgHeader));

			//printf("2m\n");

				if (size>(buf->free_size+hlen))
				{
					//GPRINT(NORMAL_LEVEL,"Critical error size<hlen%s pack size %d hlen size %d\n",__func__,size,sizeof(sInterThrMsgHeader));
					GPRINT(NORMAL_LEVEL,"Critical error size[%d]>(buf->free_size[%d]+hlen[%d]) %s\n",size,buf->free_size,hlen,__func__);
					throw ERROR;
				}
			//	GPRINT(NORMAL_LEVEL,"Get pack witch size %d\n",size);
		//	printf("3m\n");
				//printf("pop1 %s\n",objname.c_str());
				//printhex(gbuf,size,16);
				CopyToBuf(buf,&gbuf[hlen],size-hlen);
				//printhex(buf->buf,size-hlen,16);

				buf->len=size-hlen;
				buf->base_len=buf->len;
				buf->free_size-=buf->len;

			//printf("4m\n");
				memcpy(&Mheader,gbuf,hlen);
				if (Mheader.MsgType==srvmSKEY){
					if (memcmp(buf->buf,"][",2)==0){
						skey=true;
					}
				}
				//printhex((u8*)&Mheader,hlen,16);
				err=NO_ERROR;
			}
			catch(eErrorTp e){
				err=e;
			}
			mutex_unlock(FifoTObj.mutex);
			//printf("5m\n");
			return err;
		}

		eErrorTp FifoEngineT::SendSharedFifoMessage(u8 Msg,string Dest, Json::Value & data){
			string str=FastWriteJSON(data);
			GPRINT(HARD_LEVEL,"Send %s to %d/%s\n",str.c_str(),Msg,Dest.c_str());
			return SendSharedFifoMessage(Msg,Dest, str);
		}
		eErrorTp FifoEngineT::SendSharedFifoMessage(u8 Msg,string Dest, u8 * Data=NULL,u32 DataLen=0)
		{
			eErrorTp err=NO_ERROR;
			BufHandler Buf;

			u32 BufSize=(DataLen+3)*2;
			sInterThrMsgHeader Mheader;

			try{
				if (InitBuf(&Buf)==ERROR) throw ERROR;
				if (AllocBuf(&Buf,BufSize)==ERROR) throw ERROR;

				if (AddToBuf(&Buf, Data, DataLen)==ERROR) throw ERROR;
				Mheader.MsgType=Msg;
				if (SafeMoveToStartPointerBuf(&Buf)==ERROR) throw ERROR;
				if (Data!=NULL)
					GPRINT(HARD_LEVEL,"Send over fifo msg len [%d] msg type %d to %s\n",DataLen,Mheader.MsgType,Dest.c_str());
				if (Dest!="all"){
					if (SendUcastFifoMessage(&Buf, Mheader, Dest)==ERROR) throw ERROR;
				}
				else{
					if (SendMcastFifoMessage(&Buf, Mheader)==ERROR) throw ERROR;
				}
				FreeBuf(&Buf);
			}catch(eErrorTp e)
			{
				FreeBuf(&Buf);
				return ERROR;
			}

			return err;

		}

		eErrorTp FifoEngineT::SendSharedFifoMessage(u8 Msg,string Dest, string & data)
		{
					return SendSharedFifoMessage(Msg,Dest,(u8*)data.c_str(),data.size()+1);
		}

		eErrorTp FifoEngineT::SendUcastFifoMessage(BufHandler * buf, sInterThrMsgHeader & Mheader, string Dest)
		{
			BufHandler bufsend;
			eErrorTp err=ERROR;

			//printf("FifoTObj.FifosReady %d\n",FifoTObj.FifosReady);
			//if (FifoTObj.FifosReady==false) return ERROR;

			BufClone(buf,&bufsend);

			bufsend.buf=buf->buf+buf->base_len;
			bufsend.max_size-=buf->base_len;

			if ((buf->len+sizeof(sInterThrMsgHeader))>bufsend.max_size)
			{
				GPRINT(NORMAL_LEVEL,"Critical error %s\n",__func__);
				return ERROR;
			}

			memcpy(bufsend.buf,&Mheader,sizeof(sInterThrMsgHeader));

			memcpy(&bufsend.buf[sizeof(sInterThrMsgHeader)],buf->buf,buf->len);

			bufsend.len=sizeof(sInterThrMsgHeader)+buf->len;


			//printf("flock\n");
			mutex_lock(FifoTObj.mutex);
			//u32 fsz=FifoTObj.thread_fifo.size();
			//printf("FifoTObj.thread_fifo.size() %d\n",FifoTObj.thread_fifo.size());
			u32 n=0;

			auto it= FifoTObj.fifo_item.find(Dest);
			if (it != FifoTObj.fifo_item.end()){
				//printf("push1\n");
				//printhex(bufsend.buf,bufsend.len,16);
				FifoTObj.fifo_item[Dest]->FFifo.PushBuf(bufsend.buf,bufsend.len);
				err=NO_ERROR;
			}
		//	if (n==fsz)
			//	err = ERROR;

			mutex_unlock(FifoTObj.mutex);
			//printf("funlock\n");
			return err;

		}


		eErrorTp FifoEngineT::SendMcastFifoMessage(BufHandler * buf,sInterThrMsgHeader & Mheader)
				{

					BufHandler bufsend;

					//if (FifoTObj.FifosReady==false) return ERROR;

					BufClone(buf,&bufsend);

					bufsend.buf=buf->buf+buf->base_len;
					bufsend.max_size-=buf->base_len;

					if ((buf->len+sizeof(sInterThrMsgHeader))>bufsend.max_size)
					{
						GPRINT(NORMAL_LEVEL,"Critical error %s\n",__func__);
						return ERROR;
					}

					memcpy(bufsend.buf,&Mheader,sizeof(sInterThrMsgHeader));

					memcpy(&bufsend.buf[sizeof(sInterThrMsgHeader)],buf->buf,buf->len);

					bufsend.len=sizeof(sInterThrMsgHeader)+buf->len;

					mutex_lock(FifoTObj.mutex);

					bool trig=false;
					for (auto it = FifoTObj.fifo_item.begin(); it != FifoTObj.fifo_item.end(); ++it){
						if (it->first!=objname){
							it->second->FFifo.PushBuf(bufsend.buf,bufsend.len);
							trig=true;
						}


					     // cout << (*it).first << " : " << (*it).second << endl;
					  }
					mutex_unlock(FifoTObj.mutex);

					if (trig)
						return NO_ERROR;
					else
						return ERROR;
				}


		eErrorTp FifoEngineT::SendReport_eMail(vector<string> & email_addr,string data,bool html)
		{
			Json::Value root;
			if (html)
				root["data_format"][0]="html";
			else
				root["data_format"][0]="text";

			if (email_addr.size()!=0){
				for(u32 n=0;n<email_addr.size();n++){
					root["recipients"][n]=email_addr[n];
				}
			}

			root[root["data_format"][0].asString()]=data;
			root["type"]="email";

			SendReport(root);
			return NO_ERROR;
		}

		eErrorTp FifoEngineT::SendReport_eMail(vector<string> & email_addr,string data,string subject,vector<string> & file,bool html)
		{
			Json::Value root;
			if (html)
				root["data_format"][0]="html";
			else
				root["data_format"][0]="text";
			root[root["data_format"][0].asString()]=data;

			if (email_addr.size()!=0){
				for(u32 n=0;n<email_addr.size();n++){
					root["recipients"][n]=email_addr[n];
				}
			}

			if (subject.size()!=0)
				root["subject"]=subject;

			root["type"]="email";
			if (file.size()!=0){
				root["data_format"][1]="file";
				for (u32 n=0;n<file.size();n++)
					root["file"][n]=file[n];
			}
			SendReport(root);
			return NO_ERROR;
		}

		eErrorTp FifoEngineT::SendReport_SMS(vector<string> & tel,string data)
		{

			Json::Value root;
			root["data_format"][0]="text";
			root["text"]=data;
			if (tel.size()!=0){
				for(u32 n=0;n<tel.size();n++){
					root["recipients"][n]=tel[n];
				}
			}
			root["type"]="sms";
			SendReport(root);
			return NO_ERROR;
		}

		eErrorTp FifoEngineT::SendReport(Json::Value & repdata)
		{
			/* Format
			 * {
				 data_format:["sms/text/html/sound/file"]
				 html:""
				 text:""
				 sound:""
				 file:[""]
				 client_id:"tel/mail/sip"
				 type:"sms/smsex/email/sntp/mqtt"
				}
			 */
			if (!repdata.isMember("data_format")||
					!repdata.isMember(repdata["data_format"][0].asCString())||
					!repdata.isMember("type")){
				GPRINT(NORMAL_LEVEL,"Error:Incorrect fill report [%s]\n",StyledWriteJSON(repdata).c_str());
				return ERROR;
			}
			string rep=FastWriteJSON(repdata);

			//printf("SendReport \n%s\n",rep);
			SendSharedFifoMessage(srvmReport,"relo",rep);
			return NO_ERROR;
		}

		eErrorTp FifoEngineT::SendDataFromCnodaToWEBSRV(string dMessage,string Action)
		{

					string data="{}";
					string sid="";
					//if (user.length()!=0){
					//	sid=string_format(",\"sid\":\"%s\"",(char*)user.c_str());
					//}
					if (dMessage[0]=='{'){
						data=dMessage;
					}
					else
						data=string_format("\"%s\"",dMessage.c_str());
							//
					string container=string_format("{\"t\":[%d,%d],\"d\":{\"action\":\"%s\",\"%s\":%s}}",JSON_PACK_TYPE_TO_WEB_SRV,JSON_PACK_VERSION,Action.c_str(),Action.c_str(),data.c_str());

					GPRINT(MEDIUM_LEVEL,"SendDataFromCnodaToWEBSRV\n");
					return SendSharedFifoMessage(srvmSendToLH,"wead", (u8*)container.c_str(),container.size());
		}


		eErrorTp FifoEngineT::RequestEventFromCnodaToJnoda_base(char * iface,char * reqname,char * reqvalue, u32 timeout){
			//ex. RequestEventFromCnodaToJnoda_base("eth0","ifconf","ip", 5000);
			string req;
			if ((iface!=NULL)||(strlen(iface)!=0))
				req=sf("{\"action\":\"upd_evmap\",\"upd_evmap\":{\"t\":[%d,1],\"d\":{\"iface\":\"%s\",\"req\":\"%s\",\"value\":\"%s\",\"req_t\":%d}}}",JSON_PACK_TYPE_REQ_EVENT,iface,reqname,reqvalue,timeout);
			else
				req=sf("{\"action\":\"upd_evmap\",\"upd_evmap\":{\"t\":[%d,1],\"d\":{\"req\":\"%s\",\"value\":\"%s\",\"req_t\":%d}}}",JSON_PACK_TYPE_REQ_EVENT,reqname,reqvalue,timeout);

			string jsons=string_format("{\"t\":[%d,%d],\"d\":%s,\"id\":\"%s\"}",JSON_PACK_TYPE_TO_WEB_SRV,JSON_PACK_VERSION,req.c_str(),"cnoda");

			return SendSharedFifoMessage(srvmSendToLH,"wead", (u8*)jsons.c_str(),jsons.size());

		}
		eErrorTp FifoEngineT::SendDataFromCnodaToUI_base(string dMessage,u32 json_pack_type,u32 json_pack_version, string user ,char * action)
		{

			string data="{}";
			string sid="";
			if (user.length()!=0){
				sid=string_format(",\"sid\":\"%s\"",(char*)user.c_str());
			}
			if (dMessage[0]=='{'){
				data=string_format("\"t\":[%d,%d],\"d\":{\"action\":\"%s\",\"%s\":%s,\"from\":\"cnoda\"}%s",json_pack_type,json_pack_version,action,action,dMessage.c_str(),sid.c_str());
			}
			else
				data=string_format("\"t\":[%d,%d],\"d\":{\"action\":\"%s\",\"%s\":\"%s\",\"from\":\"cnoda\"}%s",json_pack_type,json_pack_version,action,action,dMessage.c_str(),sid.c_str());
					//
			string container=string_format("{\"t\":[%d,%d],\"d\":{%s}}",JSON_PACK_TYPE_TO_UI,JSON_PACK_VERSION,data.c_str());

			GPRINT(MEDIUM_LEVEL,"SendEventFromCnoda\n");

			return SendSharedFifoMessage(srvmSendUI_JSON_BUF,"wead", (u8*)container.c_str(),container.size());
		}
		eErrorTp FifoEngineT::activationMessageActivated(){
					Json::Value needLicRoot;
					Json::Value forceLogoutRoot;
					needLicRoot["activation"]["stat"]="activated";
					needLicRoot["activation"]["uid"]=CnT->deviceUID;
					//forceLogoutRoot["forceLogout"]["group"]="";
					SendSystemFromCnodaToUI(FastWriteJSON(needLicRoot));
					//SendSystemFromCnodaToUI(FastWriteJSON(forceLogoutRoot));
					return NO_ERROR;
				}
		eErrorTp FifoEngineT::activationMessageNeedLic(){
			Json::Value needLicRoot;
			Json::Value forceLogoutRoot;
			needLicRoot["activation"]["stat"]="needLicense";
			needLicRoot["activation"]["uid"]=CnT->deviceUID;
			//forceLogoutRoot["forceLogout"]["group"]="";
			SendSystemFromCnodaToUI(FastWriteJSON(needLicRoot));
			//SendSystemFromCnodaToUI(FastWriteJSON(forceLogoutRoot));
			return NO_ERROR;
		}
		//Отправка системного эвента cnoda из Cnoda в WEB
		eErrorTp FifoEngineT::SendSystemFromCnodaToUI(string dMessageJson)
		{
			return SendDataFromCnodaToUI_base(dMessageJson,JSON_PACK_TYPE_SET_SYSTEM,JSON_PACK_VERSION,"","cnoda");
		}
		//Отправка системного эвента cnoda из Cnoda в WEB
		eErrorTp FifoEngineT::SendSystemFromCnodaToUI(string dMessageJson,string user)
		{
			return SendDataFromCnodaToUI_base(dMessageJson,JSON_PACK_TYPE_SET_SYSTEM,JSON_PACK_VERSION,user,"cnoda");
		}
		//Отправка эвента cnoda из Cnoda в WEB
		eErrorTp FifoEngineT::SendEventFromCnodaToUI(string dMessageJson)
		{

			//string cnodahead=string_format("\"t\":[%d,%d],\"d\":{\"action\":\"cnoda\",\"cnoda\":\"%s\"}",JSON_PACK_TYPE_SEND_EVENT,JSON_PACK_VERSION,dMessageJson.c_str());
			//string jsons=string_format("{\"t\":[%d,%d],\"d\":{%s}}",JSON_PACK_TYPE_TO_UI,JSON_PACK_VERSION,cnodahead.c_str());

			//GPRINT(MEDIUM_LEVEL,"SendMessageFromCnodaToUI\n");
			//return SendSharedFifoMessage(srvmSendToLH,"wead", (u8*)jsons.c_str(),jsons.size());
			return SendDataFromCnodaToUI_base(dMessageJson,JSON_PACK_TYPE_SEND_EVENT,JSON_PACK_VERSION,"","cnoda");
		}

		eErrorTp FifoEngineT::SendEventFromCnodaToUI(string dMessageJson,string user)
		{

			//string cnodahead=string_format("\"t\":[%d,%d],\"d\":{\"action\":\"cnoda\",\"cnoda\":\"%s\"},\"sid\":\"%s\"",JSON_PACK_TYPE_SEND_EVENT,JSON_PACK_VERSION,dMessageJson.c_str(),user.c_str());
			//string jsons=string_format("{\"t\":[%d,%d],\"d\":{%s}}",JSON_PACK_TYPE_TO_UI,JSON_PACK_VERSION,cnodahead.c_str());

			//GPRINT(MEDIUM_LEVEL,"SendMessageFromCnodaToUI\n");
			//return SendSharedFifoMessage(srvmSendToLH,"wead", (u8*)jsons.c_str(),jsons.size());
			return SendDataFromCnodaToUI_base(dMessageJson,JSON_PACK_TYPE_SEND_EVENT,JSON_PACK_VERSION,user,"cnoda");
		}
		eErrorTp FifoEngineT::SendWebEventFromCnodaToUI(string req,string iface,Json::Value & result)
		{
			Json::Value root;
			root["req"]=req;
			root["iface"]=iface;
			root["result"]=result;

			return SendDataFromCnodaToUI_base(FastWriteJSON(root),JSON_PACK_TYPE_SEND_EVENT,JSON_PACK_VERSION,"","webevent");
		}

		eErrorTp FifoEngineT::SendUserWebEventFromCnodaToUI(string userEvent,Json::Value & args)//WEB приняв пакет сгенерит событие userEvent с параметрами args
		{
			string dMessageJson=sf("{\"userEvent\":\"%s\",\"args\":%s}",userEvent.c_str(),FastWriteJSON(args).c_str());
			return SendDataFromCnodaToUI_base(dMessageJson,JSON_PACK_TYPE_SEND_EVENT,JSON_PACK_VERSION,"","userWebEvent");
		}

		eErrorTp FifoEngineT::SendUserWebEventFromCnodaToUI(string userEvent,string ssid,Json::Value & args)//WEB приняв пакет сгенерит событие userEvent с параметрами args
		{
			string dMessageJson=sf("{\"userEvent\":\"%s\",\"args\":%s}",userEvent.c_str(),FastWriteJSON(args).c_str());
			return SendDataFromCnodaToUI_base(dMessageJson,JSON_PACK_TYPE_SEND_EVENT,JSON_PACK_VERSION,ssid,"userWebEvent");
		}

		eErrorTp FifoEngineT::SendCnodaReadyToJNODA()
		{

					string cnodahead=string_format("\"t\":[%d,%d],\"d\":{\"action\":\"cnodaready\",\"cnodaready\":\"\"}",JSON_PACK_TYPE_FROM_CNODA,JSON_PACK_VERSION);
					string jsons=string_format("{\"t\":[%d,%d],\"d\":{%s}}",JSON_PACK_TYPE_TO_JNODA,JSON_PACK_VERSION,cnodahead.c_str());

					GPRINT(MEDIUM_LEVEL,"SendReadyToJNODA\n");
					return SendSharedFifoMessage(srvmSendToLH,"wead", (u8*)jsons.c_str(),jsons.size());
		}

		eErrorTp FifoEngineT::SendReadyToJNODA()
		{

			string cnodahead=string_format("\"t\":[%d,%d],\"d\":{\"action\":\"ready\",\"ready\":\"\"}",JSON_PACK_TYPE_FROM_CNODA,JSON_PACK_VERSION);
			string jsons=string_format("{\"t\":[%d,%d],\"d\":{%s}}",JSON_PACK_TYPE_TO_JNODA,JSON_PACK_VERSION,cnodahead.c_str());

			GPRINT(MEDIUM_LEVEL,"SendReadyToJNODA\n");
			return SendSharedFifoMessage(srvmSendToLH,"wead", (u8*)jsons.c_str(),jsons.size());
		}

		eErrorTp FifoEngineT::SendWaitToJNODA()
		{

			string cnodahead=string_format("\"t\":[%d,%d],\"d\":{\"action\":\"busy\",\"busy\":\"\"}",JSON_PACK_TYPE_FROM_CNODA,JSON_PACK_VERSION);
			string jsons=string_format("{\"t\":[%d,%d],\"d\":{%s}}",JSON_PACK_TYPE_TO_JNODA,JSON_PACK_VERSION,cnodahead.c_str());

			GPRINT(MEDIUM_LEVEL,"SendWaitToJNODA\n");
			return SendSharedFifoMessage(srvmSendToLH,"wead", (u8*)jsons.c_str(),jsons.size());
		}

		eErrorTp FifoEngineT::SendEventToJNODA( string dMessageJson)
		{

			string cnodahead=string_format("\"t\":[%d,%d],\"d\":{\"action\":\"event\",\"event\":%s}",JSON_PACK_TYPE_FROM_CNODA,JSON_PACK_VERSION,dMessageJson.c_str());
			string jsons=string_format("{\"t\":[%d,%d],\"d\":{%s}}",JSON_PACK_TYPE_TO_JNODA,JSON_PACK_VERSION,cnodahead.c_str());

			GPRINT(HARD_LEVEL,"SendEventToJNODA %s\n",jsons.c_str());
			return SendSharedFifoMessage(srvmSendToLH,"wead", (u8*)jsons.c_str(),jsons.size());
		}

		eErrorTp FifoEngineT::SendRestartClusterToJNODA(void){
			string jsons=string_format("{\"t\":[%d,%d],\"d\":{\"type\":\"server_reboot\",\"arg\":[\"jssrv\"]}}",JSON_PACK_TYPE_WD_RESTART,JSON_PACK_VERSION);
			return SendSharedFifoMessage(srvmSendToLH,"wead", (u8*)jsons.c_str(),jsons.size());
		}
		eErrorTp FifoEngineT::SendRestartSettingToJNODA(string setting_name)
		{
			string jsons=string_format("{\"t\":[%d,%d],\"d\":{\"type\":\"restart_setting\",\"arg\":[\"%s\"]}}",JSON_PACK_TYPE_WD_RESTART,JSON_PACK_VERSION,setting_name.c_str());
			return SendSharedFifoMessage(srvmSendToLH,"wead", (u8*)jsons.c_str(),jsons.size());
		}
		eErrorTp FifoEngineT::SendRestartNecron(void)
		{
			string jsons=string_format("{\"t\":[%d,%d],\"d\":{\"type\":\"server_reboot\",\"arg\":[\"allsrv\"]}}",JSON_PACK_TYPE_WD_RESTART,JSON_PACK_VERSION);
			return SendSharedFifoMessage(srvmSendToLH,"wead", (u8*)jsons.c_str(),jsons.size());
		}
		//Отправка сообщения Cnoda в Jnoda
		eErrorTp FifoEngineT::SetCnodaValue(string name,u32 val)
		{
			string tojn=string_format("{\"cnvalue\":{\"%s\":\"%d\"}}",name.c_str(),val);

			SendEventToJNODA(tojn.c_str());

			return NO_ERROR;
		}

		//Отправка сообщения Cnoda в Jnoda
		eErrorTp FifoEngineT::SetCnodaValue(string name,string val)
		{
			string tojn=string_format("{\"cnvalue\":{\"%s\":\"%s\"}}",name.c_str(),val.c_str());

			SendEventToJNODA(tojn.c_str());

			return NO_ERROR;
		}
		//Отправка сообщения внутри Cnoda
		eErrorTp FifoEngineT::SendActionLocal(u8 Msg,string Dest,string action,Json::Value & data)
		{
			return SendActionLocal(Msg,Dest,action,"none",data);
		}
		//Отправка сообщения внутри Cnoda
		eErrorTp FifoEngineT::SendActionLocal(u8 Msg,string Dest,string action,string comp,Json::Value & data)
		{
			//{"t":[2,1],

			string dataStr=FastWriteJSON(data);
			string resultDataStr;
			if (comp=="gzip"){
				 string compRes=gzip::compress(dataStr.c_str(),dataStr.size(),Z_DEFAULT_COMPRESSION);
				 resultDataStr="\"";
				 resultDataStr+=base64_encode((unsigned char const*)compRes.c_str(),compRes.size());
				resultDataStr+="\"";
			}
			else
				resultDataStr=dataStr;

			string data_s=string_format("{\"t\":[%d,%d],\"d\":{\"action\":\"%s\",\"%s\":%s,\"comp\":\"%s\"}}",JSON_PACK_TYPE_FROM_CNODA,JSON_PACK_VERSION,action.c_str(),action.c_str(),resultDataStr.c_str(),comp.c_str());
			GPRINT(MEDIUM_LEVEL,"SendActionLocal size comp %s orig/comp %d/%d\n",comp.c_str(),dataStr.size(),resultDataStr.size());
			SendSharedFifoMessage(Msg,Dest,data_s);
			return NO_ERROR;
		}

		eErrorTp FifoEngineT::SendMQ(char * topic,string & data,bool comp){
			Json::Value send;
			send["mqProto"]="mqtt";
			send["mqMQTT_topics"][0]=topic;

			if (comp==false){
				send["mqMQTT_send"]=data;
				SendActionLocal(srvmJSON_ACTION_FORMAT,"mqco","mq_send",send);
				GPRINT(MEDIUM_LEVEL,"SendMQ %s to %s\n",data.c_str(),topic);
			}
			else{
				string compRes=gzip::compress(data.c_str(),data.size(),Z_DEFAULT_COMPRESSION);
				send["mqMQTT_send"]=sf("{\"comp\":\"base64gzip\",\"data\":\"%s\"}",(char*)base64_encode((unsigned char const*)compRes.c_str(),compRes.size()).c_str());
				GPRINT(MEDIUM_LEVEL,"SendMQ compressed pack size orig/comp %d/%d\n",data.size(),compRes.size());
				SendActionLocal(srvmJSON_ACTION_FORMAT,"mqco","mq_send",send);
			}

			return NO_ERROR;
		}

		eErrorTp FifoEngineT::SendMQ(char * topic,Json::Value & jdata,bool comp=false,u8 prec=5){
			Json::Value send;
			string data=FastWriteJSON(jdata,prec);
			return SendMQ(topic,data,comp);
		}
		shared_ptr<FifoEngineT> shared_fifo;
