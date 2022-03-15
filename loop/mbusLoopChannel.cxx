/*
 * mbusLoopChannel.cxx
 *
 *  Created on: 7 апр. 2021 г.
 *      Author: root
 */

#ifdef _MODBUSCPP


#include "mbus_loop.h"

eErrorTp mbusLoopGroupEndPoint::config(Json::Value & cfg){

	devName=cfg["devName"].asString();
	devAddr=cfg["devAddr"].asUInt();
	channel=cfg["channel"].asString();
	groupName=cfg["groupName"].asString();
	startAddr=cfg["startAddr"].asUInt();
	countRegs=cfg["countRegs"].asUInt();
	emu=cfg["emu"].asUInt();
	dnk=cfg["dnk"];
	if(emu){
		emuBuf.create(countRegs*2);
		for (const auto & pathValue: dnk.getMemberNames()){
				string emuSetPath="emu."+pathValue;
				dnkReadEmu[emuSetPath]=dnk[pathValue];
		}
	}
	if (cfg.isMember("interval")){
		interval=cfg["interval"].asUInt();
		timer.set(interval,LOOPTIMER_TIMER_MODE);
	}

	if (cfg.isMember("event")){
		eventCfg=cfg["event"];
		if (eventCfg.isMember("sendDnkResult")){
			if (eventCfg["sendDnkResult"].asUInt()){
				sendDnkResult=1;
			}
			else
				sendDnkResult=0;
		}
	}

	startAddr=cfg["startAddr"].asUInt();
	mode=cfg["mode"].asString();


	access=cfg["access"].asString();
	for (u8 n=0;n<cfg["listener"].size();n++){
		listener.emplace_back(cfg["listener"][n].asString());
	}
	print->GPRINT(NORMAL_LEVEL,"Config %s.%s.%s.%s\n",devName.c_str(),groupName.c_str(),mode.c_str(),access.c_str());
	return NO_ERROR;
}

mbusLoopGroupEndPoint::mbusLoopGroupEndPoint(modbuscpp_client * mbusLine,ThreadT * gp){
	mLine=mbusLine;
	print=gp;
}

eErrorTp mbusLoopGroupEndPoint::read(){
	string id="";
	if (interval!=0){
		if (timer.alarm()){
			if (emu){
				if (needInit){
					Json::Value pack;
					string id="";
					buffer buf(emuBuf.size());
					//printJSON("dnkReadEmu",dnkReadEmu);
					encodePackDataDNK(dnkReadEmu,buf,pack,id);
					needInit=false;
				}
				readEmu();
			}
			readSeq(id);
		}
	}
	return NO_ERROR;
}
eErrorTp mbusLoopGroupEndPoint::write(){
	string id="";
	if (interval!=0){
		if (timer.alarm()){
			writeSeq(id);
		}
	}
	return NO_ERROR;
}
eErrorTp mbusLoopGroupEndPoint::readOdm(Json::Value & event){
	if ((event["channel"]==channel)&&
					(event["dev"]==devName)&&
					(event["group"]==groupName)){
		print->GPRINT(MEDIUM_LEVEL,"Read ODM %s.%s.%s\n",channel.c_str(),devName.c_str(),groupName.c_str());
		string id=event["id"].asString();

		if (emu){
			if (needInit){
				Json::Value pack;
				string id="";
				buffer buf(emuBuf.size());
				//printJSON("dnkReadEmu",dnkReadEmu);
				encodePackDataDNK(dnkReadEmu,buf,pack,id);
				needInit=false;
			}
			readEmu();
		}
		readSeq(id);
	}

	return NO_ERROR;
}
eErrorTp mbusLoopGroupEndPoint::writeOdm(Json::Value & event){
	if ((event["channel"]==channel)&&
			(event["dev"]==devName)&&
			(event["group"]==groupName)){
		print->GPRINT(MEDIUM_LEVEL,"Write ODM %s.%s.%s\n",channel.c_str(),devName.c_str(),groupName.c_str());
		string id=event["id"].asString();
		writeSeq(id);

	}
	else{
		print->GPRINT(MEDIUM_LEVEL,"Not my Write ODM channel[%s] devName[%s] groupName[%s]\n",channel.c_str(),devName.c_str(),groupName.c_str());
	}
	return NO_ERROR;
}

