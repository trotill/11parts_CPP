#include "engine/minisw/minisw.h"
#include "engine/lib/11p_string.h"

eErrorTp MiniSW_Run(int argc,char *argv[],string SWname)
{

	//printf("run %s\n",SWname.c_str());
	if (SWname.compare(USB_RESET_SW)==0)
	{
		usb_reset_main(argc,argv);
	}

	if (SWname.compare(IMAGE_CHECK)==0)
	{
		imcheck_main(argc,argv);
	}

	if (SWname.compare(CNODA_PROCESS_NAME)==0)
	{
		EventSender(argc,argv,SWname);
	}

	if (SWname.compare(WDT_PROCESS_NAME)==0)
	{
		EventSender(argc,argv,SWname);
	}

	if (SWname.compare(NODE_PROCESS_NAME)==0)
	{
		EventSender(argc,argv,SWname);
	}

#ifdef _SNMP
	if (SWname.compare(SNMP_AGENT)==0)
	{
		snmp_main(argc,argv);
	}
#endif

	if (SWname.compare(LOAD_SETTINGS_PROCESS_NAME)==0)
	{

		load_settings(argc,argv);
	}

	if (SWname.compare(SAVE_SETTINGS_PROCESS_NAME)==0)
	{

		save_settings(argc,argv);
	}


	if (SWname.compare(SETTINGS_CLEAN_PROCESS_NAME)==0)
	{

		settings_clean_main(argc,argv);
	}

	if (SWname.compare(SETTINGS_GET_PRIVATE)==0)
	{

		settings_private(argc,argv);
	}

	if (SWname.compare(TO_FACTORY)==0)
	{

		to_factory(argc,argv);
	}

//
	//zr();
#ifdef _SAFE_LOGGER
	if (SWname.compare(SAFE_LOGGER_FW)==0)
	{
		//zr();
		safe_logger(argc,argv);
	}
#endif

	return NO_ERROR;
}


EvSender::EvSender(string & ToProcess,char * event, char * args,int method,int packtype)
	{
		debug_level=NO_USE;
		u16 ToPort=0;
		 if (ToProcess.compare(WDT_PROCESS_NAME)==0)
		 {
			 ToPort=CnT->WDT_PORT_NODEJS;
		 }
		 else
		 {
			 if (ToProcess.compare(NODE_PROCESS_NAME)==0)
			 {
				 ToPort=CnT->NODE_PORT_NODEJS;
			 }
			 else
			 {
				 if (ToProcess.compare(CNODA_PROCESS_NAME)==0)
				 {
				 	 ToPort=CnT->NODE_PORT_CPP;
				 }
				 else
				 {
					 GPRINT(NORMAL_LEVEL,"Incorrect port, Error send Event %s(%s)\n",event,args);
				 	 exit(1);
				 }
			 }
		 }
		 GPRINT(NORMAL_LEVEL,"Send from port %d to port %d\n",htons(CnT->WDT_PORT_CPP),htons(ToPort));
		 if (InitLHBus(CnT->WDT_PORT_CPP,ToPort,(char*)CnT->NODE_GROUP_IP.c_str())==NO_ERROR)
		 {
			 stringstream SendBuf;
			 if (strlen(args)!=0){
				 if (method==EV_SENDER_METHOD_EVENT)
				 {
					 SendBuf<< "{\"t\":["<< packtype <<","<< JSON_PACK_HEADER_VERSION << "],\"d\":{\"" << event << "\":" << args << "}}";
				 }
				 if (method==EV_SENDER_METHOD_RAW)
				 {
					 SendBuf<< "{\"t\":["<< packtype <<","<< JSON_PACK_HEADER_VERSION << "],\"d\":" << event << args <<"}";
				 }
			 }
			 else{
				 SendBuf<< event;
			 }
			 SockSendToLH_Bus((char*)SendBuf.str().c_str(), SendBuf.str().length());
			 GPRINT(NORMAL_LEVEL,"Send Event %s(%s) %s\n",event,args,SendBuf.str().c_str());
		 }
		 else
		 {
			 GPRINT(NORMAL_LEVEL,"Error send Event %s(%s)\n",event,args);
		 }
		 exit(1);

	}


eErrorTp SendDataFromCnodaToUI_base(string dMessage,u32 json_pack_type,u32 json_pack_version, string user ,char * action)
{

			string data="{}";
			string sid="";
			if (user.length()!=0){
				sid=string_format(",\"sid\":\"%s\"",(char*)user.c_str());
			}

			string evnative="{}";

			if (dMessage[0]=='{'){
			  evnative=string_format("%s/evwdt '{\"t\":[%d,%d],\"d\":' '{\"action\":\"%s\",\"%s\":%s}%s}' %d\n",
					  (char*)CnT->CNODA_PATH.c_str(),
					  json_pack_type,
					  json_pack_version,
					  action,
					  action,
					  dMessage.c_str(),
					  sid.c_str(),
					  JSON_PACK_TYPE_TO_UI);
					  //(char*)user.c_str());
			}
			 else{
				 evnative=string_format("%s/evwdt '{\"t\":[%d,%d],\"d\":' '{\"action\":\"%s\",\"%s\":\"%s\"}%s}' %d\n",
					(char*)CnT->CNODA_PATH.c_str(),
					json_pack_type,
					json_pack_version,
					action,
					action,
					dMessage.c_str(),
					sid.c_str(),
					JSON_PACK_TYPE_TO_UI);
					//(char*)user.c_str());
			}

			printf("%s\n",evnative.c_str());
			system(evnative.c_str());
			return NO_ERROR;
			//GPRINT(MEDIUM_LEVEL,"SendEventFromCnoda\n");
			//return SendSharedFifoMessage(srvmSendToLH,"wead", (u8*)container.c_str(),container.size());
}

eErrorTp SendSystemFromCnodaToUI(string dMessageJson)
{
	return SendDataFromCnodaToUI_base(dMessageJson,JSON_PACK_TYPE_SET_SYSTEM,JSON_PACK_VERSION,"","cnoda");
}

eErrorTp SendSystemFromCnodaToUI(string dMessageJson,string user)
{
	return SendDataFromCnodaToUI_base(dMessageJson,JSON_PACK_TYPE_SET_SYSTEM,JSON_PACK_VERSION,user,"cnoda");
}
eErrorTp SendEventFromCnodaToUI(string dMessageJson)
{
	return SendDataFromCnodaToUI_base(dMessageJson,JSON_PACK_TYPE_SEND_EVENT,JSON_PACK_VERSION,"","cnoda");
}

eErrorTp SendEventFromCnodaToUI(string dMessageJson,string user)
{
	return SendDataFromCnodaToUI_base(dMessageJson,JSON_PACK_TYPE_SEND_EVENT,JSON_PACK_VERSION,user,"cnoda");
}
