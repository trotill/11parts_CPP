/*
 * necron.cxx
 *
 *  Created on: 10 дек. 2019 г.
 *      Author: root
 */

#include "necron.h"


u32 EnableWDT=false;
u32 TimeoutWDT=20;
u32 sh_loglevel=NORMAL_LEVEL;
string fifo_file="/run/wdt";
u32 jsSrvDebug=0;
u32 jsJnodaDebug=0;
string necronRoot="/www/pages/necron/";
string PathFirst="/www/pages/sys/";
string PathDef="/etc/necron/";
string rollOutToFactory="/run/necron/rollOutToFactory";
string tmpRoot="/run/necron/";
string webElXorgParams="";
string webElXorgCfgScript="";

u32 webElXorgDpmsStandby=0;
u32 webElXorgDpmsSuspend=0;
u32 webElXorgDpmsOff=0;

string prog_name="necron";
bool webElXorgEn=false;
bool webElectronEn=false;
string webElCmdLine="";
bool restartAction=false;
bool do_exit=false;
pid_t xorgPID;
Json::Value JnodaCfg;



/*eErrorTp existsSync(const char * fname)
{

	FILE * fil=NULL;

	fil=fopen(fname,"rb");

	if (fil==NULL) return ERROR;

	fclose(fil);
	return NO_ERROR;
}*/

eErrorTp readJnodaConfig(void){

	string JnodaStrCfg;

	if (ReadStringFile((char*)string(necronRoot+"Jnoda/Config.json").c_str(),JnodaStrCfg)==NO_ERROR){
		JnodaCfg=parseJSON(JnodaStrCfg);
		return NO_ERROR;
	}
	return ERROR;
	//JnodaCfg
}
eErrorTp readCnodaConfig(string config){
	if (existsSync(config.c_str())==ERROR){
		return ERROR;
	}
	Json::Value json_cfg;
	std::ifstream config_doc(config.c_str(), std::ifstream::binary);
	config_doc >> json_cfg;


	if (json_cfg["EnableWDT"].isNull()==false)
	{
		if (json_cfg["EnableWDT"].isString())
			EnableWDT=stoul(json_cfg["EnableWDT"].asString());
		if (json_cfg["EnableWDT"].isInt())
			EnableWDT=json_cfg["EnableWDT"].asInt();
	}

	if (json_cfg["TimeoutWDT"].isNull()==false)
	{
		if (json_cfg["TimeoutWDT"].isString())
			TimeoutWDT=stoul(json_cfg["TimeoutWDT"].asString());
		if (json_cfg["TimeoutWDT"].isInt())
			TimeoutWDT=json_cfg["TimeoutWDT"].asInt();
	}

	if (json_cfg["sh_loglevel"].isNull()==false)
	{
		if (json_cfg["sh_loglevel"].isString())
			sh_loglevel=stoul(json_cfg["sh_loglevel"].asString());
		if (json_cfg["sh_loglevel"].isInt())
			sh_loglevel=json_cfg["sh_loglevel"].asInt();
	}

	if (json_cfg.isMember("FifoFileWDT"))
	{
		fifo_file=json_cfg["FifoFileWDT"].asString();
	}
	if (json_cfg.isMember("jsSrvDebug"))
	{
		jsSrvDebug=json_cfg["jsSrvDebug"].asUInt();
	}
	if (json_cfg.isMember("jsJnodaDebug"))
	{
		jsJnodaDebug=json_cfg["jsJnodaDebug"].asUInt();
	}
	if (json_cfg.isMember("necronRoot"))
	{
		necronRoot=json_cfg["necronRoot"].asString();
	}
	return NO_ERROR;
}