Json::Value mbusLoopGroupEndPoint::genEvent(string resp,string mode,string & id){
	Json::Value pack;
	pack["type"]="event";
	pack["channel"]=channel;
	pack["dev"]=devName;
	pack["group"]=groupName;
	pack["mode"]=mode;
	pack["resp"]=resp;
	pack["id"]=id;

	return pack;
}

Json::Value  mbusLoopGroupEndPoint::genReadEventDNK(string msg,string & id,Json::Value resultDnk){
	Json::Value pack=genEvent(msg,"read",id);
	pack["dnk"]=resultDnk;
	return pack;
}

Json::Value  mbusLoopGroupEndPoint::genReadEvent(string msg,string & id){
	Json::Value pack=genEvent(msg,"read",id);
	return pack;
}

Json::Value  mbusLoopGroupEndPoint::genWriteEvent(string msg,string & id){
	Json::Value pack=genEvent(msg,"write",id);
	return pack;
}
eErrorTp mbusLoopGroupEndPoint::sendPackToListeners(Json::Value & pack){
	for (u32 n=0;n<listener.size();n++){
		print->SendSharedFifoMessage(srvmMODBUS_MESSAGE,listener[n], pack);
	}
	return NO_ERROR;
}
eErrorTp mbusLoopGroupEndPoint::encodePackDataDNK(Json::Value & map,buffer & buf, Json::Value & pack,string & id){
	Json::Value json_result;

	for (const auto& key : map.getMemberNames()) {
			u32 offset=map[key][0].asUInt();
			u32 size=map[key][1].asUInt();
			string algo=map[key][2].asString();
			string path=key;
			u32 dstSize=(size/8)+10;//10 про запас
			buffer dst(dstSize);
			bitCopy(dst.p(),0,size,buf.p(),offset);
			if (CnT->dnk_var->MC_ParsePackData_ConvValue(dst.p(),size,algo,path,json_result)==ERROR){
				return ERROR;
			}

	}

	//printf("sendDnkResult %d groupName %s devName %s channel %s\n",sendDnkResult,groupName.c_str(),devName.c_str(),channel.c_str());
	if (sendDnkResult){
		pack=genReadEventDNK("ok",id,json_result);
		//printJSON("json_result",json_result);
		//printJSON("map",map);
	}
	else{
		pack=genReadEvent("ok",id);
	}
	return NO_ERROR;
};

eErrorTp mbusLoopGroupEndPoint::codePackDataDNK(Json::Value & map,buffer & buf,Json::Value & pack){

	for (const auto& key : map.getMemberNames()) {
			//printf("mname %s\n",key.c_str());
			u32 offset=map[key][0].asUInt();
			u32 size=map[key][1].asUInt();
			string algo=map[key][2].asString();
			string path=key;

			if (((offset+size)/8)>buf.size()){
				print->GPRINT(NORMAL_LEVEL,"Error: override buffer (offset+size)/8>buf.size() %d>%d\n",(offset+size)/8,buf.size());
				continue;
			}
			u32 dstSize=(size/8)+1;//1 про запас

			buffer convBuf(dstSize);
			//
			print->GPRINT(MEDIUM_LEVEL,"GetValue from %s\n",(char*)path.c_str());
			Json::Value jsValue=CnT->dnk_var->GetValue((char*)path.c_str());
			//if (emu){
				//string emuPath="emu."+path;
				//CnT->dnk_var->Dump("emu",false);
				//CnT->dnk_var->SetValue(emuPath,jsValue);
				//CnT->dnk_var->Dump("emu",false);
			//}

			if (CnT->dnk_var->MC_CreatePackData_ConvValue(size,algo,jsValue,convBuf.p())==ERROR)
				return ERROR;

			bitCopy(buf.p(),offset,size,convBuf.p(),0);

	}

	return NO_ERROR;
};

