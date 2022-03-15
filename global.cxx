/*
 * global.cxx
 *
 *  Created on: 16 дек. 2018 г.
 *      Author: root
 */
#include "global.h"
#include "engine/proto/json_proto.h"
#include "engine/settings_adm.h"

eErrorTp CpuInfo::init(){
	 cpu_result=Json::Value(Json::arrayValue);
	 FILE *cpuinfo = fopen("/proc/cpuinfo", "r");

	 if (cpuinfo==NULL)
		 return ERROR;

	 char line[2048];
	 string str_line;

	 while (1){

		 if (fgets(line, sizeof(line), cpuinfo)==NULL)
			 break;

		 str_line=line;
		 string param;
		 string value;
		 size_t posd=str_line.find(':');
		 if (posd!=string::npos){
			 param=str_line.substr(0,posd);
			 param=delCh(param,'\t');
			 value=str_line.substr(posd+1,str_line.size()-posd);
			 value=delCh(value,'\t');
			 value=delCh(value,0x09);
			 value=delCh(value,'\n');
			 value=delSpaces(value);
		 }

		 if (param=="processor"){
			 cpu_idx=stoul(value);
		 }
		 if (param.length()!=0)
			 cpu_result[cpu_idx][param]=value;

	 }
	 fclose(cpuinfo);

	 if (cpu_result.size()==0)
		 return ERROR;

	 return NO_ERROR;
}
eErrorTp DistroInfo::init(){
		//SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]
		//Json::Value json_cfg;
		string distro_cfg="settings.distro";
		string websrv_cfg="settings.websrv";

		SettingsAdm sm;

		if (sm.LoadSetting(websrvCfg,websrv_cfg)!=ERROR){
			if (websrvCfg["d"].isMember("version"))
				version=websrvCfg["d"]["version"].asString();
			if (websrvCfg["d"].isMember("webJnodaUsePrivate"))
				if (websrvCfg["d"]["webJnodaUsePrivate"]=="true")
					webJnodaUsePrivate=true;

		}
		Json::Value root;
		if (sm.LoadSetting(root,distro_cfg)!=ERROR){
			cpu=root["d"]["cpu"].asString();
			hw=root["d"]["hw"].asString();
			version_hw=root["d"]["version_hw"].asString();
			version_major=root["d"]["version_major"].asString();
			swvers=root["d"]["swvers"].asString();
			swbuild=root["d"]["swbuild"].asString();
			hwvers=root["d"]["hwvers"].asString();
			swdate=root["d"]["swdate"].asString();
			cpu_count=root["d"]["cpu_count"].asString();
			hwmanuf=root["d"]["hwmanuf"].asString();
			hwmodel=root["d"]["hwmodel"].asString();
			inited=true;
			return NO_ERROR;
		}
		else{
			printf("Error: distro cfg [%s]!!!\n",distro_cfg.c_str());
			return ERROR;
		}


	};

eErrorTp CnodaT::ReadConfigConf(string config){
		if (SearchFile(config.c_str())==ERROR){
			return ERROR;
		}

		std::ifstream config_doc(config.c_str(), std::ifstream::binary);
		config_doc >> json_cfg;

		JSON_ReadConfigField(json_cfg,"NodePortCpp",NODE_PORT_CPP);
		JSON_ReadConfigField(json_cfg,"NodePortNodejs",NODE_PORT_NODEJS);
		JSON_ReadConfigField(json_cfg,"WdtPortCpp",WDT_PORT_CPP);
		JSON_ReadConfigField(json_cfg,"WdtPortNodejs",WDT_PORT_NODEJS);

		JSON_ReadConfigField(json_cfg,"PathFirst",SETTING_STORAGE_MOUNTP[0]);
		JSON_ReadConfigField(json_cfg,"PathReserved",SETTING_STORAGE_MOUNTP[1]);
		JSON_ReadConfigField(json_cfg,"PathDefault",SETTING_STORAGE_MOUNTP[2]);

		JSON_ReadConfigField(json_cfg,"NodeGroupIp",NODE_GROUP_IP);
		JSON_ReadConfigField(json_cfg,"DownloadPath",DOWNLOAD_PATH);
		JSON_ReadConfigField(json_cfg,"CachePath",CACHE_PATH);
		JSON_ReadConfigField(json_cfg,"SafeLoggerConfig",SAFE_LOGGER_CONFIG);

		JSON_ReadConfigField(json_cfg,"FirmwarePath",FIRMWARE_PATH);
		JSON_ReadConfigField(json_cfg,"FirmwareName",FIRMWARE_NAME);
		JSON_ReadConfigField(json_cfg,"ShowSysInfoDump",cpu_and_distro_dump);
		JSON_ReadConfigField(json_cfg,"debug",debug);
		JSON_ReadConfigField(json_cfg,"DnkVariablesConfig",DNK_VAL_CFG);
		JSON_ReadConfigField(json_cfg,"dnk_loglevel",dnk_loglevel);
		JSON_ReadConfigField(json_cfg,"version",version);
		return NO_ERROR;
	}