eErrorTp readWebSrvConfig(string config){
	if (existsSync(config.c_str())==ERROR){
		return ERROR;
	}
	Json::Value json_cfg;
	Json::Value json_cfgd;
	std::ifstream config_doc(config.c_str(), std::ifstream::binary);
	config_doc >> json_cfg;
	json_cfgd=json_cfg["d"];
	if (json_cfgd.isMember("webElectronEn"))
	{
		if (json_cfgd["webElectronEn"].asString()=="true"){
			webElectronEn=true;
		}
	}

	if (json_cfgd.isMember("webElXorgEn"))
	{
		if (json_cfgd["webElXorgEn"].asString()=="true"){
			webElXorgEn=true;
		}
	}

	if (json_cfgd.isMember("webElXorgParams"))
	{
		webElXorgParams=json_cfgd["webElXorgParams"].asString();
	}

	if (json_cfgd.isMember("webElXorgCfgScript"))
	{
		webElXorgCfgScript=json_cfgd["webElXorgCfgScript"].asString();
	}

	if (json_cfgd.isMember("webElCmdLine"))
	{
		webElCmdLine=json_cfgd["webElCmdLine"].asString();
	}

	if (json_cfgd.isMember("webElXorgDpmsStandby"))
	{
		webElXorgDpmsStandby=json_cfgd["webElXorgDpmsStandby"].asInt();
	}
	if (json_cfgd.isMember("webElXorgDpmsSuspend"))
	{
		webElXorgDpmsSuspend=json_cfgd["webElXorgDpmsSuspend"].asInt();
	}
	if (json_cfgd.isMember("webElXorgDpmsOff"))
	{
		webElXorgDpmsOff=json_cfgd["webElXorgDpmsOff"].asInt();
	}
    if (json_cfgd.isMember("webJnodaEnableInspect")){
        if (json_cfgd["webJnodaEnableInspect"]=="true")
            jsJnodaDebug=1;
    }
    if (json_cfgd.isMember("webSrvEnableInspect")){
        if (json_cfgd["webSrvEnableInspect"]=="true")
            jsSrvDebug=1;
    }
	//
	return NO_ERROR;
}


 void forward_handler(int signum)
{
	printf("forward_handler %d\n",signum);
	do_exit=true;
}