eErrorTp mbusLoopGroupEndPoint::writeSeq(string & id){
	print->GPRINT(MEDIUM_LEVEL,"write dev [%d] mode [%s] addr [%d] count [%d]\n",devAddr,mode.c_str(),startAddr,countRegs);
	buffer buf(countRegs*2);
	eErrorTp err=ERROR;
	Json::Value pack;
	codePackDataDNK(dnk,buf,pack);////тип регистра oneCoil(5) oneInput(6) coil(15) input(16)
	if (!emu){
		if (mode=="input")
			err=mLine->WriteMultipleReg(devAddr,startAddr,countRegs,buf);
		else
		if (mode=="oneInput")
			err=mLine->WriteSingleReg(devAddr,startAddr,buf);
		else
		if (mode=="coil")
			err=mLine->WriteMultipleCoil(devAddr,startAddr,countRegs,buf);
		else
		if (mode=="oneCoil")
			err=mLine->WriteSingleCoil(devAddr,startAddr,buf);

		if (err==NO_ERROR){
			pack=genWriteEvent("ok",id);
		}else{
			pack=genWriteEvent("err",id);
		}

	}
	else{
		if (mode=="input")
			memcpy(emuBuf.p(),buf.p(),countRegs*2);
		else
		if (mode=="oneInput")
			memcpy(emuBuf.p(),buf.p(),2);
		else
		if (mode=="coil")
			memcpy(emuBuf.p(),buf.p(),countRegs);
		else
		if (mode=="oneCoil")
			emuBuf.p()[0]=buf.p()[0];
		pack=genWriteEvent("ok",id);
		//printhex(emuBuf.p(),emuBuf.size(),16);
	}
	sendPackToListeners(pack);
	return NO_ERROR;
}


eErrorTp mbusLoopGroupEndPoint::readSeq(string & id){
	print->GPRINT(MEDIUM_LEVEL,"read dev [%d] mode [%s] addr [%d] count [%d]\n",devAddr,mode.c_str(),startAddr,countRegs);
	buffer buf;
	eErrorTp err=ERROR;
	Json::Value pack;
	if (!emu){
		if (mode=="input")
			err=mLine->ReadInputReg(devAddr,startAddr,countRegs,buf,try_count);
		else
		if (mode=="holding")
			err=mLine->ReadHoldingReg(devAddr,startAddr,countRegs,buf,try_count);
		else
		if (mode=="discrete")
			err=mLine->ReadDiscreteInput(devAddr,startAddr,countRegs,buf,try_count);
		else
		if (mode=="coil")
			err=mLine->ReadCoil(devAddr,startAddr,countRegs,buf,try_count);
	}
	else{
		err=NO_ERROR;
		buf.create(countRegs*2);
		if (mode=="input")
			memcpy(buf.p(),emuBuf.p(),countRegs*2);
		else
		if (mode=="holding")
			memcpy(buf.p(),emuBuf.p(),countRegs*2);
		else
		if (mode=="discrete")
			memcpy(buf.p(),emuBuf.p(),countRegs);
		else
		if (mode=="coil")
			memcpy(buf.p(),emuBuf.p(),countRegs);
	}

	if (err==NO_ERROR){
		encodePackDataDNK(dnk,buf,pack,id);
	}
	else{
		//zr();
		pack=genReadEvent("err",id);
	}
	//printhex(buf.p(),buf.size(),16);
	//printJSON("readMB",pack);
	sendPackToListeners(pack);
	return NO_ERROR;
}

