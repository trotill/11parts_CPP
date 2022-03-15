/*
 * redis.h
 *
 *  Created on: 25 сент. 2019 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_PROTO_REDIS_H_
#define SRC_ENGINE_PROTO_REDIS_H_

#ifdef _HIREDIS
#include <hiredis/hiredis.h>
#include <engine/lib/11p_bin.h>
#include <engine/proto/json_proto.h>

//REDIS DEF PORT
class redis: public virtual GprintT {
	public:
	void creator(eDebugTp dlevel,char * ip,u16 port,u8 dbid){
		debug_level=dlevel;
		SourceStr="redis";
		ObjPref="client";
		dbidx=dbid;
		this->ip=ip;
		this->port=port;

		if (reconnect()==ERROR)
			GPRINT(NORMAL_LEVEL,"Error connect to redis DB ip %s port %d dbid %d\n",ip,port,dbid);
		else{
			 GPRINT(NORMAL_LEVEL,"Connect Redis DB with ip %s port %u, select DB id %u\n",ip,port,dbid);
			GPRINT(NORMAL_LEVEL,"\n------------------------------\nHei!!!! REDIS-SERVER dump for default all data, check /etc/redis.conf,\n	for disable dump (Persistence) change options save \"\"\n------------------------------\n");
		}

	}
	redis(eDebugTp dlevel,string & config){


		//u16 port;
		//u8 dbid=0;
		Json::Value config_js;
		std::ifstream config_doc(config.c_str(), std::ifstream::binary);
		config_doc >> config_js;

		JSON_ReadConfigField(config_js,"redis_dbid",dbidx);
		JSON_ReadConfigField(config_js,"redis_port",port);
		JSON_ReadConfigField(config_js,"redis_url",ip);

		creator(dlevel,(char*)ip.c_str(),port,dbidx);
	}
	redis(eDebugTp dlevel,char * ip,u16 port,u8 dbid){

		creator(dlevel,ip,port,dbid);
		// reply = redisCommand(context, "SET foo %b", value, (size_t) valuelen);
	}
	eErrorTp connect(void){
		if (redis_connect==false){
			struct timeval timeout = { 2, 500000 };
			c = redisConnectWithTimeout(ip.c_str(), port,timeout);
			redis_connect=true;
		}
		else
			redisReconnect(c);

		if (c == NULL || c->err) {
			if (c) {
				 GPRINT(NORMAL_LEVEL,"Error: %s\n", c->errstr);
			} else {
				 GPRINT(NORMAL_LEVEL,"Can't allocate redis context\n");
			}
			return ERROR;
		}
		else
		  inited=true;

		return NO_ERROR;
	}
	eErrorTp init(void){
		if (!inited)
			return ERROR;

		std::pair<eErrorTp, vector<string>> res;
		res=command("SELECT %d",dbidx);
		if (res.first==ERROR){
			GPRINT(NORMAL_LEVEL,"Reconnect error, not accept command SELECT %d",dbidx);
			return ERROR;
		}
		/*res=command("config set appendonly no");
		if (res.first==ERROR){
			GPRINT(NORMAL_LEVEL,"Reconnect error, not accept command config set appendonly no");
			return ERROR;
		}*/

		//command("config set save \"\"");
		return NO_ERROR;
	}
	eErrorTp reconnect(void){
		//redisReconnect(c);


		if (connect()==ERROR)
			return ERROR;
		if (init()==ERROR)
			return ERROR;
		//zr();
		//sleep(1);
		//mdelay(100);



		 //Disable save dump to disk
		// command("config set save %s","");
		return NO_ERROR;
	}
	eErrorTp cleardb(void){
		return command("FLUSHDB").first;
	}
	std::pair<eErrorTp, vector<string>> command(const char *format, ...){
		buffer tb(1000);
		va_list argptr;
		eErrorTp err=NO_ERROR;
		result.clear();

		if (inited==false) {
			return std::make_pair(ERROR, result);
		}

		va_start(argptr, format);
		 //vsnprintf((char*)tb.p(), 1000, format, argptr);
		// GPRINT(HARD_LEVEL,"[%d]CMD [%s]\n",dbidx,tb.p());
//(redisReply*)redisCommand(c,"SELECT 3");///
		redisReply *reply=(redisReply*)redisvCommand(c,format,argptr);//(redisReply*)redisvCommand(c, format, argptr);
		// redisReply *reply=(redisReply*)redisCommand(c,"LPUSH test %b","E F G H",7);
		if (reply==NULL){
			va_end(argptr);
			result.push_back("error request");
			//reconnect();
			GPRINT(NORMAL_LEVEL,"error request\n");
			return std::make_pair(ERROR, result);
		}

		GPRINT(HARD_LEVEL,"reply->type %d\n",reply->type);
		switch (reply->type) {
			case REDIS_REPLY_ARRAY:
				GPRINT(HARD_LEVEL,"[%d]REDIS_REPLY_ARRAY\n",dbidx);
				for (u32 j = 0; j < reply->elements; j++) {
					GPRINT(HARD_LEVEL,"[%d]%u) %s\n", dbidx,j, reply->element[j]->str);
					if (reply->element[j]->str!=NULL)
						result.push_back(reply->element[j]->str);
				}
				break;
			case REDIS_REPLY_STRING:
				GPRINT(HARD_LEVEL,"[%d]REDIS_REPLY_STRING[%s]\n",dbidx,reply->str);
				result.push_back(reply->str);
				break;
			case REDIS_REPLY_INTEGER:
				GPRINT(HARD_LEVEL,"[%d]REDIS_REPLY_INTEGER\n",dbidx);
				break;
			case REDIS_REPLY_NIL:
				GPRINT(HARD_LEVEL,"[%d]REDIS_REPLY_NIL\n",dbidx);
				result.push_back("");
				break;
			case REDIS_REPLY_STATUS:
				GPRINT(HARD_LEVEL,"[%d]REDIS_REPLY_STATUS[%s]\n",dbidx,reply->str);
				result.push_back(reply->str);
				break;
			case REDIS_REPLY_ERROR:
				GPRINT(NORMAL_LEVEL,"[%d]REDIS_REPLY_ERROR [%s]\n",dbidx,reply->str);
				result.push_back(reply->str);
				//reconnect();
				err=ERROR;
				break;
		}
		freeReplyObject(reply);
		va_end(argptr);
		return std::make_pair(err, result);
	}
	~redis(){
		if (c!=NULL)
			redisFree(c);
	}
	private:
	vector<string> result;
	redisContext *c=NULL;
	bool inited=false;
	bool redis_connect=false;
	u32 dbidx=0;
	string ip="127.0.0.1";
	u16 port=6379;


};

#endif /* SRC_ENGINE_PROTO_REDIS_H_ */
#endif