static int forward_signal(const int signum)
{
    struct sigaction act;

    memset(&act, 0, sizeof act);
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = forward_handler;
    act.sa_flags = SA_SIGINFO | SA_RESTART;

    //if (signum==SIGPIPE)
    printf("forward_signal signum %d\n",signum);
    if (sigaction(signum, &act, NULL))
        return errno;

    return 0;
}
void xset(void){
	string cmd=sf("DISPLAY=:0.0 xset dpms %d %d %d",webElXorgDpmsStandby,webElXorgDpmsSuspend,webElXorgDpmsOff);
	printf("DPMS[ %s ]\n",cmd.c_str());
	system(cmd.c_str());
	if ((webElXorgDpmsStandby==webElXorgDpmsSuspend)&&(webElXorgDpmsSuspend==webElXorgDpmsOff)&&(webElXorgDpmsOff==0)){
		system("DISPLAY=:0.0 xset s off -dpms");
		printf("DPMS[ xset s off -dpms ] DPMS force off\n");
	}
	else{
		system("DISPLAY=:0.0 xset dpms force on");
		printf("DPMS[ xset dpms force on ] DPMS force on\n");
	}
}
string genFactBashScriptNand(void){
	return ""
			"#!/bin/sh\n"
			"\n"
			"FIT_PART=2\n"
			"RW_MOUNTP=/www/pages\n"
			"RW_PART=4\n"
			"RW_FS=ubi\n"
			"\n"
			"echo Erase user fit parts\n"
			"flash_erase /dev/mtd$FIT_PART 0 0\n"
			"\n"
			"umount -f $RW_MOUNTP\n"
			"if [ \"$RW_FS\" == \"ubi\" ]; then\n"
			"    ubidetach -m $RW_PART\n"
			"fi\n"
			"\n"
			"echo \"Erase ubi parts\"\n"
			"flash_erase /dev/mtd$RW_PART 0 0\n"
			". /etc/init.d/necron_extract_ubifs 4 1 0 /www/pages /usr/share/necron\n"
			"echo \"Success...Reboot...\"\n";
}
string genFactBashScriptDebug(void){
	return ""
			"#!/bin/sh\n"
			"ls / -l\n"
			"echo Finish\n";
}
string genFactBashScriptV2(void){
	return ""
			"#!/bin/sh\n"
			"source /etc/necron/necron_image.conf\n"
			"echo remove FIT2\n"
			"dd if=/dev/zero of=$NECRON_FIT2_ROOT_DEVICE bs=1M count=1\n"
			"\n"
			"echo umount $BASE_FOLDER$SYS_FOLDER $BASE_FOLDER$SYS_EX_FOLDER ${TMP_DIR}/disks$SYS_FOLDER ${TMP_DIR}/disks$SYS_EX_FOLDER ${TMP_DIR}/disks/shared"
			"$BASE_FOLDER$DOWNLOAD_FOLDER $BASE_FOLDER$UPDATE_FOLDER $BASE_FOLDER$LOG_FOLDER\n"
			"umount   $BASE_FOLDER$SYS_FOLDER\n"
			"umount   $BASE_FOLDER$SYS_EX_FOLDER\n"
			"umount ${TMP_DIR}/disks$SYS_FOLDER\n"
			"umount ${TMP_DIR}/disks$SYS_EX_FOLDER\n"
			"umount ${TMP_DIR}/disks/shared\n"
			"umount  $BASE_FOLDER$DOWNLOAD_FOLDER\n"
			"umount  $BASE_FOLDER$UPDATE_FOLDER\n"
			"umount  $BASE_FOLDER$LOG_FOLDER\n"
			"\n"
			"echo Format sys,sys_ex,shared\n"
			"mkfs.$NECRON_FS_SYS_MOUNT_FS $NECRON_SYS_ROOT_DEVICE $NECRON_MKFS_SYS_OPTS\n"
			"mkfs.$NECRON_FS_SYS_MOUNT_FS $NECRON_SYSEX_ROOT_DEVICE $NECRON_MKFS_SYS_OPTS\n"
			"mkfs.$NECRON_FS_SHARE_MOUNT_FS $NECRON_SHARE_ROOT_DEVICE $NECRON_MKFS_SHARE_OPTS\n"
			"\n"
			"DeploySysSettings \"\"\n"
			"echo mount $NECRON_SYS_ROOT_DEVICE $BASE_FOLDER$SYS_FOLDER\n"
			"mount $NECRON_SYS_ROOT_DEVICE $BASE_FOLDER$SYS_FOLDER\n"
			"echo Finish\n";
}

