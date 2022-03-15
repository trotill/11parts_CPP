/*
 * algo_sqlite_cli.h
 *
 *  Created on: 6 апр. 2020 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_LOGGER_ALGO_SQLITE_CLI_H_
#define SRVIOT_SRC_ENGINE_LOGGER_ALGO_SQLITE_CLI_H_


#include "safe_logger_client.h"

#ifdef _SQLITE
#include <sqlite3.h>
#include "engine/proto/json_proto.h"

class algo_sqlite_cli: public safelogger_client {
	public:
	class sql_format{
		public:
		sql_format(GprintT * gp):gp(gp){
		}
		u32 status=0;
		string new_format="";
		GprintT * gp;
		u32 total_items=0;
		u64 u64result=0;
		u32 row=0;
		vector<string> col;
		vector<string> val;
		//rapidjson::Document & result_root;
		//Json::Value col_types;
		Json::Value formatJson;
		vector<string> colsTypeString;
	};
	algo_sqlite_cli(string format,string config,eDebugTp debug_lvl):safelogger_client(format,config,debug_lvl){

		SourceStr="sqlite_cli";

		std::ifstream config_doc(config.c_str(), std::ifstream::binary);

		config_doc >> config_js;
		JSON_ReadConfigField(config_js,"sqal_db_fname",sqal_db_fname);
		SourceStr=sqal_db_fname;
		ObjPref="slogger_cli";
		u32 val=debug_lvl;
		//JSON_ReadConfigField(config_js,"slogger_loglevel",val);
		debug_level=(eDebugTp)val;
		JSON_ReadConfigField(config_js,"sqal_base_path[0]",sqal_base_path[0]);
		sqal_db_path=sqal_base_path[0]+"/"+sqal_db_fname;

		GPRINT(NORMAL_LEVEL,"algo_sqlite_cli created for db %s\n",sqal_db_path.c_str());
		getColTypes();

	}

	eErrorTp make_selection(u64 ts_start_ms,u64 ts_stop_ms,u32 interval,rapidjson::Document & result_root){
		u64 interval_ms=(u64)interval*1000;

		if (interval_ms==0){
			return make_selection_by_interval(ts_start_ms,ts_stop_ms,selection_limit,result_root);
		}
		else{
			GPRINT(NORMAL_LEVEL,"selection step by step not supported, used selection by interval\n");
			return make_selection_by_interval(ts_start_ms,ts_stop_ms,selection_limit,result_root);
		}
	}

	eErrorTp make_selection_by_sql(string sql,string & json_string);
	private:
		eErrorTp opendb(void){
				if	(sqlite3_open(sqal_db_path.c_str(), &db)){
					GPRINT(NORMAL_LEVEL, "Error open DB %s, remove db file\n", sqlite3_errmsg(db));
					return ERROR;
				}
				else
					return NO_ERROR;
		}
		eErrorTp closedb(void){
			sqlite3_close(db);
			db=NULL;
			return NO_ERROR;
		}
		eErrorTp getColTypes(void);
		string GenRequest(string request){
			GPRINT(MEDIUM_LEVEL,"DB SQL[%s]\n",request.c_str());
			return request;
		}
		eErrorTp sq_exec_db( string req, sql_format * st,int (*callback)(void*,int,char**,char**)){
			//zr();
				if (db==0)
					return ERROR;
			//	zr();
				string request=GenRequest(req);
			//	zr();
				char *dberr = 0;
				int result;
				u8 counter=0;
				bool trig=false;
				do{
					//zr();
					//try {
						//Есть ошибка. При первой выгрузке, если БД пустая, программа вылитает в этом месте. Походу sqlite3_exec кривоват.
						result=sqlite3_exec(db, (char*)request.c_str(),callback,st,&dberr);
					//} catch (std::exception& ex) {
					//	return ERROR;
					//}
					//zr();
					if (result){
						//zr();
						if ((result==SQLITE_BUSY)||(result==SQLITE_LOCKED)||(result==SQLITE_PROTOCOL)){
							if (result==SQLITE_BUSY) GPRINT(NORMAL_LEVEL, "DB busy, try again %d\n",counter);
							if (result==SQLITE_LOCKED) GPRINT(NORMAL_LEVEL, "DB table in the database is locked, try again %d\n",counter);
							if (result==SQLITE_PROTOCOL) GPRINT(NORMAL_LEVEL, "DB database lock protocol error, try again %d\n",counter);
							mdelay(100);
							counter++;
						}
						else{
							//zr();
							GPRINT(NORMAL_LEVEL, "Error SQL: %s,err num %d\n", dberr,result);
							sqlite3_free(dberr);
							return ERROR;
						}
					}
					if (counter>=50){
						//zr();
						GPRINT(NORMAL_LEVEL, "Error DB, oops\n");
						return ERROR;
					}
					//zr();
				}while((result==SQLITE_BUSY)||(result==SQLITE_LOCKED)||(result==SQLITE_PROTOCOL));
				//zr();
				return NO_ERROR;
		}

		static const u8 total_storage=2;
		Json::Value config_js;
		u32 selection_limit=4096;
		eErrorTp fault=NO_ERROR;
		string dbpath;
		sqlite3 *db=NULL;
		string sqal_db_fname="slogger.sqlite";
		string sqal_base_path[2]={"/www/pages/sys/db","/www/pages/sys_ex/db"};
		string sqal_db_path="";
		Json::Value col_types;
		Json::Value formatJson;
		bool dbReadyForRead=false;
		eErrorTp make_selection_by_interval(u64 ts_start_ms,u64 ts_stop_ms,u32 limit,rapidjson::Document & result_root);
		eErrorTp make_selection_by_interval(u64 ts_start_ms,u64 ts_stop_ms,u32 limit,string & json_string);

};

#endif
#endif /* SRVIOT_SRC_ENGINE_LOGGER_ALGO_SQLITE_CLI_H_ */
