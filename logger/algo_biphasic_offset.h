/*
 * algo_biphasic_offset.h
 *
 *  Created on: 19 сент. 2019 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_LOGGER_ALGO_BIPHASIC_OFFSET_H_
#define SRC_ENGINE_LOGGER_ALGO_BIPHASIC_OFFSET_H_

#include "safe_logger_srvalgo.h"



// public PrintT
class algo_biphasic_offset: public algo_abstract,GprintT {
	public:
	algo_biphasic_offset(eDebugTp debug_lvl,string * aal_path,u32 aal_max_blocks,u32 aal_lines_in_block,u32 aal_sync_time,u32 aal_offset_time,u8 style,string tmpdir);
	~algo_biphasic_offset(){
		GPRINT(NORMAL_LEVEL,"algo:biphasic_offset is down\n");
	}

	eErrorTp buildlog();
	eErrorTp tick(void);
	eErrorTp save(u8 * buf);
	private:
	eErrorTp clean_oldest_block(u32 stid,u32 cnt);
	eErrorTp move_up_idx();
	eErrorTp sync_blocks();
	eErrorTp analytic(void);
	u32 calc_lines_in_segment(searched_file_list * block );
	eErrorTp seg_info(u8 stid);
	static const u8 total_storage=2;
	string aal_prefix="-aal.log";
	string aal_path[total_storage];
	u32 aal_max_blocks=10000;
	u32 aal_lines_in_block=100;
	bool aal_compress_enable=true;
	bool aal_cache_stings=true;
	u8 aal_save_style=AAL_SAVE_QUOTES_COMMA;
	u32 source_logid=0;
	u32 line_counter=0;
	int sync_time=1;
	u32 offset_time=1;
	filesys file[total_storage];
	TIME_T old_ts=0,old_ts2=0,new_ts=0;
	string tmpdir;
	//string buildLogName="slog.log.gz";
	class{
		public:
		u32  count=0;
		u32  min_idx=0xffffffff;
		u32  max_idx=0;
		u32  total_size=0;
		vector<searched_file_list> files;
		searched_file_list * last_block=NULL;
		searched_file_list * first_block=NULL;
		u32 total_lines=0;
		string select_file;

	} stor[total_storage];

};



#endif /* SRC_ENGINE_LOGGER_ALGO_BIPHASIC_OFFSET_H_ */