eErrorTp mbusLoopGroup::config(Json::Value & groupCfg,Json::Value & listener,string groupName,string devName,u32 devAddr,string channel){
	Json::Value epCfg;

	//print->GPRINT(NORMAL_LEVEL,"Config device %s with group %s\n",devName.c_str(),groupName.c_str());
	epCfg["devName"]=devName;
	epCfg["devAddr"]=devAddr;
	epCfg["channel"]=channel;
	epCfg["listener"]=listener;
	epCfg["groupName"]=groupName;
	u8 idx=0;
	if (groupCfg.isMember("emu")){
		epCfg["emu"]=groupCfg["emu"];

	}
	else{
		epCfg["emu"]=0;
	}
	emu=epCfg["emu"].asUInt();
	//printJSON("groupCfg",groupCfg);
	if (groupCfg.isMember("read")){
		epCfg["startAddr"]=groupCfg["addr"];
		epCfg["countRegs"]=groupCfg["cnt"];
		epCfg["interval"]=groupCfg["read"]["interval"];
		epCfg["mode"]=groupCfg["read"]["mode"];

		if ((groupCfg["read"].isMember("dnk")==false)&&
				(groupCfg.isMember("write"))&&
				(groupCfg["write"].isMember("dnk"))){
			epCfg["dnk"]=groupCfg["write"]["dnk"];

		}
		else
			epCfg["dnk"]=groupCfg["read"]["dnk"];

		if (groupCfg["read"].isMember("event"))
				epCfg["event"]=groupCfg["read"]["event"];
		epCfg["access"]="read";
		ep.emplace_back(make_shared<mbusLoopGroupEndPoint>(mLine,print));
		ep[idx]->config(epCfg);
		idx++;
	}
	if (groupCfg.isMember("write")){
		epCfg["startAddr"]=groupCfg["addr"];
		epCfg["countRegs"]=groupCfg["cnt"];
		epCfg["interval"]=groupCfg["write"]["interval"];
		epCfg["mode"]=groupCfg["write"]["mode"];

		if ((groupCfg["write"].isMember("dnk")==false)&&
						(groupCfg.isMember("read"))&&
						(groupCfg["read"].isMember("dnk"))){
					epCfg["dnk"]=groupCfg["read"]["dnk"];
		}
		else
			epCfg["dnk"]=groupCfg["write"]["dnk"];

		if (groupCfg["write"].isMember("event"))
				epCfg["event"]=groupCfg["write"]["event"];
		epCfg["access"]="write";
		ep.emplace_back(make_shared<mbusLoopGroupEndPoint>(mLine,print));
		ep[idx]->config(epCfg);
	}


	return NO_ERROR;
}

mbusLoopGroup::mbusLoopGroup(modbuscpp_client * mbusLine, ThreadT * gp){
		mLine=mbusLine;
		print=gp;
	}

eErrorTp mbusLoopGroup::readWriteByInterval(){
	for (auto & epItem:ep){
		if (epItem->access=="read"){
			epItem->read();
		}
		else{
			if (epItem->access=="write"){
				epItem->write();
			}
		}
	}
	return NO_ERROR;
}
eErrorTp mbusLoopGroup::readWriteOdm(Json::Value & event){
	//zr();
	for (auto & epItem:ep){
		if ((epItem->access=="write")&&(event["mode"]=="write")){
			print->GPRINT(MEDIUM_LEVEL,"mbusLoopGroup::readWriteOdm write mode\n");
				epItem->writeOdm(event);
		}
		else
		if ((epItem->access=="read")&&(event["mode"]=="read")){
			//zr();
			print->GPRINT(MEDIUM_LEVEL,"mbusLoopGroup::readWriteOdm read mode\n");
				epItem->readOdm(event);
		}
	}
	return NO_ERROR;
}

eErrorTp mbusLoopDevice::config(Json::Value & devCfg,string channel){
	u32 z=0;
	print->GPRINT(NORMAL_LEVEL,"Config channel %s\n",channel.c_str());
	for (const auto & devName:devCfg.getMemberNames()){
		for (const auto & grpName:devCfg[devName]["data"].getMemberNames()){
			device.emplace_back(make_shared<mbusLoopGroup>(mLine,print));
			device[z]->config(devCfg[devName]["data"][grpName],
					devCfg[devName]["listener"],
					grpName,
					devName,
					devCfg[devName]["addr"].asUInt(),
					channel);
			z++;
		}
	}

	return NO_ERROR;
}

mbusLoopDevice::mbusLoopDevice(modbuscpp_client * mbusLine, ThreadT * gp){
	mLine=mbusLine;
	print=gp;
}

eErrorTp mbusLoopDevice::readWriteByInterval(){
	for (auto & deviceItem:device){
		deviceItem->readWriteByInterval();
	}
	return NO_ERROR;
}
eErrorTp mbusLoopDevice::readWriteOdm(Json::Value & event){
	for (auto & deviceItem:device){
		print->GPRINT(MEDIUM_LEVEL,"mbusLoopDevice::readWriteOdm\n");
		deviceItem->readWriteOdm(event);
	}
	return NO_ERROR;
}