void killAllXorg(){
	printf("kill Xorg, found CNT %d\n",SearchProcess("Xorg"));
	KillProcess("Xorg","killall -9 Xorg",1000);
}
void killAllBase(){
	printf("kill electron, found CNT %d\n",SearchProcess("electron"));
	KillProcess("electron","killall -9 electron",1000);
	printf("kill node, found CNT %d\n",SearchProcess("node"));
	KillProcess("node","killall -9 node",1000);
	//printf("kill Xorg, found CNT %d\n",SearchProcess("Xorg"));
	//KillProcess("Xorg","killall -9 Xorg",1000);
	printf("kill Cnoda, found CNT %d\n",SearchProcess("Cnoda"));
	KillProcess("Cnoda","killall -9 Cnoda",1000);
	printf("kill svc, found CNT %d\n",SearchProcess("svc"));
	KillProcess("svc","killall -9 svc",1000);
	printf("kill tail, found CNT %d\n",SearchProcess("tail"));
	KillProcess("tail","killall -9 tail",1000);
}
void factoryRollBack(void){
	string immortal=tmpRoot+"immortal/";
	mkdir(immortal.c_str(),0xffffffff);
	Json::Value immortalRoot;
	eErrorTp errCopy=NO_ERROR;
	printJSON("JnodaCfg",JnodaCfg);

	if (JnodaCfg.isMember("factory")){
		if (JnodaCfg["factory"].isMember("immortal")==false)
			JnodaCfg["factory"]["immortal"]=Json::Value(Json::arrayValue);
		immortalRoot=JnodaCfg["factory"]["immortal"];
		printJSON("immortalRoot",immortalRoot);
		for (u32 n=0;n<immortalRoot.size();n++){
			string settSrc=PathFirst+immortalRoot[n].asString();
			string settDst=immortal+immortalRoot[n].asString();

			if (existsSync(settSrc.c_str())==NO_ERROR){//если файла в сборке нет, то ничего страшного, ищем следующие по списку
				if (CopyFile((char*)settSrc.c_str(),(char*)settDst.c_str())==ERROR){//но если при копировании ошибка, то все остановить
					errCopy=ERROR;
					printf("Error copy %s to %s, stop\n",(char*)settSrc.c_str(),(char*)settDst.c_str());
					break;
				}
				else
					printf("Copy %s to %s\n",(char*)settSrc.c_str(),(char*)settDst.c_str());
			}
		}
		if (errCopy==NO_ERROR){
			string scr;
			//если все вечные конфиги сохранены, то запускаем процедуру
			if (existsSync((char*)string(PathDef+"necron_image.conf").c_str())==NO_ERROR){//проверка типа сборки
				//тип для emmc
				scr=genFactBashScriptV2();
			}
			else{
				//
				if (existsSync("/etc/init.d/necron_extract_ubifs")==NO_ERROR)//тип для nand
					scr=genFactBashScriptNand();
				else{
					printf("Undef image, skip roll back\n");
					scr=genFactBashScriptDebug();
					//return;
				}
			}
			string scrFile=tmpRoot+"factory.sh";
			if (WriteStringFile((char*)scrFile.c_str(),scr)==NO_ERROR){
				chmod((char*)scrFile.c_str(), S_IRWXU);
				//while(1){
				//	system("killall -9 node");
				//	system("killall -9 electron");
				//	system("killall -9 Cnoda");
				//	system("killall -9 safe_logger");
				//	system("killall -9 Xorg");

				//}
				killAllBase();
				printf("kill safe_logger, found CNT %d\n",SearchProcess("safe_logger"));
				KillProcess("safe_logger","killall -9 safe_logger",1000);

//#ifndef ROLL_OUT_FACTORY_DEBUG
				printf("script ---\n%s\n",scr.c_str());//для отладки
				string scrCmd=scrFile;//string("source ")+scrFile;
				system(scrCmd.c_str());//выполнить свежесгенеренный скрипт
				//string out=BashResult(scrCmd);//выполнить свежесгенеренный скрипт
				//printf("script result %s\n",out.c_str());
				CopyFile((char*)scrFile.c_str(),(char*)string(PathFirst+"factory.sh").c_str());//для отладки
//#endif
				errCopy=NO_ERROR;
				for (u32 n=0;n<immortalRoot.size();n++){

					string settSrc=immortal+immortalRoot[n].asString();
					string settDst=PathFirst+immortalRoot[n].asString();

					if (existsSync(settSrc.c_str())==NO_ERROR){//если файла нет, то ничего страшного, ищем следующие по списку
						if (CopyFile((char*)settSrc.c_str(),(char*)settDst.c_str())==ERROR){
							errCopy=ERROR;
							printf("Error copy %s to %s, stop\n",(char*)settSrc.c_str(),(char*)settDst.c_str());
							break;
						}
						else{
							printf("Copy %s to %s\n",(char*)settSrc.c_str(),(char*)settDst.c_str());
						}
					}
				}
				if (errCopy==ERROR){
					printf("Error rollout, copy error, go to service center\n");
				}
				else{
					printf("Rollout is Okay, reboot system!!!!");
#ifndef ROLL_OUT_FACTORY_DEBUG
					system("reboot");//перезагрузить
#endif
				}
			}


		}
		else{
			printf("Cancel rollout, copy error\n");
		}
	}

}

