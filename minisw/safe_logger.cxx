/*
 * safe_logger.cxx
 *
 *  Created on: 18 апр. 2018 г.
 *      Author: root
 */
#include "global.h"
#include "logger/safe_logger.h"

//chpst -P -u root:root stdbuf -i0 -o0 -e0 node /www/pages/necron/test1.js

//chpst -P -u root:root stdbuf -i0 -o0 -e0 Cnoda
int safe_logger(int argc, char **argv) {

	if (argc<=1){
		printf("Error run safe_logger, not set config, exit\n");
		exit(-1);
	}
	//printf("argc %d\n",argc);
	string config=argv[1];
	if (CnT->ReadConfigConf(config)==ERROR){
		printf("Error run safe_logger, config incorrect, exit\n");
		exit(-1);
	}

	u32 ll=NORMAL_LEVEL;


	JSON_ReadConfigField(CnT->json_cfg,"slogger_loglevel",ll);
	//printf("%s",StyledWriteJSON(CnT->json_cfg).c_str());
	eDebugTp level=(eDebugTp)ll;
	//printf("loglevel %d\n",ll);
	bool create_pipe=false;
	if (argc==2)
		create_pipe=true;

	safelogger_server srv(create_pipe,config,level);
	CnT->SigActionConfig();
	//printf("argc %d\n",argc);
	if (argc==2){
		//srv.create_fifo();
		while(1){
			mdelay(30);
			srv.get();
			if (CnT->SWTerminateReq==1){
				printf("Exit safe_logger\n");
				return 0;
			}
		}
	}
	else{
		string mode=argv[2];
		if (mode=="buildlog"){
			srv.buildlog();
		}
	}

	return 0;
}
