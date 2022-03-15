/*
 * modbusInterThread.h
 *
 *  Created on: 8 апр. 2021 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_PROTO_MODBUSINTERTHREAD_H_
#define SRVIOT_SRC_ENGINE_PROTO_MODBUSINTERTHREAD_H_

#include "engine/thread.h"

class mbusInterThread{
	private:
	ThreadT * pr;
	string fifoName;
	u32 id=0;
	bool inited=false;
	void cleanFifo(){
		buffer buf;
		u8  MsgType;
		while (pr->GetUcastFifoMessage(buf,MsgType, fifoName)==NO_ERROR);
	}
	public:
	eErrorTp init(string fifoName,ThreadT * parent,u32 initId){//fifoName - имя второго канала потока для получения сообщений, parent - this, initId - любое значение
		pr=parent;
		this->fifoName=fifoName;
		id=initId;
		inited=true;
		return NO_ERROR;
	}


	eErrorTp sendMbusASync(char * channel,char * dev,char * group){
			if (!inited){
				pr->GPRINT(NORMAL_LEVEL,"Error: sendMbusASync mbusInterThread not init\n");
				return ERROR;
			}
			Json::Value pack;
			pack["type"]="event";
			pack["channel"]=channel;
			pack["dev"]=dev;
			pack["group"]=group;
			pack["mode"]="write";
			pack["id"]=sf("%u",id);
			id++;

			pr->SendSharedFifoMessage(srvmMODBUS_MESSAGE,"mblo", pack);
			return NO_ERROR;
	}

	eErrorTp sendMbusSync(char * channel,char * dev,char * group,bool tryAgain=false){
			if (!inited){
				pr->GPRINT(NORMAL_LEVEL,"Error: sendMbusSync mbusInterThread not init\n");
				return ERROR;
			}
			Json::Value pack;
			pack["type"]="event";
			pack["channel"]=channel;
			pack["dev"]=dev;
			pack["group"]=group;
			pack["mode"]="write";
			pack["id"]=sf("%u",id);
			id++;
			u8  MsgType;
			buffer buf;

			cleanFifo();
			pr->SendSharedFifoMessage(srvmMODBUS_MESSAGE,"mblo", pack);

			RTC_Timer tout(5000);
			eErrorTp err=ERROR;
			while (1){
				if (pr->GetUcastFifoMessage(buf,MsgType, fifoName)==NO_ERROR){
					//printf("sendMbusSync wait %d resp %s\n",id-1,buf.p());
					if (buf.size()==0){
						continue;
					}
					Json::Value parsed=parseJSON((char*)buf.p());
					if ((parsed["id"]==pack["id"])&&
						(parsed["dev"]==dev)&&
						(parsed["mode"]=="write")&&
						(parsed["group"]==group)&&
						(parsed["channel"]==channel)){
						if (parsed["resp"]=="ok"){
							err=NO_ERROR;
							break;
						}
						else{
							if (tryAgain){
								printf("Error write, repeat pack %s\n",FastWriteJSON(pack).c_str());
								cleanFifo();
								pack["id"]=sf("%u",id);
								pr->SendSharedFifoMessage(srvmMODBUS_MESSAGE,"mblo", pack);
								id++;
							}
							else{
								break;
							}
						}
					}

				}

				if (tout.alarm())
					break;

				mdelay(1);
			}
			return err;
		}
	eErrorTp readMbusASync(char * channel,char * dev,char * group){
			if (!inited){
				pr->GPRINT(NORMAL_LEVEL,"Error: readMbusASync mbusInterThread not init\n");
				return ERROR;
			}
			Json::Value pack;
			pack["type"]="event";
			pack["channel"]=channel;
			pack["dev"]=dev;
			pack["group"]=group;
			pack["mode"]="read";
			pack["id"]=sf("%u",id);
			id++;
			pr->SendSharedFifoMessage(srvmMODBUS_MESSAGE,"mblo", pack);
			return NO_ERROR;
	}
	eErrorTp readMbusSync(char * channel,char * dev,char * group,Json::Value & dnkResult,bool tryAgain=false){
			if (!inited){
				pr->GPRINT(NORMAL_LEVEL,"Error: readMbusSync mbusInterThread not init\n");
				return ERROR;
			}
			Json::Value pack;
			pack["type"]="event";
			pack["channel"]=channel;
			pack["dev"]=dev;
			pack["group"]=group;
			pack["mode"]="read";
			pack["id"]=sf("%u",id);
			id++;
			//zr();
			pr->SendSharedFifoMessage(srvmMODBUS_MESSAGE,"mblo", pack);
			u8  MsgType;
			buffer buf;
			RTC_Timer tout(5000);
			eErrorTp err=ERROR;
			while (1){
				//zr();
				if (pr->GetUcastFifoMessage(buf,MsgType, fifoName)==NO_ERROR){
					//printhex(buf.p(),buf.size(),16);
					//printf("readMbusSync [%s]\n",(char*)buf.p());
					if (buf.size()==0){
						continue;
					}

					Json::Value parsed=parseJSON((char*)buf.p());
					if ((parsed["id"]==pack["id"])&&
						(parsed["dev"]==dev)&&
						(parsed["mode"]=="read")&&
						(parsed["group"]==group)&&
						(parsed["channel"]==channel)){
						if (parsed["resp"]=="ok"){
							//printJSON("parsed",parsed);
							if (parsed.isMember("dnk"))
								dnkResult=parsed["dnk"];

							err=NO_ERROR;
							break;
						}
						else{
							if (tryAgain){
								printf("Error read, repeat pack %s\n",FastWriteJSON(pack).c_str());
								cleanFifo();
								pack["id"]=sf("%u",id);
								pr->SendSharedFifoMessage(srvmMODBUS_MESSAGE,"mblo", pack);
								id++;
							}
							else
								break;
						}

					}
				}

				if (tout.alarm())
					break;

				mdelay(1);
			}
			return err;
	}
};



#endif /* SRVIOT_SRC_ENGINE_PROTO_MODBUSINTERTHREAD_H_ */
