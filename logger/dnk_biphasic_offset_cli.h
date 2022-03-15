/*
 * dnk_biphasic_offset_cli.h
 *
 *  Created on: 30 сент. 2019 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_LOGGER_DNK_BIPHASIC_OFFSET_CLI_H_
#define SRC_ENGINE_LOGGER_DNK_BIPHASIC_OFFSET_CLI_H_


#include "safe_logger_client.h"
#ifdef _HIREDIS
#include "engine/proto/redis.h"


#define SBS_UPLOAD_METHOD_IN_DB_ALL 0

class dnk_biphasic_offset_cli: public safelogger_client,redis {
	public:
	class diapason {
		public:
		diapason(u64 t64,TIME_T tt,string fname){
			ts.push_back(t64);
			start_ts=tt;
			//printf("start ts %u\n",start_ts);
			this->fname=fname;
		}
		void add(u64 & t64){
			ts.push_back(t64);
		}
		vector<u64> ts;
		TIME_T start_ts;
		string fname;
	};
	dnk_biphasic_offset_cli(string format,string config,eDebugTp debug_lvl):safelogger_client(format,config,debug_lvl),redis(debug_lvl,config){
		debug_level=debug_lvl;
		SourceStr="dbo_cli";

		std::ifstream config_doc(config.c_str(), std::ifstream::binary);

		config_doc >> config_js;

		JSON_ReadConfigField(config_js,"dal_groupid",dal_groupid);
		JSON_ReadConfigField(config_js,"dal_base_path[0]",dal_base_path[0]);
		JSON_ReadConfigField(config_js,"dal_base_path[1]",dal_base_path[1]);
		JSON_ReadConfigField(config_js,"dal_lines_in_block",dal_lines_in_block);
		JSON_ReadConfigField(config_js,"dal_selection_limit",dal_selection_limit);
		JSON_ReadConfigField(config_js,"dal_upload_from_DB_method",dal_upload_from_DB_method);

		if (reconnect()==NO_ERROR)
			fault=NO_ERROR;

		GPRINT(NORMAL_LEVEL,"dnk_biphasic_offset_cli created\n");
	}

	eErrorTp make_selection(u32 ts_start,u32 ts_stop,u32 interval,rapidjson::Document & result_root){
		u64 ts_start_ms=(u64)ts_start*1000;
		u64 ts_stop_ms=(u64)ts_stop*1000;
		u64 interval_ms=(u64)interval*1000;

		if (interval_ms==0){
			return make_selection_by_interval(ts_start_ms,ts_stop_ms,dal_selection_limit,result_root);
		}
		else{
			return make_selection_step_by_step_rpd(ts_start_ms,ts_stop_ms,interval_ms,dal_selection_limit,result_root);
		}
	}
	eErrorTp make_selection(u64 ts_start_ms,u64 ts_stop_ms,u32 interval,rapidjson::Document & result_root){
			u64 interval_ms=(u64)interval*1000;

			if (interval_ms==0){
				return make_selection_by_interval(ts_start_ms,ts_stop_ms,dal_selection_limit,result_root);
			}
			else{
				return make_selection_step_by_step_rpd(ts_start_ms,ts_stop_ms,interval_ms,dal_selection_limit,result_root);
			}
		}


	eErrorTp make_selection_from_db(u64 ts_start_ms,u64 ts_stop_ms,rapidjson::Document & result_db_root);
	eErrorTp make_selection_by_interval(u64 ts_start_ms,u64 ts_stop_ms,u32 limit,rapidjson::Document & result_root);
	eErrorTp make_selection_step_by_step_rpd(u64 ts_start_ms,u64 ts_stop_ms,u32 interval_ms,TIME_T point_limit,rapidjson::Document & result_root);
	eErrorTp search_min_max(TIME_T & ts_start,TIME_T & ts_stop);
	eErrorTp make_selection_step_by_step_DB_All(u64 ts_start_ms,u64 saved_ts_stop_ms,rapidjson::Document & header_root,rapidjson::Document & result_root);
	eErrorTp make_selection_step_by_step_DB_step(u64 ts_start_ms,u64 saved_ts_stop_ms,u64 interval_ms,rapidjson::Document & header_root,rapidjson::Document & result_root,u32 & aidx);
	eErrorTp GetDataFromGzip(string & result,string & fname,TIME_T ts,string * format_path);
	eErrorTp GetFormatPath(string * format_path);


	eErrorTp get_fault_status(void){
		return fault;
	}

	private:
		static const u8 total_storage=2;
		Json::Value config_js;
		eErrorTp fault=ERROR;
		u32 result_limit=20;//2000;
		u32 max_rezults=2000000;//20000;
		string dal_base_path[total_storage]={"/www/pages/sys","/www/pages/sys_ex"};
		string dal_groupid="undefined";
		u32 dal_lines_in_block=50;
		u32 dal_selection_limit=4096;
		u8 dal_upload_from_DB_method=SBS_UPLOAD_METHOD_IN_DB_ALL;

};
#endif
#endif /* SRC_ENGINE_LOGGER_DNK_BIPHASIC_OFFSET_CLI_H_ */
