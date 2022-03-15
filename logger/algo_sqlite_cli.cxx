/*
 * algo_sqlite_cli.cxx
 *
 *  Created on: 14 апр. 2020 г.
 *      Author: root
 */

#ifdef _SQLITE

#include "algo_sqlite_cli.h"

eErrorTp algo_sqlite_cli::getColTypes(void){
	//col_types
	opendb();
	string req=sf("SELECT name, value FROM '%s.params' WHERE name='types'",sqal_db_fname.c_str());

	sql_format sql(this);

	/*sq_exec_db(req, &sql,[](void *st, int argc, char **argv, char **azColName) {

		if ((st==NULL)||(argv==NULL)||(azColName==NULL))
			return 0;

		auto sql_frm_p=(sql_format*)st;
		for (int n=0;n<argc;n++){

			//printf("n %d col %s val %s\n",n,azColName[n],argv[n]);
			if (strcmp(azColName[n],"value")==0){

				sql_frm_p->new_format=argv[n];
			}
		}

		return 0;
	});*/

	req=sf("SELECT name, value FROM '%s.params' WHERE name='formatJson'",sqal_db_fname.c_str());
	sq_exec_db(req, &sql,[](void *st, int argc, char **argv, char **azColName) {

			if ((st==NULL)||(argv==NULL)||(azColName==NULL))
				return 0;

			auto sql_frm_p=(sql_format*)st;
			for (int n=0;n<argc;n++){

				//printf("n %d col %s val %s\n",n,azColName[n],argv[n]);
				if (strcmp(azColName[n],"value")==0){

					sql_frm_p->formatJson=parseJSON(argv[n]);
					sql_frm_p->new_format=argv[n];
				}
			}

			return 0;
		});

	closedb();
	//Json::Reader rd;
	//formatJson
	if (sql.new_format.size()!=0){
		//rd.parse(sql.new_format,col_types);
		//col_types=parseJSON(sql.new_format);
		formatJson=sql.formatJson;
		dbReadyForRead=true;
		GPRINT(MEDIUM_LEVEL,"Found DB is ready for read, jsonFormat %s\n",StyledWriteJSON(formatJson).c_str());
	}
	else{
		dbReadyForRead=false;
		return ERROR;
	}
	return NO_ERROR;
}
eErrorTp algo_sqlite_cli::make_selection_by_interval(u64 ts_start_ms,u64 ts_stop_ms,u32 limit,rapidjson::Document & result_root){
	result_root.SetObject();
	string result;
	if (make_selection_by_interval(ts_start_ms,ts_stop_ms,limit,result)==NO_ERROR){
		result_root.Parse(result.c_str());
		return NO_ERROR;
	}
	return ERROR;
}
eErrorTp algo_sqlite_cli::make_selection_by_sql(string sql_req,string & json_string){
	GPRINT(MEDIUM_LEVEL,"Make_selection_by_sql SQL[%s]\n",sql_req.c_str());

	string pattern=sf("SELECT");

	//printf("pos %d\n",sql_req.find(pattern,0));
	if ((int)sql_req.find(pattern,0)==-1){
		GPRINT(NORMAL_LEVEL,"Request not supported, patern %s not found\n",pattern.c_str());
		return ERROR;
	}
	if ((int)sql_req.find(";",0)!=-1){
		GPRINT(NORMAL_LEVEL,"Only one request allowed\n",pattern.c_str());
		return ERROR;
	}
	if (dbReadyForRead==false){
		if (getColTypes()==ERROR){
			GPRINT(NORMAL_LEVEL,"Error read DB. DB not ready or empty!!!\n");
			return ERROR;
		}
	}

	opendb();
	string req=sql_req;
	sql_format sql(this);

	//sql.col_types=col_types;
	for (auto colName:formatJson.getMemberNames()){
		if (formatJson[colName]=="TEXT")
			sql.colsTypeString.push_back(colName);
	}

	//for (u8 n=0;n<sql.colsTypeString.size();n++){
	//	printf("TEXT COL %s\n",sql.colsTypeString[n].c_str());
	//}

	sq_exec_db(req, &sql,[](void *st, int argc, char **argv, char **azColName) {
		auto s=(sql_format*)st;
		string col;
		u8 colTypeStringMark[1000]={0};//максимальное кол столбцов, больше делают только идиоты, по дефолту в SQLITE ограничение 2000
		for (int n=0;n<argc;n++){
			col=azColName[n];

			string value;
			if (colTypeStringMark[n]==0){
				for (u32 z=0;z<s->colsTypeString.size();z++){
					if (s->colsTypeString[z]==col){
						colTypeStringMark[n]=2;//если столбец имеет текстовый тип, то помечаем 2
					}
				}
				if (colTypeStringMark[n]==0)//если столбец не имеет текстовый тип, то помечаем 1
					colTypeStringMark[n]=1;
			}

			//printf("colName %s colTypeStringMark %d value %s\n",col.c_str(),colTypeStringMark[n],argv[n]);

			if (colTypeStringMark[n]==2){//если столбец имеет текстовый тип, то обрамляем в кавычки
				if (argv[n][0]=='['){
					value=argv[n];
				}
				else{
					value+="\"";
					value+=argv[n];
					value+="\"";
				}
			}
			else
				value=argv[n];

			if (std::find(s->col.begin(),s->col.end(),col)==s->col.end()){
				s->col.push_back(col);
				s->val.push_back("");
			}
			s->val[n]+=value+",";

			if (strcmp(azColName[n],azColName[0])==0){
				s->row++;
			}
		}
		return 0;
	});
	json_string="{";
	if (sql.col.size()!=0){
		for (u32 z=0;z<sql.col.size();z++){
			if (sql.val[z].size()!=0){
				sql.val[z].pop_back();
			}
			json_string+=sf("\"%s\":[%s],",sql.col[z].c_str(),sql.val[z].c_str());
		}
		json_string.pop_back();
	}
	json_string+="}";
	//printf("%s",result.c_str());

	closedb();
	return NO_ERROR;
}

eErrorTp algo_sqlite_cli::make_selection_by_interval(u64 ts_start_ms,u64 ts_stop_ms,u32 limit,string & json_string){
		GPRINT(MEDIUM_LEVEL,"Make_selection_by_interval start %llu stop %llu point_limit %u\n",ts_start_ms,ts_stop_ms,limit);
		string req=sf("SELECT * FROM '%s.data' WHERE m>%llu AND m<%llu ORDER BY m LIMIT %u",sqal_db_fname.c_str(),ts_start_ms,ts_stop_ms,selection_limit);
		return make_selection_by_sql(req,json_string);
	}
#endif
