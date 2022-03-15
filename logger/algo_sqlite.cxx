/*
 * algo_sqlite.cxx
 *
 *  Created on: 6 апр. 2020 г.
 *      Author: root
 */




#ifdef _SQLITE
#include "engine/logger/algo_sqlite.h"
#include "engine/proto/json_proto.h"
#include "engine/lib/11p_process.h"

#if 1
eErrorTp algo_sqlite::backup(dbs & db_sql,dbs & db_sql_rsv){
	sqlite3_backup* pBackup;
	GPRINT(NORMAL_LEVEL,"Run backup\n");
	pBackup=sqlite3_backup_init(db_sql_rsv.db,"main",db_sql.db,"main");
	//zr();
	if (pBackup == 0)
	{
		GPRINT(NORMAL_LEVEL,"Backup Init Error:%s\n", sqlite3_errmsg(db_sql_rsv.db));
	    sqlite3_close(db_sql_rsv.db);
	    sqlite3_close(db_sql.db);
	    db_sql_rsv.db=0;
	    db_sql.db=0;
	    return ERROR;
	}
	int rc;
	//zr();
	do
	{
		//zr();
		rc = sqlite3_backup_step(pBackup,1000);
		//zr();
		if (rc == SQLITE_BUSY || rc == SQLITE_LOCKED)
		{
		  sqlite3_sleep(10);
		}

		GPRINT(NORMAL_LEVEL,"Backup remaining %d rc %d\n", sqlite3_backup_remaining(pBackup),rc);
	}
	while (rc == SQLITE_OK || rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
	//zr();
	sqlite3_backup_finish(pBackup);
	if (rc == SQLITE_DONE){
		GPRINT(NORMAL_LEVEL,"Backup finish\n");
		return NO_ERROR;
	}
	else{
		GPRINT(NORMAL_LEVEL,"Backup Step Error:%s\n", sqlite3_errmsg(db_sql_rsv.db));
		sqlite3_close(db_sql_rsv.db);
		sqlite3_close(db_sql.db);
	    db_sql_rsv.db=0;
	    db_sql.db=0;
		return ERROR;
	}
}
#endif

eErrorTp algo_sqlite::opendb(string path,dbs & db){
		GPRINT(NORMAL_LEVEL,"Try open DB %s\n",path.c_str());
		if	(sqlite3_open(path.c_str(), &db.db)){
			GPRINT(NORMAL_LEVEL, "Error open DB %s, remove db file\n", sqlite3_errmsg(db.db));
			db.db=0;
			remove(path.c_str());
			GPRINT(NORMAL_LEVEL,"Second try open DB %s\n",path.c_str());
			if	(sqlite3_open(path.c_str(), &db.db)){
				GPRINT(NORMAL_LEVEL, "Error open DB %s\n", sqlite3_errmsg(db.db));
				db.db=0;
				return ERROR;
			}
			else{
				GPRINT(NORMAL_LEVEL,"DB opened (try 2)\n");
			 return NO_ERROR;
			}
		}
		else{
			GPRINT(NORMAL_LEVEL,"DB opened (try 1)\n");
			return NO_ERROR;
		}

}
algo_sqlite::algo_sqlite(Json::Value & config){
		char *dberr = 0;
		eErrorTp err;
		u64 amin=0,amax=0;
		u64 bmin=0,bmax=0;

		u32 val=(u32)NORMAL_LEVEL;
		JSON_ReadConfigField(config,"sqal_db_fname",sqal_db_fname);
		SourceStr=sqal_db_fname;
		ObjPref="slogger";
		JSON_ReadConfigField(config,"slogger_loglevel",val);
		debug_level=(eDebugTp)val;
		JSON_ReadConfigField(config,"sqal_base_path[0]",sqal_base_path[0]);
		JSON_ReadConfigField(config,"sqal_base_path[1]",sqal_base_path[1]);
		JSON_ReadConfigField(config,"sqal_max_items",sqal_max_items);
		JSON_ReadConfigField(config,"sqal_remove_items",sqal_remove_items);
		sqal_db_path=sqal_base_path[0]+"/"+sqal_db_fname;
		sqal_db_path_rsv=sqal_base_path[1]+"/"+sqal_db_fname;
		JSON_ReadConfigField(config,"sqal_sync_time",(u32&)sqal_sync_time);
		JSON_ReadConfigField(config,"sqal_offset_time",(u32&)sqal_offset_time);
		JSON_ReadConfigField(config,"sqal_col_type",sqal_col_type);

		syncDB_Timer=make_shared<RTC_Timer>(sqal_offset_time*1000);
		//allow_remove_if_low_space
		JSON_ReadConfigField(config,"sqal_allow_remove_if_low_space",allow_remove_if_low_space);
		if (allow_remove_if_low_space)
			GPRINT(NORMAL_LEVEL,"Allow remove oldest files if detect low space\n");

		JSON_ReadConfigField(config,"sqal_min_free_space_MB",(u32&)min_free_space_MB);

		GPRINT(NORMAL_LEVEL,"sqal_path[0] %s,sqal_path[1] %s, sqal_max_items %d,  sqal_sync_time %d, sqal_offset_time %d\n",
				sqal_base_path[0].c_str(),sqal_base_path[1].c_str(),sqal_max_items,sqal_sync_time,sqal_offset_time);

		GPRINT(NORMAL_LEVEL,"algo_sqlite log_level %d\n",debug_level);

		MkPath(sqal_base_path[0].c_str(), 0xffffffff);
		MkPath(sqal_base_path[1].c_str(), 0xffffffff);

		GPRINT(NORMAL_LEVEL,"algo:algo_sqlite started, open db %s\n",sqal_db_path.c_str());
		GPRINT(NORMAL_LEVEL,"algo:algo_sqlite started, open db %s\n",sqal_db_path_rsv.c_str());
		//check DB0 and DB1
		//sync DBs
		//const char* SQL = "CREATE TABLE IF NOT EXISTS foo(a,b,c); INSERT INTO FOO VALUES(1,2,3); INSERT INTO FOO SELECT * FROM FOO;";

		auto check_db=[&](string & path,dbs & db){
			opendb(path,db);
			if (sq_exec_db(db,sf("CREATE TABLE IF NOT EXISTS '%s.params'(name,value)",sqal_db_fname.c_str()),0,0)==ERROR){
				GPRINT(NORMAL_LEVEL,"Error create %s\n",sqal_db_fname.c_str());
				remove(path.c_str());
				opendb(path,db);
				if (sq_exec_db(db,sf("CREATE TABLE IF NOT EXISTS '%s.params'(name,value)",sqal_db_fname.c_str()),0,0)==ERROR){
					GPRINT(NORMAL_LEVEL,"Check file system, fatal error, all DB stop, %s\n",path.c_str());
					return (eErrorTp)ERROR;
				}
			}
			GPRINT(NORMAL_LEVEL,"DB %s ok!!!\n",sqal_db_fname.c_str());
			return (eErrorTp)NO_ERROR;
		};

		if (check_db(sqal_db_path,db_sql)==ERROR)
			return;

		if (check_db(sqal_db_path_rsv,db_sql_rsv)==ERROR)
			return;

		GPRINT(NORMAL_LEVEL,"1 db_sql.db db [%d] db_sql_rsv.db [%d]\n",db_sql.db,db_sql_rsv.db);

		if (GetMinMax(db_sql,&amin,&amax)==ERROR){
			GPRINT(NORMAL_LEVEL,"Error get DB main min/max\n");
			if (opendb(sqal_db_path,db_sql)==ERROR) {
				GPRINT(NORMAL_LEVEL,"Check file system, fatal error, all DB stop, m2\n");
				return;
			}
		}
		else
			GPRINT(NORMAL_LEVEL,"DB main min %llu max %llu\n",amin,amax);

		if (GetMinMax(db_sql_rsv,&bmin,&bmax)==ERROR){
			GPRINT(NORMAL_LEVEL,"Error get DB rsv main min/max\n");
			if (opendb(sqal_db_path_rsv,db_sql_rsv)==ERROR){
				GPRINT(NORMAL_LEVEL,"Check file system, fatal error, all DB stop, m3\n");
				return;
			}
		}
		else{
			GPRINT(NORMAL_LEVEL,"DB reserved min %llu max %llu\n",bmin,bmax);
		}

		if (amax>bmax){
			GPRINT(NORMAL_LEVEL,"Try backup DB->DB rsv\n");
			if (backup(db_sql,db_sql_rsv)==ERROR){
				GPRINT(NORMAL_LEVEL,"Error backup DB->DB rsv\n");
				GPRINT(NORMAL_LEVEL,"Check file system, fatal error, all DB stop, remove %s\n",sqal_db_path_rsv.c_str());
				remove(sqal_db_path_rsv.c_str());
			}
		}
		else{
			if (amax==bmax){
				if (amax==0)
					GPRINT(NORMAL_LEVEL,"Create new clean DB\n");

				GPRINT(NORMAL_LEVEL,"DBs identical, not req sync\n");
			}
			else{
				GPRINT(NORMAL_LEVEL,"Try backup from reserved DB rsv->DB\n");
				if (backup(db_sql_rsv,db_sql)==ERROR){
					GPRINT(NORMAL_LEVEL,"Error backup DB rsv->DB\n");
					GPRINT(NORMAL_LEVEL,"Check file system, fatal error, all DB stop, remove %s\n",sqal_db_path.c_str());
					remove(sqal_db_path.c_str());
				}
			}
		}

		GPRINT(NORMAL_LEVEL,"2 db_sql.db db [%d] db_sql_rsv.db [%d]\n",db_sql.db,db_sql_rsv.db);

		if (amax!=0){
			Clean();
			FillTotalItems();
		}
		GPRINT(NORMAL_LEVEL,"Exit constructor db_sql.db db [%d] db_sql_rsv.db [%d]\n",db_sql.db,db_sql_rsv.db);
}

eErrorTp ExtractValueRecurs(Json::Value & root,Json::Value & arrvalue,u32 & nidx){
			//u32 nidx=1;
			for (const auto& key : root.getMemberNames()){
				if (root[key].isObject()){
					ExtractValueRecurs(root[key],arrvalue,nidx);
				}
				else{
					arrvalue[nidx]=root[key];
				}
				nidx++;
			}
			return NO_ERROR;
	}

eErrorTp ExtractFormatRecurs(Json::Value & root,Json::Value & arrvalue){
	u32 nidx=0;
	for (const auto& key : root.getMemberNames()){
		if (root[key].isObject()){
			ExtractFormatRecurs(root[key],arrvalue[nidx]);
		}
		else{
			arrvalue[nidx]=key;
		}
		nidx++;
	}
	return NO_ERROR;
}

eErrorTp algo_sqlite::ExtractColumns(Json::Value & root, Json::Value & columns){
	Json::Value rvalue;
	Json::Reader reader;

	//if (reader.parse(json,root)){
		//if ((root.isMember("t"))&&(root.isMember("d"))){
	Json::Value format;
	Json::Value value;

	ExtractFormatRecurs(root["d"],format);
	u32 nidx=0;
	ExtractValueRecurs(root["d"],value,nidx);
	for (u32 n=0;n<format.size();n++){
		string sfrm="";

		if (sqal_col_type.isMember(format[n].asCString())){
			sfrm=sqal_col_type[format[n].asCString()].asString();
			GPRINT(NORMAL_LEVEL,"Overload format for col %s %s\n",format[n].asCString(),sfrm.c_str());
		}
		else
		if (value[n].isBool()){
			sfrm="NUMERIC";
		}
		else
		if (value[n].isInt()||value[n].isUInt()||value[n].isInt64()||value[n].isUInt64()){
			sfrm="INTEGER";
		}
		else
		if (value[n].isDouble()){
			sfrm="REAL";
		}
		else{
			sfrm="TEXT";
		}
		GPRINT(MEDIUM_LEVEL,"Col %s Val %s DetectType %s\n",format[n].asCString(),FastWriteJSON(value[n]).c_str(),sfrm.c_str());
		columns[format[n].asCString()]=sfrm;
	}
			//value=FastWriteJSON(rvalue);
		//}
	//}
	//else{
	//	GPRINT(NORMAL_LEVEL,"ExtractID_N_Value error json parse\n");
	//	return ERROR;
	///
	return NO_ERROR;
}

eErrorTp algo_sqlite::GetMinMax(dbs & db,u64 * min,u64 * max){
	sql_format sql_frm(0,this);
	eErrorTp err=ERROR;
	//SELECT * FROM 'slogger.sqlite.data' WHERE 1 LIMIT 10;
	sq_exec_db(db,sf("SELECT m FROM '%s.data' WHERE 1 LIMIT 1",sqal_db_fname.c_str()),&sql_frm,
				[](void *st, int argc, char **argv, char **azColName) {
					auto sql_frm_p=(sql_format*)st;
					if (argc==1){
						sql_frm_p->status=1;
						sql_frm_p->u64result=std::stoull(string(argv[0]));
					}
					//for (u8 n=0;n<argc;n++){
				//		printf("argv[%s]\n",argv[n]);
				//	}
					//if (argc!=0)

						//sql_frm_p->total_items=atol(argv[0]);
					return 0;
				});

	if (sql_frm.status){
		sql_frm.status=0;
		*min=sql_frm.u64result;
		sq_exec_db(db,sf("SELECT m FROM '%s.data' ORDER BY m DESC LIMIT 1",sqal_db_fname.c_str()),&sql_frm,
						[](void *st, int argc, char **argv, char **azColName) {
							auto sql_frm_p=(sql_format*)st;
							if (argc==1){
								sql_frm_p->status=1;
								sql_frm_p->u64result=std::stoull(string(argv[0]));
							}
							return 0;
						});
		if (sql_frm.status){
			*max=sql_frm.u64result;
			return NO_ERROR;
		}
	}
	else{
		GPRINT(MEDIUM_LEVEL,"error get count min/max items\n");
	}

	return ERROR;
}

u64 algo_sqlite::GetCountItems(dbs & db,eErrorTp & err){
	sql_format sql_frm(0,this);
	err=ERROR;
	sq_exec_db(db,sf("SELECT COUNT (*) FROM '%s.data'",sqal_db_fname.c_str()),&sql_frm,
				[](void *st, int argc, char **argv, char **azColName) {
					auto sql_frm_p=(sql_format*)st;
					sql_frm_p->status=1;
					if (argc!=0)
						sql_frm_p->total_items=atol(argv[0]);
					return 0;
				}
			);
	if (sql_frm.status){
		err=NO_ERROR;
		GPRINT(MEDIUM_LEVEL,"sqal_total_items %d\n",sqal_total_items);
		return sql_frm.total_items;
	}
	else{
		GPRINT(MEDIUM_LEVEL,"error get count items\n");
		return 0;
	}
}
eErrorTp algo_sqlite::FillTotalItems(void)
{
	eErrorTp err;
	sqal_total_items=GetCountItems(db_sql,err);
	return err;
	//SELECT COUNT (*) FROM 'slogger.sqlite.data' ;
}

eErrorTp algo_sqlite::CheckColumns(Json::Value & columns)
{
	//https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
	char *dberr = 0;
	string col="m INTEGER, t INTEGER,";
	string types="[\"INTEGER\",\"INTEGER\",";
	string formatJson=FastWriteJSON(columns);
	for (const auto& key : columns.getMemberNames()){
		col+=sf("\"%s\" %s,",key.c_str(),columns[key].asCString());
		types+=sf("\"%s\",",columns[key].asCString());
	}
	//for (u32 n=0;n<columns.size();n++){
	//	col+=sf("\"%s\",",columns[n].asCString());
	//}
	col.pop_back();
	types.pop_back();
	types+="]";
	sq_exec_two_db(sf("CREATE TABLE IF NOT EXISTS '%s.data'(%s)",sqal_db_fname.c_str(),col.c_str()));

	sql_format sql_frm(0,this);
	sql_frm.new_format=col;

	sq_exec(sf("SELECT name, value FROM '%s.params' WHERE name='format'",sqal_db_fname.c_str()),&sql_frm,
		[](void *st, int argc, char **argv, char **azColName) {

			auto sql_frm_p=(sql_format*)st;
			sql_frm_p->status=1;
			string src_format;
			for (int n=0;n<argc;n++){
				//printf("n %d col %s val %s\n",n,azColName[n],argv[n]);
				if (strcmp(azColName[n],"value")==0){
					src_format=argv[n];
				}
			}
			if (src_format!=sql_frm_p->new_format){
				sql_frm_p->gp->GPRINT(NORMAL_LEVEL,"Warning diff format!!!\n***old [%s]\n***new[%s]\n",src_format.c_str(),sql_frm_p->new_format.c_str());
				sql_frm_p->status=2;
			}
			return 0;
		}
	);

	if (sql_frm.status==1){
		FillTotalItems();
		return NO_ERROR;
	}


	TIME_T t=TIME((u32*)NULL);
	if (sql_frm.status==0){
		try{//formatJson
			if (sq_exec_two_db(sf("INSERT INTO '%s.params' VALUES('formatJson','%s')",sqal_db_fname.c_str(),formatJson.c_str()))==ERROR)
				throw ERROR;
			if (sq_exec_two_db(sf("INSERT INTO '%s.params' VALUES('format','%s')",sqal_db_fname.c_str(),col.c_str()))==ERROR)
				throw ERROR;
			if (sq_exec_two_db(sf("INSERT INTO '%s.params' VALUES('types','%s')",sqal_db_fname.c_str(),types.c_str()))==ERROR)
				throw ERROR;
			if (sq_exec_two_db(sf("INSERT INTO '%s.params' VALUES('tab_data_init_ts',%u)",sqal_db_fname.c_str(),(u32)t))==ERROR)
				throw ERROR;
		}catch(eErrorTp e){
			return ERROR;
		}
	}

	if (sql_frm.status==2)
	{
		try{

			if (sq_exec_two_db(sf("DROP TABLE '%s.data'",sqal_db_fname.c_str()))==ERROR)
				throw ERROR;
			if (sq_exec_two_db(sf("CREATE TABLE IF NOT EXISTS '%s.data'(%s)",sqal_db_fname.c_str(),sql_frm.new_format.c_str()))==ERROR)
				throw ERROR;
			if (sq_exec_two_db(sf("UPDATE '%s.params' SET name='formatJson',value='%s' WHERE name='formatJson'",sqal_db_fname.c_str(),formatJson.c_str())))
				throw ERROR;
			if (sq_exec_two_db(sf("UPDATE '%s.params' SET name='format',value='%s' WHERE name='format'",sqal_db_fname.c_str(),sql_frm.new_format.c_str())))
				throw ERROR;
			if (sq_exec_two_db(sf("UPDATE '%s.params' SET name='types',value='%s' WHERE name='types'",sqal_db_fname.c_str(),types.c_str())))
					throw ERROR;
			if (sq_exec_two_db(sf("INSERT INTO '%s.params' VALUES('tab_data_recreate_ts',%u)",sqal_db_fname.c_str(),(u32)t))==ERROR)
				throw ERROR;
			sqal_total_items=0;
		}
		catch (eErrorTp e){
			return ERROR;
		}
		GPRINT(NORMAL_LEVEL,"All values delete, format updated");
	}
	return NO_ERROR;
}

eErrorTp algo_sqlite::CheckAndSaveFormat(Json::Value & root){
		//struct tm * ptm=GMTIME(&t);
		//u32 year=ptm->tm_year+1900;
		Json::Value format;
		//if (year!=old_year)
			//format_table_found=false;


		if ((format_table_found==false)){
			if (ExtractColumns(root, format)==NO_ERROR){
				GPRINT(NORMAL_LEVEL,"Extract format %s\n",StyledWriteJSON(format).c_str());
				CheckColumns(format);
				format_table_found=true;
			}
			else
				return ERROR;
		}
		//old_year=year;
		return NO_ERROR;
	}

eErrorTp algo_sqlite::RemoveFirstItems(void){
	 eErrorTp err=ERROR;
	 if (sq_exec_rep(sf("DELETE FROM '%s.data' WHERE m IN (SELECT m FROM '%s.data' limit %d)",sqal_db_fname.c_str(),sqal_db_fname.c_str(),sqal_remove_items))!=ERROR){
		 GPRINT(NORMAL_LEVEL,"Items removed, do vacuum\n");
		 err=sq_exec_rep("vacuum;");
		 sync();
		 return err;
	 }
	 else{
		 GPRINT(NORMAL_LEVEL,"Error remove items\n");
		 return err;
	 }
}

eErrorTp algo_sqlite::Clean(void){
	FillTotalItems();
	while(1){
			if (allow_remove_if_low_space){
				for (u8 n=0;n<2;n++){
					GetFolderInfo(&storage_info[n],(char*)sqal_base_path[n].c_str());
					GPRINT(HARD_LEVEL,"stor %d free %u (MB) total %u (MB)\n",n,storage_info[n].FreeSizeMB,storage_info[n].TotalSizeMB);
					if ((storage_info[n].FreeSizeMB<min_free_space_MB)&&(sqal_total_items!=0)){
						GPRINT(NORMAL_LEVEL,"Clean DB%d, free %u min_free %u\n",n,storage_info[n].FreeSizeMB,min_free_space_MB);
						do_clean=true;
					}
				}
			}

			if (sqal_total_items>sqal_max_items){
				do_clean=true;
				GPRINT(NORMAL_LEVEL,"Limit is exceeded, clean DB %llu>%llu\n",sqal_total_items,sqal_max_items);
			}

			if (do_clean){
				GPRINT(NORMAL_LEVEL,"Clean DB, remove %u items\n",sqal_remove_items);
				if (RemoveFirstItems()==ERROR){
					GPRINT(NORMAL_LEVEL,"Error clean DB, not remove %u items\n",sqal_remove_items);
					break;
				}
				else{
					do_clean=false;
					FillTotalItems();
					GPRINT(NORMAL_LEVEL,"sqal_total_items %llu\n",sqal_total_items);
				}
			}
			else
				break;
		}

	return NO_ERROR;
}
eErrorTp algo_sqlite::SaveValues(Json::Value & valarray){
	string values="";
	for (u32 n=0;n<valarray.size();n++){

		if (valarray[n].isString()){
			values+=sf("'%s',",valarray[n].asCString());
			continue;
		}
		//zr();
		if (valarray[n].isUInt64()){
			values+=sf("%llu,",valarray[n].asUInt64());
			continue;
		}
		//zr();
		if (valarray[n].isInt64()){
			values+=sf("%lld,",valarray[n].asInt64());
			continue;
		}
		//zr();
		if (valarray[n].isUInt()){
			values+=sf("%u,",valarray[n].asUInt());
			continue;
		}
		//zr();
		if (valarray[n].isInt()){
			values+=sf("%d,",valarray[n].asInt());
			continue;
		}
		//zr();
		if (valarray[n].isDouble()){
			values+=sf("%lf,",valarray[n].asDouble());
			continue;
		}
		//zr();
		if (valarray[n].isBool()){
			values+=(valarray[n].asBool())?"true,":"false,";
			continue;
		}
		//zr();
		if (valarray[n].isArray()){
			values+="'";
			values+=FastWriteJSON(valarray[n])+"',";
		}
		//zr();

	}
	values.pop_back();
	if (sq_exec_rep(sf("INSERT INTO '%s.data' VALUES(%s)",sqal_db_fname.c_str(),values.c_str()))==ERROR){
		GPRINT(NORMAL_LEVEL,"Error INSERT data\n");
		return ERROR;
	}

	sqal_total_items++;
	GPRINT(NORMAL_LEVEL,"total items %u\n",sqal_total_items);
	Clean();
	return NO_ERROR;
}
eErrorTp algo_sqlite::ExtractValue(string & json, string & value,TIME_T & extract_tstamp,u64 & extract_ts64){
		Json::Value root;
		Json::Value rvalue;
		Json::Reader reader;
		//Json::FastWriter fwr;
		//Json::StyledWriter sfwr;
		value="";

		extract_ts64=0;
		extract_tstamp=0;
		if (reader.parse(json,root)){
			if ((root.isMember("t"))&&(root.isMember("d"))){
				extract_tstamp=root["t"].asUInt();
				extract_ts64=root["m"].asUInt64();
				CheckAndSaveFormat(root);
				rvalue[0]=extract_ts64;
				rvalue[1]=extract_tstamp;
				u32 nidx=2;
				ExtractValueRecurs(root["d"],rvalue,nidx);
				GPRINT(MEDIUM_LEVEL,"Extracted %s\n",FastWriteJSON(rvalue).c_str());
				SaveValues(rvalue);
			//	value=FastWriteJSON(rvalue);
			//	delCh(value, (char)0x0a);
			}
		}
		else{
			GPRINT(NORMAL_LEVEL,"ExtractID_N_Value error json parse\n");
			return ERROR;
		}
		return NO_ERROR;
	}

eErrorTp algo_sqlite::save(u8 * buf){
	string stb=(char*)buf;
	string value;
	//dal_groupid
	TIME_T tstamp=0;
	u64 ts64=0;
	if (db_sql.db==0){
		GPRINT(NORMAL_LEVEL,"Error save in DB\n");
		return ERROR;
	}

	return ExtractValue(stb, value,tstamp,ts64);

	//return NO_ERROR;

}

eErrorTp algo_sqlite::tick(void){
	if (syncDB_Timer->alarm()){
		replication();
	}
	if ((db_sql.db==0)||(db_sql_rsv.db==0)){
		GPRINT(NORMAL_LEVEL,"DB is broken main %x rsv %x, force reboot after 5 sec\n",db_sql.db,db_sql_rsv.db);
		sleep(5);
		return ERROR;
	}
	return NO_ERROR;
};

eErrorTp algo_sqlite::buildlog(){
	GPRINT(NORMAL_LEVEL,"Build log\n");
	string fname=tmpdir+"/"+tmpfile;
	u32 fsz=GetFileSize(sqal_db_path.c_str());
	string cmd=sf("%s -k -f -n -c > %s",sqal_db_path.c_str(),fname.c_str());
	printf("cmd %s\n",cmd.c_str());
	string result=ExecResult("gzip",(char*)cmd.c_str());
	struct stat statbuf;
	lstat((char*)fname.c_str(), &statbuf);
	GPRINT(NORMAL_LEVEL,"Compressed %d byte, ready slog file %s, %d byte, total items %d\n",fsz,fname.c_str(),statbuf.st_size,sqal_total_items);

	return NO_ERROR;
}

#endif