void CnodaT::SigHandler(int snum)
{
	printf("%s:Catched signal %d\n",__FILE__,snum);
	printf("%s:SIGINT %d,SIGPIPE %d,SIGURG %d, SIGCHLD %d SIGTERM %d\n",__FILE__,SIGINT,SIGPIPE,SIGURG,SIGCHLD,SIGTERM);

	if (pthread_mutex_trylock(&glbSigMutex)!=0)
		printf("%s:SigHandler blocked\n",__FILE__);
	else
	{
		switch (snum) {
			case SIGPIPE: break;
			//case SIGTERM:TerminateEngine(); break;
			case SIGINT:
			case SIGTERM:
			case SIGKILL:
				SWTerminateReq=1;
				//TerminateEngine();
			break;
			//case SIGCHLD:TerminateEngine(); break;
		}

		pthread_mutex_unlock(&glbSigMutex);
	}
	//delmy!!!
	//if (snum==SIGHUP) sleep(10);
	//delmy!!!

	//if ((snum!=SIGCHLD)|(snum!=SIGPIPE))
	//  TerminateEngine();


}

void SigHandlerWrapper(int snum){
	CnT->SigHandler(snum);
}
void CnodaT::SigActionConfig(void)
{
	struct sigaction act;

	sigemptyset(&act.sa_mask);
	act.sa_handler=&SigHandlerWrapper;
	act.sa_flags=0;

	sigset_t newset;
	sigemptyset(&newset);
	sigaddset(&newset, SIGPIPE);
	sigprocmask(SIG_BLOCK, &newset, 0);

	if (sigaction(SIGINT,&act,NULL)==-1){
		printf("%s:Signal SIGINT is blocked\n",__FILE__);
	}

	if (sigaction(SIGTERM,&act,NULL)==-1){
			printf("%s:Signal SIGTERM is blocked\n",__FILE__);
	}

	if (sigaction(SIGKILL,&act,NULL)==-1){
		printf("%s:Signal SIGKILL is blocked\n",__FILE__);
	}


    if (sigaction(SIGPIPE,&act,NULL)==-1){
		printf("%s:Signal SIGPIPE is blocked\n",__FILE__);
	}

	if (sigaction(SIGURG,&act,NULL)==-1){
		printf("%s:Signal SIGURG is blocked\n",__FILE__);
	}
}

void CnodaT::Help(void)
	{
		u32 i;
		cout << PROGRAMM_NAME << " server V" << GetProgVersion() << " (" << __DATE__ << ")" << endl;
		cout << "This help " << endl;
		cout << "   opts --help" << endl;
		cout << "configuration file, default /Cnoda.json" << endl;
		cout << "   opts --conf" <<endl;
		cout << "For example: " << PROGRAMM_NAME << " --conf=/etc/config.json" << endl;


		printf("------------------------------------------------------------\n");
	}

eErrorTp CnodaT::ParseOpts(int argc, char *argv[])
	{
		int c;
		u8 trig=0;

		static struct option long_options[] =
	            {
	              {"help",     no_argument,       0, 'h'},
	              {"conf",  required_argument,       0, 'c'},
	              {0, 0, 0, 0}
	            };
		 struct in_addr inp;
		 while (1)
		 {
			 int option_index = 0;

	          c = getopt_long (argc, argv, "eh:c:s:m",
	                           long_options, &option_index);

	          /* Detect the end of the options. */
	          if (c == -1)
	            break;

	          switch (c)
	            {
	            case 0:

	              /* If this option set a flag, do nothing else now. */
	              if (long_options[option_index].flag != 0)
	                break;
	              cout << "option " << long_options[option_index].name;
	              if (optarg)
	                cout << "with arg " << optarg;
	              cout << endl;
	              break;

	            case 'c':
	            	if (optarg!=0)
	            	{
	            		if (SearchFile(optarg)!=ERROR)
	            		{
	            			SERVER_CONFIG_FILE=optarg;
	            			trig=1;
	            			 cout << "Found conf " << SERVER_CONFIG_FILE << endl;
	            		}
	            	}
	            	break;

	            	//break;
	            case 'h':

	            case '?':
	            	Help();
	            	exit(0);
	            	break;
	              /* getopt_long already printed an error message. */
	            default:
	              printf("Incorrect options, exit\n");
	              exit(0);
	            }
		 }
		 if (trig==0)
			 cout << "Not found Cnoda.json, use default conf " << SERVER_CONFIG_FILE << endl;

	   return NO_ERROR;
	 }


