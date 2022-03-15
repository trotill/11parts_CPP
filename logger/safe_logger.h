/*
 * safe_logger.h
 *
 *  Created on: 16 нояб. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_LOGGER_SAFE_LOGGER_H_
#define SRC_ENGINE_LOGGER_SAFE_LOGGER_H_

#include "engine/logger/safe_logger_srvalgo.h"
#include "engine/lib/11p_time.h"
#include "engine/proto/json_proto.h"
#include "engine/proto/redis.h"
#include "engine/thread.h"

//#include <experimental/filesystem>

#define SLOGGER_READ_FIFO_BUF_SIZE 10000
#define SLOGGER_STYLED_DATE 0
#define SLOGGER_JSON_DATE 1
#define SL_ALGO_BIPHASIC_OFFSET 0//запись лога с отложенной дубликацией в другую папку и автоподстройкой времени синхронизации
#define SL_ALGO_DNK_OFFSET 1//запись лога с отложенной дубликацией в другую папку с автоподстройкой времени синхронизации, в формате DNK
#define SL_ALGO_SQLITE 2//запись лога с дубликацией в другую папку, в формате sqlite
#define LOG_TERM_STRING "_-_-#$^LTS_|]"


enum class sloggerChannelTypes:u8 {
	enuOverPipe=0,
	enuOverRedis=1
};

class safelogger_base:public virtual GprintT {
	public:
	safelogger_base(eDebugTp debug_lvl,string prefix){
		SourceStr="slogger";
		debug_level=debug_lvl;
		ObjPref=prefix;
	}
	string fifo_file="/run/slog";
};

class logger_pipe: public ThreadT {
	public:
	logger_pipe(eDebugTp debug_lvl,string & config,string prefix);
	//logger_pipe(eDebugTp debug_lvl,string fifo_file,string fifo_file_out);
	~logger_pipe();
	void closeall(void);
	eErrorTp create_in_fifo(void);
	eErrorTp ReadConfigConf(string conffile);
	eErrorTp init_redis_fifo(eDebugTp dlevel,string & config);
	eErrorTp create_out_fifo(void);
	eErrorTp set_add_time(bool add){
		add_time_to_wr=add;
		return NO_ERROR;
	}
	virtual eErrorTp Loop(void* thisPtr);
	u8 rbuf[SLOGGER_READ_FIFO_BUF_SIZE];
	string fifo_file="";
	string fifo_file_out="";
	bool add_time_to_wr=false;
	int fifo_out_fd=0;
	int f=0;
	Json::Value rootCfg;
#ifdef _HIREDIS
	shared_ptr <redis>redB;
#endif
	sloggerChannelTypes chType=sloggerChannelTypes::enuOverPipe;
	string redisListName;
};

class safelogger_server: public  safelogger_base, public  ThreadT {
	public:
	safelogger_server(bool create_pipe,string config,eDebugTp debug_lvl);
	eErrorTp buildlog(void);
	eErrorTp force_exit(void);
	eErrorTp restart(void);
	eErrorTp get();
	~safelogger_server ();
	eErrorTp ReadConfigConf(string conffile);

	shared_ptr<logger_pipe> Pipe;
	private:
	Json::Value root;
	algo_abstract * algo=NULL;
	u32 algo_type=SL_ALGO_BIPHASIC_OFFSET;
	u32 aal_max_blocks=10000;
	u32 aal_lines_in_block=100;
	u32 aal_sync_time=10;
	u32 aal_offset_time=1;
	u8 aal_save_style=AAL_SAVE_QUOTES_COMMA;
	string aal_path[2]={"/www/pages/sys/log","/www/pages/sys_ex/log"};
	string wwwtmpdir="/var/run/cache/";
	string wwwtmpfile="slog.log.gz";
	string log_out_fifo="";
	bool useTransport=false;
	bool add_time_to_wr=false;
};

#endif /* SRC_ENGINE_LOGGER_SAFE_LOGGER_H_ */
