/*
 * safe_logger.cxx
 *
 *  Created on: 14 янв. 2019 г.
 *      Author: root
 */

#include "safe_logger.h"
#include "algo_biphasic_offset.h"
#include "global.h"
#ifdef _HIREDIS
#include "dnk_biphasic_offset.h"
#endif
#ifdef _SQLITE
#include "algo_sqlite.h"
#endif

#if 0
logger_pipe::logger_pipe(eDebugTp debug_lvl,string fifo_file):ThreadT("logger_pipe",fifo_file,debug_lvl)
	{
		debug_level=debug_lvl;
		SourceStr="logger_pipe";
		ObjPref=fifo_file;
		this->fifo_file=fifo_file;
		create_in_fifo();
		//SendMcastFifoMessage(BufHandler * buf,sInterThrMsgHeader & Mheader);
	}
#endif
logger_pipe::logger_pipe(eDebugTp debug_lvl,string & config,string prefix):ThreadT("logger_pipe",prefix,debug_lvl)
	{
	//string fifo_file,string fifo_file_out
		ReadConfigConf(config);
		debug_level=debug_lvl;
		SourceStr="logger_pipe";
		ObjPref=basename(fifo_file.c_str());
		this->fifo_file=fifo_file;
		this->fifo_file_out=fifo_file_out;
		if (chType==sloggerChannelTypes::enuOverRedis){
#ifdef _HIREDIS
			init_redis_fifo(debug_lvl,config);
#endif
		}
		else
			create_in_fifo();
		if (this->fifo_file_out.size()!=0)
			create_out_fifo();
		//SendMcastFifoMessage(BufHandler * buf,sInterThrMsgHeader & Mheader);
	}

eErrorTp logger_pipe::ReadConfigConf(string conffile)
{

	std::ifstream config_doc(conffile.c_str(), std::ifstream::binary);
	config_doc >> rootCfg;
	GPRINT(NORMAL_LEVEL,"Read config %s\n",conffile.c_str());
	JSON_ReadConfigField(rootCfg,"fifo_file",fifo_file);

	JSON_ReadConfigField(rootCfg,"log_out_fifo",fifo_file_out);
	GPRINT(NORMAL_LEVEL,"Set log_out_fifo %s\n",fifo_file_out.c_str());

	string val;
	JSON_ReadConfigField(rootCfg,"slogger_channel",val);

	if (val=="redis"){
		chType=sloggerChannelTypes::enuOverRedis;
		JSON_ReadConfigField(rootCfg,"redis_list",redisListName);

	}
	else
		chType=sloggerChannelTypes::enuOverPipe;

	return NO_ERROR;
}

void logger_pipe::closeall(void){
	if (f!=0){
		GPRINT(NORMAL_LEVEL,"Close f\n");
		close(f);
		f=0;
	}
	if (fifo_out_fd!=0){
		GPRINT(NORMAL_LEVEL,"Close fifo_out_fd\n");
		close(fifo_out_fd);
		remove(fifo_file_out.c_str());
		fifo_out_fd=0;
	}
}
logger_pipe::~logger_pipe(){
	closeall();
	}

eErrorTp logger_pipe::create_out_fifo(void){

	if (fifo_file_out.length()==0)
		return ERROR;

	remove(fifo_file_out.c_str());
	mkfifo(fifo_file_out.c_str(), 0666);
	fifo_out_fd = open(fifo_file_out.c_str(), O_NONBLOCK | O_RDWR);
	GPRINT(NORMAL_LEVEL,"Init fifo_file_out [%s]\n",fifo_file_out.c_str());
	return NO_ERROR;
}

eErrorTp logger_pipe::create_in_fifo(void){
		if (fifo_file.length()!=0){
			remove(fifo_file.c_str());
			mkfifo(fifo_file.c_str(), 0666);
			f = open(fifo_file.c_str(), O_RDWR);
			GPRINT(NORMAL_LEVEL,"Init fifo_file [%s]\n",fifo_file.c_str());
		}
		else
		{
			GPRINT(NORMAL_LEVEL,"Init stdin grabber\n");
		}
		return NO_ERROR;

	}

#ifdef _HIREDIS
eErrorTp logger_pipe::init_redis_fifo(eDebugTp dlevel,string & config){
		redB=make_shared<redis>(dlevel,config);

		GPRINT(NORMAL_LEVEL,"Init redis grabber\n");

		return NO_ERROR;

	}