eErrorTp GenLangJSON(void){

	return NO_ERROR;
}

eErrorTp GenSystemValues(Json::Value & gen){
	gen["x"]["v"]["$"]["sast"]["motohour"]["settings.total_operating_time"]="tot";
	gen["x"]["v"]["$"]["sast"]["hostname"]["settings.websrv"]="hostname";
	gen["x"]["v"]["$"]["sast"]["device_reboot_cnt"]["settings.stat"]="device_reboot_cnt";
	gen["x"]["v"]["$"]["sast"]["server_reboot_cnt"]["settings.stat"]="server_reboot_cnt";
	gen["x"]["v"]["motohour"]=0;
	gen["x"]["v"]["uptime"]=0;
	gen["x"]["v"]["hostname"]="11p";
	gen["x"]["v"]["device_reboot_cnt"]=0;
	gen["x"]["v"]["server_reboot_cnt"]=0;
	gen["private"]["sys"]["sn"]["sn"]="0000000";

	gen["x"]["srclang"]="en";
	gen["x"]["wordbook"]["en"]["test"]="test";

	return NO_ERROR;
}

eErrorTp ShowTypeSize(){
	printf("Data types\n");
	printf("u8-%u\n",(u32)sizeof(u8));
	printf("u16-%u\n",(u32)sizeof(u16));
	printf("u32-%u\n",(u32)sizeof(u32));
	printf("u64-%u\n",(u32)sizeof(u64));
	printf("f32-%u\n",(u32)sizeof(f32));
	//printf("ld64-%u\n",(u32)sizeof(ld64));
	printf("d64-%u\n",(u32)sizeof(d64));
	printf("size_t-%u\n",(u32)sizeof(size_t));
	printf("time_t-%u\n",(u32)sizeof(time_t));
	eErrorTp err=ERROR;

	if ((sizeof(u8)==1)&&(sizeof(u16)==2)&&(sizeof(u32)==4)&&(sizeof(u64)==8)&&
			(sizeof(f32)==4)&&(sizeof(d64)==8))
		err=NO_ERROR;

	if (sizeof(size_t)!=4)
			printf("Warning size_t!=4 %u!!!\n",(u32)sizeof(size_t));

	if (sizeof(time_t)!=4)
			printf("Warning time_t!=4 %u!!!\n",(u32)sizeof(time_t));

	return err;
}