void factoryLoop(void){
	while(1){
		sleep(1);
		if (existsSync(rollOutToFactory.c_str())==NO_ERROR){
			printf("Found factoryRollBack cmd file\n");
			factoryRollBack();
			//remove(rollOutToFactory.c_str());
			break;
		}
		//printf("check factory do_exit %d\n",do_exit);
		if (do_exit)
			break;
	}
}

void reReadWebSrvConfig(){
    string settingsWebsrv=PathFirst+"settings.websrv.set";
    string settingsWebsrvDef=PathDef+"settings.websrv.set";
    printf("reReadWebSrvConfig\n");
    if (existsSync(settingsWebsrv.c_str())==ERROR){
        printf("Warning, not found settings.websrv.set, search default\n");
        if (existsSync(settingsWebsrvDef.c_str())==ERROR){
            printf("Critical error, image is broken, not found default settings.websrv.set\n");
        }
        else{
            readWebSrvConfig(settingsWebsrvDef);
            printf("default settings.websrv.set is founded\n");
        }
    }
    else{
        readWebSrvConfig(settingsWebsrv);
        printf("found settings.websrv.set\n");
    }
}

int main(int argc, char *argv[])
{
	//necron /www/pages/necron/main.js
	//if (argc!=2){
	//	exit(-1);
	//}

	string jspath="/www/pages/necron/main.js";
	string dbgopt1="";
	string dbgopt2="";
	string config=necronRoot+"/Cnoda.json";


	if (argc>1){
		config=argv[1];

	}

	/*
	if (argc>2){
		if (strcmp(argv[2],"debug")==0)
			dbgopt1="--inspect=0.0.0.0:9229";
			dbgopt2="debug_jnoda";
		if (strcmp(argv[2],"config")==0){
			if (argc>3){
				config=argv[3];
			}
		}
	}*/
	mkdir(tmpRoot.c_str(),0xffffffff);//fix, force create necron
	///run/necron
	readCnodaConfig(config);
	readJnodaConfig();
    reReadWebSrvConfig();

	if (jsSrvDebug){
		dbgopt1="--inspect=0.0.0.0:9229";
	}
	if (jsJnodaDebug){
		dbgopt2="debug_jnoda";
	}

	if (dbgopt1.length()>0)
		printf("Enable debug mode, open chrome://inspect/#devices\n");

//	(string dev,u32 timeout,u32 enable,eDebugTp lvl);
	//watchdog wdt("/dev/watchdog",TimeoutWDT,EnableWDT,(eDebugTp)sh_loglevel);
	//wdt.keepalive(TimeoutWDT);
		//watchdog wdt("/dev/watchdog",TimeoutWDT,EnableWDT,(eDebugTp)sh_loglevel);


	if (EnableWDT){
		int fifo_fd = open((char*)fifo_file.c_str(), O_RDWR);

		write(fifo_fd,"cnoda",6);
		printf("WDT activated\n");
	}

	string node="node";
	if (webElectronEn){
		node="electron";
		dbgopt2+=" --no-sandbox";
		if (webElCmdLine.size()!=0){
			dbgopt2+=" ";
			dbgopt2+=webElCmdLine;
		}
	}
	else
		webElXorgEn=false;
	killAllXorg();
	printf("webElXorgParams %s\n",webElXorgParams.c_str());
	if (webElXorgEn){
		Exe(xorgPID,"Xorg",webElXorgParams);
	}
	string nodeRun="export DISPLAY=:0.0&& cd "+necronRoot+"&& stdbuf -o0 -i0 -e0 "+node+" "+dbgopt1+" "+jspath+" "+dbgopt2;


	forward_signal(SIGINT);
	forward_signal(SIGHUP);
	forward_signal(SIGTERM);
	forward_signal(SIGQUIT);
	forward_signal(SIGUSR1) ;
	forward_signal(SIGPIPE) ;
	forward_signal(SIGUSR2);


	restartAction=true;
	//
	//system("killall -9 svc");
	//system("killall -9 Cnoda");
	//system("killall -9 tail");

	killAllBase();
	//system("/etc/init.d/slogger.sh restart");
	//sleep(2);
	//system("ps -aux|grep slogger");
	//sleep(2);
	//system("ps -aux|grep slogger");
	//sleep(2);
	//system("ps -aux|grep slogger");
	//system("sh /etc/init.d/slogger.sh restart");
	int xOrgPIDstatus;
	int nodePID;
	int rbFactoryPID;
	//zr();
	rbFactoryPID=fork();
	if (rbFactoryPID==0){
		factoryLoop();
		exit(1);
	}
	//zr();
	//exit(1);
	while(1){
		//printf("runnung %d count %d name %s\n",do_exit,SearchProcess(node.c_str()),node.c_str());
		if (webElXorgEn){
			if (waitpid(xorgPID, &xOrgPIDstatus, WNOHANG)!=0)
			{
				if (!restartAction){
					printf("@@@@\n@@@@\nXorg down, restart!!!\n@@@@\n@@@@\n");
                    reReadWebSrvConfig();
					Exe(xorgPID,"Xorg",webElXorgParams);
					restartAction=true;
					//exit(1);
				}

			}
		}

		if (waitpid(nodePID, &xOrgPIDstatus, WNOHANG)!=0){
			restartAction=true;
			killAllXorg();
			printf("@@@@\n@@@@\%s down, restart!!!\n@@@@\n@@@@\n",node.c_str());
			//Exe(xorgPID,"Xorg",webElXorgParams);
			//exit(1);

		}

		if (restartAction){
			if (existsSync(rollOutToFactory.c_str())==NO_ERROR){//Если происходит возврат к заводск. настр, то не перезапускать приложения
				printf("Skip restartAction, found factoryRollBack cmd file\n");
				exit(1);
			}
			cout << sf("killall -9 %s",node.c_str()) << endl;

			system(sf("killall -9 %s",node.c_str()).c_str());
            reReadWebSrvConfig();
			if (webElXorgEn){
				//killAllXorg();
				//Exe(xorgPID,"Xorg",webElXorgParams);
				sleep(1);
				xset();
				cout << sf("echo -e \"%s\"|bash",webElXorgCfgScript.c_str()) << endl;
				string bres=BashResult(sf("echo -e \"%s\"|bash",webElXorgCfgScript.c_str()));
				printf("bres %s\n",bres.c_str());

				printf("run [%s]\n",nodeRun.c_str());

				//exit(1);

				//system(run.c_str());

			}
			//else{


			nodePID=fork();
			restartAction=false;
			if (nodePID==0){
				system("/etc/init.d/slogger.sh restart");
				//Не использовать BashResult, скрипт не все slogger запускает!!!
				//BashResult("/etc/init.d/slogger.sh restart");
				printf("nodeRun [%s]\n",nodeRun.c_str());
				system(nodeRun.c_str());
				if (existsSync(rollOutToFactory.c_str())==ERROR){//Если происходит возврат к заводск. настр, то не завершать приложения
					killAllBase();
				}
				//system("killall -9 node");
				//system("killall -9 electron");
				//system("killall -9 Cnoda");
				//system("killall -9 Xorg");
				printf("exit child nodePID\n");
				exit(1);
			}
			//else
				//restartAction=false;
			//}
			//restartAction=false;
		}
		mdelay(300);
		if (do_exit){
			if (webElXorgEn)
				system("killall Xorg");

			system(sf("killall %s",node.c_str()).c_str());
			exit(0);
		}
	}
	//
}


