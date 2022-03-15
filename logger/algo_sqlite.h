/*
 * algo_sqlite.h
 *
 *  Created on: 6 апр. 2020 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_LOGGER_ALGO_SQLITE_H_
#define SRVIOT_SRC_ENGINE_LOGGER_ALGO_SQLITE_H_


#ifdef _SQLITE
#include <sqlite3.h>
#include "safe_logger_srvalgo.h"
#include "engine/algo/crc.h"
#include "engine/lib/11p_bin.h"
#include "engine/thread.h"
#include "engine/lib/11p_time.h"
//#include <sqlite/connection.hpp>
//#include <sqlite/execute.hpp>
//#include <sqlite/query.hpp>
#include <iostream>

class sql_format{
	public:
	sql_format(u32 status,GprintT * gp):status(status),gp(gp){
	}
	u32 status=0;
	string new_format="";
	GprintT * gp;
	u32 total_items=0;
	u64 u64result=0;
};



/*
 * sqal_db_fname - имя файла БД
 * slogger_loglevel - уровень лога
 * sqal_base_path[0] - путь для первой копии БД
 * sqal_base_path[1] - путь для второй копии БД
 * sqal_max_items - макс. возможное кол. значений в БД
 * sqal_remove_items - при достижении максимума или исчерпания свободного места, сколько удалить первых значений
 * sqal_sync_time - с какой частотой сбрасывать данные на диск
 * sqal_offset_time - с какой частотой синхронизировать вторую копию БД
 * sqal_allow_remove_if_low_space - разрешить удаление первых значений при достижении лимита
 * sqal_min_free_space_MB - лимит на мин. пространство
 * sqal_col_type - переопредел типов колонок
 */

class algo_sqlite: public algo_abstract,GprintT {
	public:
	class dbs{
			public:
			sqlite3 *db=NULL;
		};
	algo_sqlite(Json::Value & config);

	~algo_sqlite(){
		GPRINT(NORMAL_LEVEL,"algo:algo_sqlite is down\n");
		replication();
		sqlite3_close(db_sql_rsv.db);
		sqlite3_close(db_sql.db);
		sync();
	}

	eErrorTp buildlog(void);
	eErrorTp tick(void);
	eErrorTp save(u8 * buf);
	private:
	eErrorTp opendb(string path,dbs & db);
	eErrorTp backup(dbs & db_sql,dbs & db_sql_rsv);
	eErrorTp ExtractColumns(Json::Value & root, Json::Value & columns);
	eErrorTp SaveValues(Json::Value & val);
	eErrorTp ExtractValue(string & json, string & value,TIME_T & extract_tstamp,u64 & extract_ts64);
	eErrorTp CheckColumns(Json::Value & columns);
	eErrorTp CheckAndSaveFormat(Json::Value & root);
	eErrorTp RemoveFirstItems(void);
	eErrorTp Clean(void);
	eErrorTp FillTotalItems(void);
	u64 GetCountItems(dbs & db,eErrorTp & err);
	eErrorTp GetMinMax(dbs & db,u64 * min,u64 * max);
	string GenRequest(string request,u8 dbid){
		GPRINT(MEDIUM_LEVEL,"DB%d SQL[%s]\n",dbid,request.c_str());
		return request;
	}