#endif
eErrorTp logger_pipe::Loop(void* thisPtr){

		//msg=srvmUNDEF;
		char buf[]="hello world";
		int ret=0;
		u32 fflen=fifo_file.length();
		string inp;
		string skip_symb="\n";
		bool exit_thread=false;
		u32 empty_cntr=0;
		//debug_level=FILE_LEVEL;
		std::pair<eErrorTp, vector<string>> res;
		while(1){
			//sleep(1);
			//printf("LOOP\n");
			//debug_level=FILE_LEVEL;
			//GPRINT(NORMAL_LEVEL,"test\n");
			//printf("eee\n");
			if (exit_thread)
				break;


			if (chType==sloggerChannelTypes::enuOverRedis){
#ifdef _HIREDIS
				res=redB->command("RPOP %s",redisListName.c_str());
				if (res.first==NO_ERROR){
					//printf("size %d %s\n",res.second.size(),res.second[0].c_str());
					if (res.second[0].size()!=0){
						//printf("got %s\n",res.second[0].c_str());
						strncpy(rbuf,(char*)res.second[0].c_str(),SLOGGER_READ_FIFO_BUF_SIZE);
						ret=res.second[0].size();
					}
					else
						sleep(1);
				}
				else
					sleep(1);
#endif
			}
			else{
				if (fflen!=0){
					ret=read(f, rbuf, SLOGGER_READ_FIFO_BUF_SIZE);
					if (ret>0){
						rbuf[ret]=0;
						//очистка переноса строки. Перенос строки будет добавлен далее.
						if ((rbuf[ret-2]=='\n')&&(rbuf[ret-1]==0)){
							rbuf[ret-2]=0;
						}
					}
					GPRINT(HARD_LEVEL,"fifo[%s]\n",rbuf);
				}
				else{
					std::getline(std::cin, inp);
					if (inp.size()==0){
						mdelay(100);
						continue;
					}
					GPRINT(HARD_LEVEL,"stdin[%s]\n",inp.c_str());
					strncpy((char*)rbuf,(const char*)inp.c_str(),(size_t)SLOGGER_READ_FIFO_BUF_SIZE);
					ret=inp.size();
				}
			}
			if (strcmp((char*)rbuf,LOG_TERM_STRING)==0){
				exit_thread=true;
				closeall();
			}

			//printf("fifo_file_out %s\n",fifo_file_out.c_str());
			if ((ret!=0)&&(fifo_file_out.length()!=0)&&(fifo_out_fd!=0))
			{
				//zr();
				string stbuf;
				if (add_time_to_wr){
					struct tm  times;
					stringstream strdate;
					GetDataTime(&times,NULL);
					TimeToString(times,strdate);
					stbuf=sf("%s %s\n",strdate.str().c_str(),rbuf);
				}
				else{
					stbuf=(char*)rbuf;
					stbuf+='\n';
					//stbuf=sf("%s\n",rbuf);
				}

				write(fifo_out_fd, stbuf.c_str(), stbuf.size()+1);
				//write(fifo_out_fd, skip_symb, sizeof(skip_symb));
			}

			if ((ret!=0)&&(ret!=-1)&&(ret!=SLOGGER_READ_FIFO_BUF_SIZE)){
				rbuf[ret]=0;
				//zr();
				//printf("rbuf [%s]\n",rbuf);
				SendSharedFifoMessage(srvmUNDEF,"sase", (u8*)rbuf,(u32)strlen((char*)rbuf));
				ret=0;
			}
			//sleep(1);
			//SendSharedFifoMessage(srvmUNDEF,"sase", (u8*)buf,(u32)strlen((char*)buf));

		}
		SendSharedFifoMessage(srvmUNDEF,"sase", (u8*)LOG_TERM_STRING,(u32)strlen(LOG_TERM_STRING)+1);
		GPRINT(NORMAL_LEVEL,"Exit thread loop\n");
		return NO_ERROR;
	}


safelogger_server::safelogger_server(bool useTransport,string config,eDebugTp debug_lvl):safelogger_base(debug_lvl,"server"),ThreadT("safelogger_server","sase",debug_lvl){
		this->useTransport=useTransport;
		if (SearchFile(config.c_str())==ERROR){
			GPRINT(NORMAL_LEVEL,"Not found config %s, fifo file %s\n",config.c_str(),fifo_file.c_str());
			algo = new algo_biphasic_offset(debug_lvl,aal_path,aal_max_blocks,aal_lines_in_block,10,1,AAL_SAVE_QUOTES_COMMA,wwwtmpdir);
		}
		else
			ReadConfigConf(config);

		if (useTransport){
			Pipe=make_shared<logger_pipe>(debug_level,config,fifo_file);
			Pipe->set_add_time(add_time_to_wr);
			Pipe->Run();
		}

		GPRINT(NORMAL_LEVEL,"Slogger server started\n");
		//logger_pipe lp(debug_lvl);

	};


eErrorTp safelogger_server::buildlog(void){

		algo->buildlog();
		return NO_ERROR;
	}

