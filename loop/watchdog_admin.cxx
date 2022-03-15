/*
 * factory_reset.cxx
 *
 *  Created on: 23 дек. 2018 г.
 *      Author: root
 */

#include "watchdog_admin.h"
#include "engine/periphery/watchdog.h"
#include "engine/lib/11p_process.h"

Watchdog_admin::Watchdog_admin(eDebugTp debug_level):ThreadT("Watch_adm","waad",debug_level){
		GPRINT(HARD_LEVEL,"Create Watchdog_admin\n");
		JSON_ReadConfigField(CnT->json_cfg,"waad_loopdelay",loopdelay);
		JSON_ReadConfigField(CnT->json_cfg,"waad_interval",WDTinterval);
		JSON_ReadConfigField(CnT->json_cfg,"EnableWDT",EnableWDT);
		JSON_ReadConfigField(CnT->json_cfg,"TimeoutWDT",TimeoutWDT);
		JSON_ReadConfigField(CnT->json_cfg,"TimeoutThreadWDT_InTick",thread_hung_max_tick);
		JSON_ReadConfigField(CnT->json_cfg,"FifoFileWDT",fifo_file);
		if (existsSync(fifo_file)==NO_ERROR){
			pipe_wdt=true;
			fifo_fd = open((char*)fifo_file.c_str(), O_RDWR);
		}
		//f = open(fifo_file.c_str(), O_NONBLOCK | O_RDWR);

		DrivesWDT.isArray();
		JSON_ReadConfigField(CnT->json_cfg,"DrivesWDT",DrivesWDT);
		if (DrivesWDT.size()!=0)
			CheckDrive=true;
		//thread_hung_max_tick

		//
	}

Watchdog_admin::~Watchdog_admin (void){
		GPRINT(HARD_LEVEL,"Destroy Factory_reset\n");
	}

eErrorTp Watchdog_admin::fs_check(){
	ifstream f;
	eErrorTp err=NO_ERROR;
	if (CheckDrive){
		for (u8 n=0;n<DrivesWDT.size();n++){
			char *  drv=(char*)DrivesWDT[n].asCString();
			f.open(drv);

			if (f.good()){
				GPRINT(HARD_LEVEL,"%s is ok\n",drv);
				f.close();
			}
			else{
				GPRINT(NORMAL_LEVEL,"%s is error\n",drv);
				err=ERROR;
			}
		}
	}
	return err;
}
eErrorTp Watchdog_admin::Loop(void* thisPtr){
		//sSaved OldSaved;

			u32 lvl=NORMAL_LEVEL;
			JSON_ReadConfigField(CnT->json_cfg,"watchdog_loglevel",lvl);

			auto wdt=make_shared<watchdog>("/dev/watchdog",TimeoutWDT,EnableWDT,(eDebugTp)lvl);
			u32 set_wdt_cntr=0;
			vector<wadtchdog_thread> thread;
			bool unblock_wdt=true;
			//auto thr_iter = thread.begin();
			while(skey){
				if (fs_check()==ERROR){
					GPRINT(NORMAL_LEVEL,"Drive is broken, force reboot!!\n");
					RebootSystem();
					unblock_wdt=false;
				}

				if (set_wdt_cntr>=WDTinterval)
				{
					set_wdt_cntr=0;
					if (unblock_wdt){
						if (pipe_wdt){
							write(fifo_fd,"cnoda",6);
						}
						else
							wdt->keepalive(0);
					}
					else{
						GPRINT(NORMAL_LEVEL,"Block WDT keepalive!!\n");
					}
				}
				else
					set_wdt_cntr++;

				//GPRINT(NORMAL_LEVEL,"LOOP\n");
				u32 counter=0;
				while (GetUcastFifoMessage(FifoPreBuf,Mheader)!=ERROR)
				{	counter++;
					//printf("Mheader.MsgType %d\n",Mheader.MsgType);
					//printhex(FifoPreBuf.buf_base,FifoPreBuf.base_len,16);
					switch(Mheader.MsgType)
					{
						case srvmJNODA_READY:
								jnoda_ready=true;
						break;
						case srvmWDT:{
							if (FifoPreBuf.size()==0){
								GPRINT(NORMAL_LEVEL,"Error wdt format, set thread name\n");
								break;
							}
							//printf("Mheader.MsgType2 %d\n",Mheader.MsgType);
						//	FifoPreBuf.buf[FifoPreBuf.len+1]=0;
							auto thr_iter=thread.begin();
							bool trg=false;
							while(thr_iter!=thread.end()){
								string tn=(char*)FifoPreBuf.p();
								if (thr_iter->thr_name==tn){
									trg=true;
									thr_iter->source_tick=0;
									GPRINT(HARD_LEVEL,"Clear WDT thread %s\n",FifoPreBuf.p());
									break;
								}
								++thr_iter;
							}
							if (trg==false)
							{
								thread.emplace_back(FifoPreBuf.p(),thread_hung_max_tick);
								GPRINT(NORMAL_LEVEL,"Add WDT thread %s\n",FifoPreBuf.p());
							}
						}
						break;
						default:
							GPRINT(MEDIUM_LEVEL,"undef msg type [%d]\n",Mheader.MsgType);
					}
				}

				auto thr_iter=thread.begin();

				while(thr_iter!=thread.end()){
					//if (thr_iter->source_tick>1)
					//printf("Thr %s %d/%d\n",thr_iter->thr_name.c_str(),thr_iter->source_tick,thr_iter->max_tick);
					if (thr_iter->source_tick>=thr_iter->max_tick){
						printf("Thread %s is hung, force exit!!\n",thr_iter->thr_name.c_str());
						sleep(1);
						exit(1);
					}
					thr_iter->source_tick++;
					++thr_iter;

				}

				if (TERMReq){
					break;
				}
				mdelay(loopdelay);
			}
			if (pipe_wdt){
				write(fifo_fd,"-cnoda",6);
			}
			return NO_ERROR;
		}
