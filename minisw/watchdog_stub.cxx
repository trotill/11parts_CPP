/*
 * watchdog.cxx
 *
 *  Created on: 22 янв. 2019 г.
 *      Author: root
 */
#include "engine/periphery/watchdog.h"
#include "engine/lib/11p_process.h"
#include "engine/proto/json_proto.h"
//#include "engine/global.h"

string fifo_file="/run/wdt";
string cnoda_setting="/www/pages/necron/Cnoda/Cnoda.json";
Json::Value jscfg;
u32 EnableWDT=0;
u32 TimeoutWDT=120;
int f=0;
#define FIFO_BUF_SIZE 512
#define MAX_SIZE_STRING_FILES 20000
#define DEFAULT_WDT_TIMEOUT 20
u8 rbuf[FIFO_BUF_SIZE];

eErrorTp open_fifo(void){
		//if (fifo_file.length()!=0){
			remove(fifo_file.c_str());
			mkfifo(fifo_file.c_str(), 0666);
			f = open(fifo_file.c_str(), O_NONBLOCK | O_RDWR);
			//GPRINT(NORMAL_LEVEL,"Init fifo_file [%s]\n",fifo_file.c_str());
		//}

		return NO_ERROR;

}

void close_fifo(void){
	if (f!=0){
		//GPRINT(NORMAL_LEVEL,"Close f\n");
		close(f);
		f=0;
	}
}

eErrorTp existsSync(const char * fname)
{

	FILE * fil=NULL;

	fil=fopen(fname,"rb");

	if (fil==NULL) return ERROR;

	fclose(fil);
	return NO_ERROR;
}



eErrorTp ReadStringFile(char * fname,string & data){
	FILE * file=NULL;
	file=fopen(fname,"r");
	if (file==NULL)
		return ERROR;

	u8 b[MAX_SIZE_STRING_FILES];
	u32 nread;
	nread = fread(b, 1, MAX_SIZE_STRING_FILES, file);
	fclose(file);
	if (nread==MAX_SIZE_STRING_FILES)
		return ERROR;

	if (nread==0)
		return ERROR;

	b[MAX_SIZE_STRING_FILES-1]=0;//protect
	data=(char*)b;

	return NO_ERROR;
}

void clear_stdin(char *str){
	for (u32 n=0;n<strlen(str);n++){
		if ((str[n]==0x0d)||(str[n]==0x0a))
			str[n]=0;
	}
}
int watchdog_stub(int argc, char **argv){
	//printf("watchdog_stub\n");
	u32 foundProc=SearchProcess(argv[0]);
	//printf("watchdog_stub [found %d] - [name %s]\n",foundProc,argv[0]);
	if (foundProc>1){
		printf("wdt already run [cnt %d]!!!\n",foundProc);
		return 0;
	}

	if (argc>1) {
		cnoda_setting=argv[1];
	}
	printf("cnoda_setting %s\n",cnoda_setting.c_str());
	Json::Reader jrd;
	string data;
	if (existsSync(cnoda_setting.c_str())==NO_ERROR){
		if (ReadStringFile((char*)cnoda_setting.c_str(),data)==NO_ERROR){

			if (jrd.parse(data,jscfg)!=0){
				if (jscfg.isMember("EnableWDT")){
					//"TimeoutWDT"
					EnableWDT=stoi(jscfg["EnableWDT"].asString());
					if (jscfg.isMember("TimeoutWDT")){
						TimeoutWDT=stoi(jscfg["TimeoutWDT"].asString());
					}

					if (jscfg.isMember("FifoFileWDT"))
						fifo_file=jscfg["FifoFileWDT"].asString();

					printf("EnableWDT %d TimeoutWDT %d WDT fifo %s\n",EnableWDT,TimeoutWDT,fifo_file.c_str());
					int ret=0;
					int delay_ms=100;
					u32 max_ctr=TimeoutWDT*1000/delay_ms;
					if (EnableWDT){
						pid_t pid=fork();
						if (pid==0){
							open_fifo();
							auto wdt=make_shared<watchdog>("/dev/watchdog",TimeoutWDT,EnableWDT,NORMAL_LEVEL);
							map<string,int> thr_map;
							bool wdt_clear=false;
							bool stub_mode=false;

							while(1){
								ret=read(f, rbuf, FIFO_BUF_SIZE);

								if (ret>0)
									rbuf[ret]=0;
								clear_stdin((char*)rbuf);
								if (rbuf[0]=='-'){
									if (thr_map.find((char*)rbuf)==thr_map.end()){
										char * rm_obj=(char*)&rbuf[1];
										thr_map.erase(rm_obj);
										printf("remove %s\n",rm_obj);

									}
									memset(rbuf,0,FIFO_BUF_SIZE);
									continue;
								}
								if(strcmp((char*)rbuf,STUB_MAGIC_STRING)==0){
									stub_mode=true;
									printf("run stub mode\n");
									for(auto const& item:thr_map){
										thr_map[(char*)rbuf]=0;
									}
									memset(rbuf,0,FIFO_BUF_SIZE);
								}
								if (stub_mode){
									wdt->keepalive(TimeoutWDT);
									sleep(1);
									continue;
								}
								if (ret>0){
									wdt->keepalive(0);
									printf("keepalive[%s], ",rbuf);
									if (thr_map.find((char*)rbuf)==thr_map.end()){
										printf("insert[%s]\n",rbuf);
									}
									else{
										printf("reset[%s]\n",rbuf);
									}
									thr_map[(char*)rbuf]=0;
								}
								mdelay(delay_ms);

								wdt_clear=true;
								for(auto const& item:thr_map){
									 //std::cout << item.first << " " << item.second;
									if ((u32)item.second>max_ctr){
										wdt_clear=false;
										printf("object %s is frozen\n",item.first.c_str());
										//break;
									}
									thr_map[(char*)item.first.c_str()]++;
								 }

								if (wdt_clear){
									wdt->keepalive(TimeoutWDT);
								}
								else{
									printf("wdt skip keepalive\n");
									//sleep(1);
								}
								memset(rbuf,0,FIFO_BUF_SIZE);

							}
						}
					}
					else{
						pid_t pid=fork();
						if (pid==0){
							auto wdt=make_shared<watchdog>("/dev/watchdog",DEFAULT_WDT_TIMEOUT,1,NORMAL_LEVEL);
							wdt->debug_level=NORMAL_LEVEL;
							printf("EnableWDT==0, run stub mode\n");
							while(1) {

								wdt->keepalive(0);
								sleep(10);
							}
						}
					}
				}
			}
		}
	}
	//jscfg=jrd.Parse(cnoda_setting);
	//if (JSON_ParseFile(jscfg,cnoda_setting)==ERROR)
		//return 0;

	//JSON_ReadConfigField(jscfg,"EnableWDT",EnableWDT);


	return 0;
}


int main(int argc, char **argv){
	watchdog_stub(argc,argv);
	//CnT=make_shared<CnodaT>();
	//CnT->Init(argc,argv);
	//printf("watchdog_stub\n");
	//return __watchdog_stub(argc,argv);
}