eErrorTp CnodaT::LoadLanguageWordbook(){
	SettingsAdm sm;
	sm.LoadSetting(lang,"settings.lang");
	return NO_ERROR;
}
eErrorTp CnodaT::Init(int argc,char *argv[]){

		mutex_init(ConsLogMutex);
		mutex_init(SettingSave);

		if (strcmp(program_invocation_short_name,PROGRAMM_NAME)!=0){
				RunMode=RUN_MODE_INPUT;
				//IBOARD_LOG_LEVEL=NO_USE;
				MiniSW_Run(argc,argv,program_invocation_short_name);
				exit(0);
			}

		printf("Compiler version %ld\n",__cplusplus );
		if (ShowTypeSize()==ERROR){
			printf("Crit error, incorrect types\n");
			exit(1);
		}
		if (RunMode!=RUN_MODE_INPUT)
					SigActionConfig();

				//LogTest();
		cout << "\nStarted " << PROGRAMM_NAME << " server (SE), V" << GetProgVersion() << " (" << __DATE__ << ")" << endl;
		cout << "--------------------" << endl;
		cout << "Contacts for support: \n  Gorchakov Ilya V.\n  info@11-parts.com" << endl;
		cout << "--------------------" << endl;

		ParseOpts(argc,argv);
		if (ReadConfigConf(SERVER_CONFIG_FILE)==ERROR)
		{
			cout << "Not found configuration file " << SERVER_CONFIG_FILE << endl;
			cout << PROGRAMM_NAME << " will be terminated!!!" << endl;
			exit(1);
		}

		GetFirstRunState();
		dinfo.init();
		if (dinfo.version.size()!=0)
			version=dinfo.version;

		cout << "version [" << version << "]"<< endl;

		if (cpu_and_distro_dump)
			dinfo.dump();

		cpuinfo.init();

		if (cpu_and_distro_dump)
			cpuinfo.dump();

		//string wdt_stub="0";
		//if (JSON_ReadConfigField(json_cfg,"StubWDT",wdt_stub)!=ERROR)
		//	if (wdt_stub=="1"){
		//		cout << "StubWDT, watchdog skiped!!!\n"<<endl;
		//		system(string(CNODA_PATH+"/watchdog_stub").c_str());
		//	}
		eErrorTp err;
		string dnkcfg=CNODA_PATH+'/'+DNK_VAL_CFG;
		Json::Value gen;

		GenSystemValues(gen);
		if (existsSync(dnkcfg)==ERROR){

			//zr();
			for (u32 n=0;n<5;n++){
				cout << "DNK cfg " << dnkcfg<<" not found" << endl;
				sleep(1);
			}
			dnk_var=make_shared<DNKchainT>(dnk_loglevel,gen,"gen_base_dnk",err);
		}
		else{
			cout << "Found DNK cfg " << dnkcfg << endl;
			dnk_var=make_shared<DNKchainT>(dnk_loglevel,CNODA_PATH,DNK_VAL_CFG,err);
		}


		LoadDNKFromStorage();

		dnk_var->Merge(gen);


		LoadPrivateFromStorage();
		LoadLanguageWordbook();

		if (err==ERROR){
					cout << "Error load DNK "<<CNODA_PATH<<"/"<<DNK_VAL_CFG<<" settings " << endl;

		}
		else{
			dnk_var->SAST_Create_FilesDB_Standart(sast_db);
		}

		//dnk_var->Dump("",false);
		GetValueSAST("private.sys.sn.sn",dinfo.sn);
		#ifdef _SECURE_ENABLE
			epsec::secure sec;
			deviceUID=sec.getDeviceUID();
		#endif

		//dnk_var->Dump("x.ws.store0.spec",false);
		//exit(1);
		return NO_ERROR;
}
eErrorTp emit(u8 msg,string str){
	return shared_fifo->SendSharedFifoMessage(msg,"all",str);
}

void PowerOffSystem(void){
	u32 skip_poweroff=0;
	JSON_ReadConfigField(CnT->json_cfg,"skip_poweroff",skip_poweroff);
	printf("\n\nPoweroff\n\n");
	if (!skip_poweroff)
		system("poweroff");
	else{
		printf("\n\nSkip poweroff\n\n");
	}
}

#if 0
void CnodaT::Step(void){

	if (SWTerminateReq){
		//if (RunMode==RUN_MODE_SNIF)//snif mode
		//{
		//tc.Remove()
		tc->RemoveAll();
		cout << "\nStoped " << PROGRAMM_NAME << " server, V" << GetProgVersion() << " (" << __DATE__ << ")" << endl;
		delay(2);
		printf("exit \n");
		exit(1);

		//}
	}


	//if (SWTerminate){
		/*
		cout << "GV.SWTerminate ";
		SetTermRiseT(ThreadList.WWW,TERM_REQ_RISE);
		SetTermRiseT(ThreadList.iBOARD,TERM_REQ_RISE);
		SetTermRiseT(ThreadList.Helper,TERM_REQ_RISE);
		SetTermRiseT(ThreadList.PARAMS,TERM_REQ_RISE);
		mdelay(100);
		printf("iB%d WW%d HLP%d PR%d\n",CheckTermRiseT(ThreadList.iBOARD),CheckTermRiseT(ThreadList.WWW),CheckTermRiseT(ThreadList.Helper),CheckTermRiseT(ThreadList.PARAMS));
		if ((CheckTermRiseT(ThreadList.iBOARD)==TERM_STATE_RISE)&&
			(CheckTermRiseT(ThreadList.WWW)==TERM_STATE_RISE)&&
			(CheckTermRiseT(ThreadList.Helper)==TERM_STATE_RISE)&&
			(CheckTermRiseT(ThreadList.PARAMS)==TERM_STATE_RISE))
			{
				cout << "\nStoped " << PROGRAMM_NAME << " server, V" << GetProgVersion() << " (" << __DATE__ << ")" << endl;
				//log->to_log(1,SYSTEM_SERVER_STOPED);
				break;
			}*/
		//exit(0);

	//}
}
#endif