eErrorTp mbusLoopChannel::config(Json::Value cfg){
		u32 z=0;

		for (const auto & chName:cfg.getMemberNames()){
			mbusLine.emplace_back(make_shared<modbuscpp_client>(NORMAL_LEVEL));

			Json::Value conf=cfg[chName]["cfg"];
			u32 debug=0;
			if (cfg[chName]["cfg"].isMember("debug")){
				debug=cfg[chName]["cfg"]["debug"].asUInt();
			}
			string mode=cfg[chName]["cfg"]["mode"].asString();
			print->GPRINT(NORMAL_LEVEL,"Config map %s \n",FastWriteJSON(cfg).c_str());
			if (mode=="serial"){

				string tty=cfg[chName]["cfg"]["tty"].asString();
				string speed=cfg[chName]["cfg"]["speed"].asString();
				print->GPRINT(NORMAL_LEVEL,"Config serial tty %s speed %s\n",tty.c_str(),speed.c_str());
				print->GPRINT(NORMAL_LEVEL,"Try connect\n");
				if (mbusLine[z]->InitMaster(tty,stol(speed),'N',8,1)==ERROR){
					mbusLine.clear();
					print->GPRINT(NORMAL_LEVEL,"Connect error\n");
					return ERROR;
				}

				if (cfg[chName]["cfg"].isMember("readDelay_mS")){
					u32 readDelay_mS=cfg[chName]["cfg"]["readDelay_mS"].asUInt();
					mbusLine[z]->SetSwDelay(readDelay_mS);
				}

				u8 rtsMode=MODBUS_RTU_RTS_NONE;
				if (cfg[chName]["cfg"].isMember("rts")){
					string rts=cfg[chName]["cfg"]["rts"].asString();
					if (rts=="NONE")
						rtsMode=MODBUS_RTU_RTS_NONE;
					if (rts=="UP")
						rtsMode=MODBUS_RTU_RTS_UP;
					if (rts=="DOWN")
						rtsMode=MODBUS_RTU_RTS_DOWN;
				}
				mbusLine[z]->SetupLevelRTS(rtsMode);
				if (cfg[chName]["cfg"].isMember("respTimeout_mS")){
					u32 respTimeout_mS=cfg[chName]["cfg"]["respTimeout_mS"].asUInt();
					u32 sec=respTimeout_mS/1000;
					u32 usec=(respTimeout_mS-(sec*1000))*1000;
					mbusLine[z]->SetTimeout(sec,usec);
				}
				if (cfg[chName]["cfg"].isMember("swRts")){
					u32 gpio=cfg[chName]["cfg"]["swRts"]["gpio"].asUInt();
					u32 delay_uS=cfg[chName]["cfg"]["swRts"]["delay_uS"].asUInt();
					mbusLine[z]->SetupSoftwareRTS(gpio,delay_uS);
				}
				print->GPRINT(NORMAL_LEVEL,"Connect success\n");
			}

			if (mode=="net"){
				string ip=cfg[chName]["cfg"]["ip"].asString();
				string port=cfg[chName]["cfg"]["port"].asString();
				print->GPRINT(NORMAL_LEVEL,"Config net ip %s port %s\n",ip.c_str(),port.c_str());
				print->GPRINT(NORMAL_LEVEL,"Try connect\n");
				if (mbusLine[z]->InitMaster(ip,stol(port))==ERROR){
					print->GPRINT(NORMAL_LEVEL,"Connect error\n");
					mbusLine.clear();
					return ERROR;
				}
				print->GPRINT(NORMAL_LEVEL,"Connect success\n");
			}

			if (mode=="emu"){
				print->GPRINT(NORMAL_LEVEL,"Emu connect success\n");
			}

			if (debug)
				mbusLine[z]->EnableDebug();

			chann.emplace_back(make_shared<mbusLoopDevice>(mbusLine[z].get(),print));
			chann[z]->config(cfg[chName]["dev"],chName);
			z++;
			configured=true;

		}
		return NO_ERROR;
};

eErrorTp mbusLoopChannel::readWriteByInterval(){
	if (configured){
		for (auto & channItem:chann){
			channItem->readWriteByInterval();
		}
	}
	return NO_ERROR;
}
eErrorTp mbusLoopChannel::readWriteOdm(Json::Value & event){
	//zr();
	if (configured){
		//zr();
		for (auto & channItem:chann){
				print->GPRINT(MEDIUM_LEVEL,"mbusLoopChannel::readWriteOdm\n");
				channItem->readWriteOdm(event);
			}
		}
	//zr();
	return NO_ERROR;
}

#endif

