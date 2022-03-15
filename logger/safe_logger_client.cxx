/*
 * safe_logger_client.cxx
 *
 *  Created on: 20 сент. 2019 г.
 *      Author: root
 */
#include "safe_logger_client.h"
#include "algo_biphasic_offset.h"
#include "dnk_biphasic_offset.h"

eErrorTp safelogger_client::to_log(u32 count,Json::Value & json,u8 prec=JSONCPP_DEFAULT_PRECISION){
	//Json::FastWriter wr;

	return safelogger_client::to_log(count,FastWriteJSON(json).c_str());
}
eErrorTp safelogger_client::to_log(string str){
	return safelogger_client::to_log(1,str.c_str());
}
eErrorTp safelogger_client::to_log(u32 count,...){
		va_list args;

		int result = 0;
		if (!unblock)
			return ERROR;

		if (slogger_skip_data_cntr<slogger_skip_data){
			slogger_skip_data_cntr++;
			return NO_ERROR;
		}

		slogger_skip_data_cntr=0;
		stringstream ss;
		u32 fsize=formstr.size();
		va_start(args, count);
		struct tm times;
		struct timeval tv;
		if (chType==sloggerChannelTypes::enuOverPipe){
			if (f<=0){
				if ((f = open(fifo_file.c_str(), O_WRONLY|O_NONBLOCK))<0){
					GPRINT(NORMAL_LEVEL,"Error open %s\n",fifo_file.c_str());
					return ERROR;
				}
			}
		}
		//ss<<',"';
		//printf("atime %d atime_format %d\n",atime,atime_format);
		GetDataTime(&times,&tv);
		if (atime==true){
			switch (atime_format){
				case SLOGGER_STYLED_DATE:{
					//printf("SLOGGER_STYLED_DATE\n");

					string  strdate=string_format("%04d-%02d-%02d %02d:%02d:%02d ",(times.tm_year+1900),(times.tm_mon+1),times.tm_mday,
							times.tm_hour,times.tm_min,times.tm_sec);
					ss << strdate;
				}
				break;
				case SLOGGER_JSON_DATE:
				{
					//printf("SLOGGER_JSON_DATE\n");
					u64 ts_msec=(u64)((u64)tv.tv_sec*1000)+(u64)((u64)tv.tv_usec/1000);
					//printf("tv sec %llu  usec %llu calc %llu\n",(long long int)(tv.tv_sec*1000),(long long int)(tv.tv_usec)/1000,(long long int)(time(NULL)));
					ss << "{\"m\":"<< ts_msec << ",\"t\":"<<tv.tv_sec<<",\"d\":";
				}
				break;
				default:
					//printf("default\n");
					break;
			}
		}

		for (u32 i = 0; i < count; ++i) {
				if (i<fsize){
					ss << formstr[i] << va_arg(args, char *);
				}
				//result += va_arg(args, int);
		}
		if ((atime)&&(atime_format==SLOGGER_JSON_DATE)){
			ss << "}";
		}

		//ss<<'"';
		//if (add_line_break) ss<<line_break;
		//printf("LOG:[%s]\n",ss.str().c_str());
		//strncpy(str_buf,srst.c_str(),SLOGGER_READ_FIFO_BUF_SIZE);
		//ss << endl;


		if (chType==sloggerChannelTypes::enuOverPipe){
			int ret=write(f,ss.str().c_str(),ss.str().size()+1);
			fd_set rfds;
			FD_ZERO(&rfds);
			FD_SET(0, &rfds);
			/* Wait up to 10 mS. */
			tv.tv_sec = 0;
			tv.tv_usec = 10000;
			select(f, &rfds, NULL, NULL, &tv);

			int ret2=0;
			if (ret==-1){
				GPRINT(NORMAL_LEVEL,"Error write to %s\n",fifo_file.c_str());
				if (f!=0)
					close(f);
				f = open(fifo_file.c_str(), O_WRONLY|O_NONBLOCK);
				if (f<=0){
					GPRINT(NORMAL_LEVEL,"Error reopen %s\n",fifo_file.c_str());
					return ERROR;
				}
				ret2=write(f,ss.str().c_str(),ss.str().size()+1);
			}


			if ((ret==-1)&&(ret2==-1)){
				GPRINT(NORMAL_LEVEL,"Error write to safe logger %s\n",fifo_file.c_str());
				return ERROR;
			}

			return NO_ERROR;
		}

#ifdef _HIREDIS
		if (chType==sloggerChannelTypes::enuOverRedis){
			std::pair<eErrorTp, vector<string>> res;
			//printf("send %s\n",ss.str().c_str());
			res=redB->command("LPUSH %s %s",redisListName.c_str(),(char*)ss.str().c_str());
			if (res.first==ERROR){
				return ERROR;
			}
			else
				return NO_ERROR;
		}
#endif
		return ERROR;
	}
safelogger_client::safelogger_client(string format,string config,eDebugTp debug_lvl):safelogger_base(debug_lvl,"client")
	{
		//time:$ arg1:$ arg2:$
		if (SearchFile(config.c_str())==ERROR)
		{
			GPRINT(NORMAL_LEVEL,"Not found config %s, fifo_file file %s\n",config.c_str(),fifo_file.c_str());

			return;
		}
		else
			ReadConfigConf(config);

		unblock=true;
		u32 fsize=format.size();
		u32 offs=0;
		string str;

		for (u32 n=0;n<fsize;n++){
			if (format[n]=='$'){
				format[n]=0;
				str=&format.c_str()[offs];
				formstr.emplace_back(str);
				offs=n+1;
			}

		}

		if (chType==sloggerChannelTypes::enuOverPipe){
			f = open(fifo_file.c_str(), O_WRONLY|O_NONBLOCK);
			GPRINT(NORMAL_LEVEL,"Open %s\n",fifo_file.c_str());
		}
#ifdef _HIREDIS
		if (chType==sloggerChannelTypes::enuOverRedis){
			redB=make_shared<redis>(debug_lvl,config);
			GPRINT(NORMAL_LEVEL,"Open redis DB\n");
		}
#endif

	}

safelogger_client::~safelogger_client(){
	if (chType==sloggerChannelTypes::enuOverPipe){
		if (f!=0){
			close(f);
			GPRINT(NORMAL_LEVEL,"Close %s\n",fifo_file.c_str());
		}
	}
}

eErrorTp safelogger_client::ReadConfigConf(string conffile)
{
	Json::Value root;
	std::ifstream config_doc(conffile.c_str(), std::ifstream::binary);
	config_doc >> root;

	JSON_ReadConfigField(root,"fifo_file",fifo_file);
	JSON_ReadConfigField(root,"slogger_atime",atime);
	JSON_ReadConfigField(root,"slogger_atime_format",atime_format);
	JSON_ReadConfigField(root,"slogger_skip_data",slogger_skip_data);
	string val;
	JSON_ReadConfigField(root,"slogger_channel",val);

	if (val=="redis"){
		chType=sloggerChannelTypes::enuOverRedis;
		JSON_ReadConfigField(root,"redis_list",redisListName);

	}
	else
		chType=sloggerChannelTypes::enuOverPipe;


	return NO_ERROR;
}