	eErrorTp sq_exec_db(dbs & db, string req,sql_format * st,int (*callback)(void*,int,char**,char**)){
		if (db.db==0)
			return ERROR;
		u8 db_id=(db.db==db_sql.db)?0:1;
		string request=GenRequest(req,db_id);
		char *dberr = 0;
		int result;
		u8 counter=0;
		bool trig=false;
		do{
			result=sqlite3_exec(db.db, (char*)request.c_str(),callback,st,&dberr);
			if (result){
				if ((result==SQLITE_BUSY)||(result==SQLITE_LOCKED)||(result==SQLITE_PROTOCOL)){
					if (result==SQLITE_BUSY) GPRINT(NORMAL_LEVEL, "DB busy, try again %d\n",counter);
					if (result==SQLITE_LOCKED) GPRINT(NORMAL_LEVEL, "DB table in the database is locked, try again %d\n",counter);
					if (result==SQLITE_PROTOCOL) GPRINT(NORMAL_LEVEL, "DB database lock protocol error, try again %d\n",counter);
					mdelay(100);
					counter++;
				}
				else{
					GPRINT(NORMAL_LEVEL, "Error SQL: %s,err num %d\n", dberr,result);
					sqlite3_free(dberr);
					sqlite3_close(db.db);
					db.db=0;
					return ERROR;
				}
			}
			if (counter>=50){
				GPRINT(NORMAL_LEVEL, "Error DB, oops\n");
				return ERROR;
			}
		}while((result==SQLITE_BUSY)||(result==SQLITE_LOCKED)||(result==SQLITE_PROTOCOL));

		return NO_ERROR;
	}



	eErrorTp sq_exec(string req){
			return sq_exec_db(db_sql,req,0,0);
	}

	eErrorTp sq_exec_two_db(string req){
		if (sq_exec_db(db_sql,req,0,0)!=ERROR)
			return sq_exec_db(db_sql_rsv,req,0,0);
		else
			return ERROR;
	}

	eErrorTp sq_exec(string req,sql_format * st,int (*callback)(void*,int,char**,char**)){
		return sq_exec_db(db_sql,req,st,callback);
	}

	eErrorTp sq_exec_rep(string req){
		if (replic_size<max_replic_size){
			replic.push_back(req);
			replic_size+=req.size();
		}
		else{
			GPRINT(NORMAL_LEVEL,"Oversized replics %u<%u, force replications\n",replic_size,max_replic_size);
			replication();
		}
		return sq_exec(req);
	}
	eErrorTp replication(void){
		u32 rsz=replic.size();
		GPRINT(NORMAL_LEVEL,"replic size %u\n",replic.size());
		if (rsz==0)
			return NO_ERROR;

		GPRINT(NORMAL_LEVEL,"Start replication, total %d\n",replic.size());
		for (u32 n=0;n<rsz;n++){
			if (sq_exec_db(db_sql_rsv,replic[n],0,0)==ERROR){
				GPRINT(NORMAL_LEVEL,"error replication %d, db is corrupt\n",n);
				replic.clear();
				replic_size=0;
				//free db_sql_rsv, remove DB, full backup DB
				return ERROR;
			}
			else
				GPRINT(NORMAL_LEVEL,"replication %d\n",n);
		}
		GPRINT(NORMAL_LEVEL,"Replication finished!!!\n");
		replic.clear();
		replic_size=0;
		sync();
		return NO_ERROR;
	}

	string sqal_db_fname="slogger.sqlite";
	string sqal_base_path[2]={"/www/pages/sys/db","/www/pages/sys_ex/db"};
	string sqal_db_path="";
	string sqal_db_path_rsv="";
	u64 sqal_max_items=10000;
	u32 sqal_remove_items=500;
	u64 sqal_total_items=0;
	u32 sqal_sync_time=10;
	u32 sqal_offset_time=60;
	bool allow_remove_if_low_space=true;
	bool inited=false;
	vector<string> replic;
	u32 replic_size=0;
	u32 max_replic_size=1000000;
	u32 min_free_space_MB=100;
	dbs db_sql;
	dbs db_sql_rsv;
	sFolderInfo storage_info[2];
	Json::Value sqal_col_type;
	//string tmpdir="";
	//u32 old_year=0;
	bool format_table_found=false;
	bool do_clean=false;
	shared_ptr<RTC_Timer> syncDB_Timer;

};
#endif

#endif /* SRVIOT_SRC_ENGINE_LOGGER_ALGO_SQLITE_H_ */