eErrorTp safelogger_server::restart(void){
	switch (algo_type){
		case SL_ALGO_SQLITE:{
			#ifdef _SQLITE
					if (algo!=NULL)
						delete algo;
					algo = new algo_sqlite(root);
			#endif
		}
	}
	return NO_ERROR;
}
eErrorTp safelogger_server::force_exit(void){
	if (algo!=NULL)
		delete algo;
	GPRINT(NORMAL_LEVEL,"Got exit sequence, force exit\n");
	Pipe->closeall();
	exit(1);
}
eErrorTp safelogger_server::get(){
	u32 offs;

		if (CnT->SWTerminateReq==1)
			return NO_ERROR;


		if (algo->tick()==ERROR){
			restart();
		}
		//return NO_ERROR;
		sInterThrMsgHeader Mheader;
		while (GetUcastFifoMessage(&FifoPreBuf,Mheader)!=ERROR)
		{
			offs=0;
			FifoPreBuf.buf[FifoPreBuf.base_len]=0;
			if (FifoPreBuf.base_len>=sizeof(LOG_TERM_STRING)){
				offs=FifoPreBuf.base_len-sizeof(LOG_TERM_STRING)+1;
				//printf("offs %d [%s][%s] stat %d\n",offs,&FifoPreBuf.buf[offs],FifoPreBuf.buf,strcmp((char*)&FifoPreBuf.buf[offs],LOG_TERM_STRING));
			}
			if (strcmp((char*)&FifoPreBuf.buf[offs],LOG_TERM_STRING)==0){
				//printf("LOG_TERM_STRING terminate\n");
				//CnT->SWTerminateReq=1;
				force_exit();
				//break;
			}
			else
				algo->save(FifoPreBuf.buf);
		}
		return NO_ERROR;
	}

safelogger_server::~safelogger_server (){

		if (algo!=NULL)
			delete algo;
		//if (useTransport){
			//Pipe->Stop();
		//}
		GPRINT(NORMAL_LEVEL,"Slogger server is down\n");
		//printf("Srever down\n");
	}

eErrorTp safelogger_server::ReadConfigConf(string conffile)
	{

		std::ifstream config_doc(conffile.c_str(), std::ifstream::binary);
		config_doc >> root;
		GPRINT(NORMAL_LEVEL,"Read config %s\n",conffile.c_str());
		JSON_ReadConfigField(root,"fifo_file",fifo_file);

		JSON_ReadConfigField(root,"algo_type",algo_type);
		GPRINT(NORMAL_LEVEL,"Set algo type %d\n",algo_type);

		JSON_ReadConfigField(root,"algo_type",algo_type);
		GPRINT(NORMAL_LEVEL,"Set algo type %d\n",algo_type);


		JSON_ReadConfigField(root,"log_out_fifo",log_out_fifo);
		GPRINT(NORMAL_LEVEL,"Set log_out_fifo %s\n",log_out_fifo.c_str());

		JSON_ReadConfigField(root,"wwwtmpdir",wwwtmpdir);//deprecated
		JSON_ReadConfigField(root,"buildlog_dir",wwwtmpdir);
		JSON_ReadConfigField(root,"buildlog_file",wwwtmpfile);


		//printf("algo_type %d\n",algo_type);
		switch (algo_type){
			case SL_ALGO_BIPHASIC_OFFSET:{
				//printf("SL_ALGO_BIPHASIC_OFFSET\n");
				u32 val=debug_level;
				JSON_ReadConfigField(root,"slogger_loglevel",val);
				debug_level=(eDebugTp)val;

				JSON_ReadConfigField(root,"aal_path[0]",aal_path[0]);
				JSON_ReadConfigField(root,"aal_path[1]",aal_path[1]);
				JSON_ReadConfigField(root,"aal_max_blocks",aal_max_blocks);
				JSON_ReadConfigField(root,"aal_lines_in_block",aal_lines_in_block);
				JSON_ReadConfigField(root,"aal_sync_time",aal_sync_time);
				JSON_ReadConfigField(root,"aal_offset_time",aal_offset_time);
				JSON_ReadConfigField(root,"aal_save_style",aal_save_style);
				GPRINT(NORMAL_LEVEL,"aal_path[0] %s,aal_path[1] %s, aal_max_blocks %d, aal_lines_in_block %d, aal_sync_time %d, aal_offset_time %d aal_save_style %d\n",
						aal_path[0].c_str(),aal_path[1].c_str(),aal_max_blocks,aal_lines_in_block,aal_sync_time,aal_offset_time,aal_save_style);

				algo = new algo_biphasic_offset(debug_level,aal_path,aal_max_blocks,aal_lines_in_block,aal_sync_time,aal_offset_time,aal_save_style,wwwtmpdir);

			}
			break;
			case SL_ALGO_DNK_OFFSET:{

				//printf("SL_ALGO_DNK_OFFSET\n");
#ifdef _HIREDIS
				algo = new dnk_biphasic_offset(root);
#endif
			}
			break;
			case SL_ALGO_SQLITE:{
#ifdef _SQLITE
				algo = new algo_sqlite(root);

#endif
			}
			break;
			default:{
				GPRINT(NORMAL_LEVEL,"Not found algo\n");
				return ERROR;
			}
		}

		algo->tmpdir=wwwtmpdir;
		algo->tmpfile=wwwtmpfile;
		string logprefix;
		if (JSON_ReadConfigField(root,"logprefix",logprefix)==NO_ERROR)
			algo->logprefix=logprefix;

		return NO_ERROR;
	}
