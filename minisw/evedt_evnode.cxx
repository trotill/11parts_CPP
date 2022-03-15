/*
 * evedt.cxx
 *
 *  Created on: 18 дек. 2018 г.
 *      Author: root
 */


#include "engine/minisw/minisw.h"

eErrorTp EventSender(int argc,char *argv[],string SWname)
{
	//argv[1] -
	//printf("EventSender\n");
	if ((argv[1]==NULL))
		exit(1);

	char * evargs;
	int packtype=JSON_PACK_TYPE_SEND_EVENT;
	int method=EV_SENDER_METHOD_EVENT;
	char * nullstr="";
	if (argc>2){
		evargs=argv[2];
	}
	else
		evargs=nullstr;
	if (argc>3){
		packtype=stoul(argv[3]);
		method=EV_SENDER_METHOD_RAW;
	}
	//printf("packtype %d\n",packtype);
	EvSender * Ev=new EvSender (SWname,argv[1],evargs,method,packtype);

	return NO_ERROR;
}
