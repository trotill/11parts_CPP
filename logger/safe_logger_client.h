/*
 * safe_logger_client.h
 *
 *  Created on: 20 сент. 2019 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_LOGGER_SAFE_LOGGER_CLIENT_H_
#define SRC_ENGINE_LOGGER_SAFE_LOGGER_CLIENT_H_

#include "safe_logger.h"

class safelogger_client: public safelogger_base{
	public:
	eErrorTp to_log(u32 count,...);
	eErrorTp to_log(string str);
	eErrorTp to_log(u32 count,Json::Value & json,u8 prec=JSONCPP_DEFAULT_PRECISION);
	safelogger_client(string format,string config,eDebugTp debug_lvl);
	~safelogger_client();

	eErrorTp ReadConfigConf(string conffile);

	private:
		vector<string> formstr;
		bool atime=true;//add timestamp
		u8 atime_format=SLOGGER_STYLED_DATE;
		bool replace_quotes=false;//replace quotes
		u8 algo=0;
		char str_buf[SLOGGER_READ_FIFO_BUF_SIZE];
		int f=0;
		u32 slogger_skip_data=0;
		sloggerChannelTypes chType=sloggerChannelTypes::enuOverPipe;
#ifdef _HIREDIS
		shared_ptr <redis>redB;
#endif
		string redisListName="undefined";
		u32 slogger_skip_data_cntr=0;
		bool unblock=false;
};



#endif /* SRC_ENGINE_LOGGER_SAFE_LOGGER_CLIENT_H_ */
