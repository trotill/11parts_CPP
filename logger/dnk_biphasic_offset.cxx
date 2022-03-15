/*
 * dnk_biphasic_offset.cxx
 *
 *  Created on: 19 сент. 2019 г.
 *      Author: root
 */




#include "dnk_biphasic_offset.h"
#include "engine/proto/json_proto.h"
#ifdef _HIREDIS
#include "engine/proto/redis.h"

eErrorTp ReadJsonFormatFile_rpd(rapidjson::Document & header_root,string * dal_format_table_file)
{
		//Доработать блок чтения, текущая реализация не надежная, нужно чекать crc и работать с копиями, посстанавливать поврежденные файлы
	//	rapidjson::Document read;
		header_root.SetObject();
		rapidjson::ParseResult stat;
		string jsonformat_data;
		if (ReadStringFile((char*)dal_format_table_file[0].c_str(),jsonformat_data)==NO_ERROR){
			stat=header_root.Parse(jsonformat_data.c_str());
			if (stat.Code()==0)
			{
				return NO_ERROR;
			}
			printf("Incorrect json format, remove %s\n",dal_format_table_file[0].c_str());
			remove((char*)dal_format_table_file[0].c_str());
		}
		else
			printf("Not found %s\n",dal_format_table_file[0].c_str());

		header_root.SetObject();
		if (ReadStringFile((char*)dal_format_table_file[1].c_str(),jsonformat_data)==NO_ERROR){
			stat=header_root.Parse(jsonformat_data.c_str());
			if (stat.Code()==0)
			{
				return NO_ERROR;
			}
			printf("Incorrect json format, remove %s\n",dal_format_table_file[1].c_str());
			remove((char*)dal_format_table_file[1].c_str());
		}
		else
			printf("Not found %s\n",dal_format_table_file[1].c_str());

		return ERROR;
}


eErrorTp ReadJsonFormatFile(Json::Value & header_root,string * dal_format_table_file)
	{
		//Доработать блок чтения, текущая реализация не надежная, нужно чекать crc и работать с копиями, посстанавливать поврежденные файлы
		Json::Reader read;
		string jsonformat_data;
		if (ReadStringFile((char*)dal_format_table_file[0].c_str(),jsonformat_data)==NO_ERROR){
			if (read.parse(jsonformat_data,header_root))
			{
				return NO_ERROR;
			}
			printf("Incorrect json format, remove %s\n",dal_format_table_file[0].c_str());
			remove((char*)dal_format_table_file[0].c_str());
		}
		else
			printf("Not found %s\n",dal_format_table_file[0].c_str());

		if (ReadStringFile((char*)dal_format_table_file[1].c_str(),jsonformat_data)==NO_ERROR){
			if (read.parse(jsonformat_data,header_root))
			{
				return NO_ERROR;
			}
			printf("Incorrect json format, remove %s\n",dal_format_table_file[1].c_str());
			remove((char*)dal_format_table_file[1].c_str());
		}
		else
			printf("Not found %s\n",dal_format_table_file[1].c_str());

		return ERROR;
	}

eErrorTp MakeObjectFromDataAndHeader_rpd(rapidjson::Document & data_root,rapidjson::Document & header_root,rapidjson::Document & result_root,bool ts_only){


		u32 hrsz=header_root.Size();
		u32 drsz=data_root.Size();
		for (u32 head_idx=0;head_idx<hrsz;head_idx++){
			if (!header_root[head_idx].IsArray()){

				for (u32 data_idx=0; data_idx<drsz;data_idx++){

					const char* sk=header_root[head_idx].GetString();
					if (result_root.HasMember(sk)==false){
						rapidjson::Value index(sk, (u32)strlen(sk), header_root.GetAllocator());
						rapidjson::Value val(rapidjson::kArrayType);
						result_root.AddMember(index,val,result_root.GetAllocator());
					}
					result_root[sk].PushBack(data_root[data_idx][head_idx], result_root.GetAllocator());
				}
			}
			else{

				if (ts_only==false){
					//if (drsz!=header_root[0].Size()){
					//	printf("Error: The data does not match the format data %u!= head %u\n",drsz,header_root[0].Size());
					//	return ERROR;
					//}

					for (u32 data_idx=0; data_idx<drsz;data_idx++){
						//u32 dsz=data_root[data_idx][0].Size();
						for (u32 di=0; di<header_root[0].Size();di++){

							const char* sk=header_root[0][di].GetString();
							if (result_root.HasMember(sk)==false){
								rapidjson::Value index(sk, (u32)strlen(sk), header_root.GetAllocator());
								rapidjson::Value val(rapidjson::kArrayType);
								result_root.AddMember(index,val,result_root.GetAllocator());
							}
							result_root[sk].PushBack(data_root[data_idx][0][di], result_root.GetAllocator());

							//result_root[header_root[0][di].GetString()][data_idx]=data_root[data_idx][0][di];

						}
					}
				}
			}

		}

		return NO_ERROR;
	}

eErrorTp MakeObjectFromDataAndHeader(Json::Value & data_root,Json::Value & header_root,Json::Value & result_root,bool ts_only){


		//u32 t_idx=0;
		//u32 m_idx=0;
		//u32 d_idx=0;
		//printf("dsize %d\n",data_root.size());
		u32 hrsz=header_root.size();
		u32 drsz=data_root.size();
		for (u32 head_idx=0;head_idx<hrsz;head_idx++){
			if (!header_root[head_idx].isArray()){

				for (u32 data_idx=0; data_idx<drsz;data_idx++){
					result_root[header_root[head_idx].asCString()][data_idx]=data_root[data_idx][head_idx];
					//printf("%d.",data_idx);
				}
				//printf("z1\n");
			}
			else
				if (ts_only==false)
					for (u32 data_idx=0; data_idx<drsz;data_idx++){
						for (u32 di=0; di<header_root[0].size();di++){
							result_root[header_root[0][di].asCString()][data_idx]=data_root[data_idx][0][di];
						}
					}

		}
		return NO_ERROR;
	}


dnk_biphasic_offset::dnk_biphasic_offset(Json::Value & config):rdb((eDebugTp)config["slogger_loglevel"].asInt(),
		(char*)config["redis_url"].asCString(),(u16)config["redis_port"].asUInt(),(u8)config["redis_dbid"].asUInt())
{
		u32 val=(u32)NORMAL_LEVEL;
		JSON_ReadConfigField(config,"dal_groupid",dal_groupid);
		SourceStr=dal_groupid;
		ObjPref="slogger";
		JSON_ReadConfigField(config,"slogger_loglevel",val);
		debug_level=(eDebugTp)val;
		JSON_ReadConfigField(config,"dal_base_path[0]",dal_base_path[0]);
		JSON_ReadConfigField(config,"dal_base_path[1]",dal_base_path[1]);
		JSON_ReadConfigField(config,"dal_max_blocks",dal_max_blocks);
		JSON_ReadConfigField(config,"dal_lines_in_block",dal_lines_in_block);
		if (JSON_ReadConfigField(config,"dal_remove_window",dal_remove_window)==ERROR)
			dal_remove_window=(dal_max_blocks*dal_lines_in_block)/100;//1% for default

		dal_remove_treshold=dal_max_blocks*dal_lines_in_block;
		JSON_ReadConfigField(config,"dal_sync_time",(u32&)sync_time);
		JSON_ReadConfigField(config,"dal_enable_crc",dal_enable_crc);
		JSON_ReadConfigField(config,"dal_offset_time",(u32&)offset_time);

		//allow_remove_if_low_space
		JSON_ReadConfigField(config,"dal_allow_remove_if_low_space",allow_remove_if_low_space);
		if (allow_remove_if_low_space)
			GPRINT(NORMAL_LEVEL,"Allow remove oldest files if detect low space\n");

		JSON_ReadConfigField(config,"dal_min_free_space_MB",(u32&)min_free_space_MB);

		JSON_ReadConfigField(config,"wwwtmpdir",tmpdir);

		JSON_ReadConfigField(config,"dal_undefined_counter_max",undefined_counter_max[0]);
		undefined_counter_max[1]=undefined_counter_max[0];
		JSON_ReadConfigField(config,"dal_autofin_delta",(u32&)dal_autofin_delta_s);


		GPRINT(NORMAL_LEVEL,"dal_path[0] %s,dal_path[1] %s, dal_max_blocks %d, dal_lines_in_block %d, dal_sync_time %d, dal_offset_time %d\n",
				dal_base_path[0].c_str(),dal_base_path[1].c_str(),dal_max_blocks,dal_lines_in_block,sync_time,offset_time);


		GPRINT(NORMAL_LEVEL,"dnk_biphasic_offset log_level %d\n",debug_level);

		MkPath(dal_base_path[0].c_str(), 0xffffffff);
		MkPath(dal_base_path[1].c_str(), 0xffffffff);

		if (dal_max_blocks>99999) {
			GPRINT(NORMAL_LEVEL,"Error:Not correct dal_max_blocks!!!, delay 10s...\n");
			sleep(10);
			return;
		}

		GPRINT(NORMAL_LEVEL,"algo:dnk_biphasic started\n");

		for (u8 n=0;n<10;n++){
			if(rdb.reconnect()==ERROR){
				sleep(1);
				printf("Redis DB error, not connect!!!\n");
			}
			else
				break;
		}

		rdb.cleardb();
		sync_blocks();

		rdb.cleardb();
		DB_FillAndScanDAL(0,METHOD_FILLDB);


		if (DB_GetMessagesCount(total_item_on_storage)==NO_ERROR){
			printf("Total items %u\n",total_item_on_storage);
		}

		DB_RemoveOldestFilesByItems(dal_remove_treshold,0);
		DB_RemoveOldestFilesByLowFreeSize();
		for (u8 n=0;n<total_storage;n++)
			DB_ChangeUncomplete(undefined_counter,n);
	}

eErrorTp dnk_biphasic_offset::buildlog()
	{
		//seg_info(0);
#if 0
		GPRINT(NORMAL_LEVEL,"Build log\n");
		if (stor[0].count==0){
			GPRINT(NORMAL_LEVEL,"Not build log, 0 blocks\n");
			return ERROR;
		}
		vector<u32> sort;
		//filesys file;
		u32 min;
		u32 m=0;//stor[0].min_idx;
	//	sort.push_back(stor[0].min_idx);
		u32 imin=0;
		for (u32 i=0;i<stor[0].count;i++){
			//GPRINT(NORMAL_LEVEL,"%d in %d [%s]\n",i,stor[0].count,stor[0].files[i].name.c_str());
			min=0xffffffff;
			for (u32 z=0;z<stor[0].count;z++){
				if ((stor[0].files[z].idx<min)&&(stor[0].files[z].idx>m)){
					min=stor[0].files[z].idx;
					imin=z;
				}
			}
			//printf("min %d,imin %d,idx %d\n",min,imin,stor[0].files[imin].idx);
			sort.push_back(imin);
			m=min;
			//sort.push_back(i);
		}

		//file.file_set_file_name(tmpdir+"/slog.log",false);
		string fname=tmpdir+"/slog.log.gz";
		FILE * file=fopen(fname.c_str(),"w");
		//GPRINT(NORMAL_LEVEL,"open %s\n",fname.c_str());
		for (u32 i=0;i<stor[0].count;i++){
			//GPRINT(NORMAL_LEVEL,"%d in %d, sort[%d]=%d fidx=[%d]\n",i,stor[0].count,i,sort[i],stor[0].files[sort[i]].idx);


			ifstream dfile(stor[0].files[sort[i]].fname);
			std::string str((std::istreambuf_iterator<char>(dfile)),
			                 std::istreambuf_iterator<char>());

			//printf("Compress %s sorted idx[%d] abs idx [%d]\n",stor[0].files[sort[i]].fname.c_str(),sort[i],i);
			string compressed_data = gzip::compress(str.c_str(), str.size());
			fwrite(compressed_data.data(),compressed_data.size(),1,file);
			//file.file_add_string(compressed_data.data());
			//printf("push %d\n",sort[i]);
		}
		//file.file_close();
		fclose(file);


		struct stat statbuf;
		lstat((char*)fname.c_str(), &statbuf);
		GPRINT(NORMAL_LEVEL,"Compressed %d byte, ready slog file %s, %d byte, total block %d\n",stor[0].total_size,fname.c_str(),statbuf.st_size,sort.size());
		sort.clear();
#endif
		return NO_ERROR;
	}

	eErrorTp dnk_biphasic_offset::tick(void){
		int time_last=0;
		new_ts=TIME((u32*)NULL);
		if (old_ts>new_ts)
			old_ts=new_ts;

		time_last=new_ts-old_ts;
		if ((time_last>sync_time)&&(file[0].file_reset_write_cntr()!=0)){
			old_ts2=old_ts=new_ts;
			file[0].file_sync();
			GPRINT(MEDIUM_LEVEL,"SYNC1\n");
		}

		if (old_ts2!=0){
			if (old_ts2>new_ts)
				old_ts2=new_ts;
			u32 tts=(new_ts-old_ts2);
			if (tts>offset_time){
				file[1].file_sync();
				old_ts2=0;
				GPRINT(MEDIUM_LEVEL,"SYNC2\n");
			}
		}
		//if (dal_compress_enable){

		//}

		bool trig=false;
		for (u8 n=0;n<total_storage;n++){
			//printf("last_save_ts[%d]=%d, new_ts=%d, dal_autofin_delta_s=%d file[n].get_cache_size()=%d\n",n,last_save_ts[n],new_ts,dal_autofin_delta_s,file[n].get_cache_size());
			if (((last_save_ts[n]>new_ts)||((last_save_ts[n]+dal_autofin_delta_s)<new_ts))&&(line_counter>0)){
				GPRINT(NORMAL_LEVEL,"Try finalize file %s\n",stor[n].select_file.c_str());
				file[n].file_sync();
				FinalizeFile(dal_path[n],stor[n].select_file,n);
				stor[n].select_file=string_format("%s/%s%d",dal_path[n].c_str(),DNK_BIPHASIC_BUILD_FILE_NAME,undefined_counter[n]);
				DB_ChangeUncomplete(undefined_counter,n);
				file[n].file_set_file_name(stor[n].select_file,true);
				trig=true;

				//TermWriteLogic();
				/*
				file[n].file_sync();
				FinalizeFile(dal_path[n],stor[n].select_file,n);
				stor[n].select_file=string_format("%s/%s%d",dal_path[n].c_str(),DNK_BIPHASIC_BUILD_FILE_NAME,undefined_counter[n]);
				file[n].file_set_file_name(stor[n].select_file,true);
				trig=true;
				*/

			}
		}

		if (trig)
			line_counter=0;

		return NO_ERROR;
	}

	eErrorTp dnk_biphasic_offset::save(u8 * buf){
		string stb=(char*)buf;
		string value;
		//dal_groupid
		TIME_T tstamp=0;
		u64 ts64=0;
		//format_table_found
		last_save_ts[0]=last_save_ts[1]=TIME((u32*)NULL);
		ExtractValue(stb, value,tstamp,ts64);
		DB_AddUncomplData(value,ts64,undefined_counter);
		value=value+',';
		if (line_counter==0){
			value='['+value;

		}
		//printf("line_counter %u undefined_counter %u\n",line_counter,undefined_counter[0]);
		file[0].file_add_string(value);
		file[1].file_add_string(value);


		line_counter++;
		if (line_counter>=dal_lines_in_block){
			line_counter=0;
			TermWriteLogic();
			DB_RemoveOldestFilesByItems(dal_remove_treshold,dal_remove_window);
			DB_RemoveOldestFilesByLowFreeSize();
		}

		return NO_ERROR;
	}


	eErrorTp dnk_biphasic_offset::sync_blocks(){
		std::pair<eErrorTp, vector<string>> res1,res2;
		std::pair<eErrorTp, vector<string>> r1,r2;
		string s_src;
		string s_dst;
		if (DB_FillAndScanDAL(0,METHOD_COMPARE_FILES)==NO_ERROR){
			if (DB_FillAndScanDAL(1,METHOD_COMPARE_FILES)==NO_ERROR){
				res1=rdb.command("SDIFF s0 s1");
				res2=rdb.command("SDIFF s1 s0");
				if ((res1.first==ERROR)){
					GPRINT(NORMAL_LEVEL,"Error diff set s1 s2\n");
					return ERROR;
				}
				else{
					for(u32 n=0;n<res1.second.size();n++){
						r1=rdb.command("GET 0:%s",res1.second[n].c_str());
						if (r1.first==NO_ERROR){
							TIME_T ts_start=stoul(res1.second[n]);
							struct tm * ptm=GMTIME(&ts_start);
							u32 year=ptm->tm_year+1900;
							s_src=string_format("%s/%s/%u/%s/%s",dal_base_path[0].c_str(),DNK_BIPHASIC_PREFIX,year,dal_groupid.c_str(),r1.second[0].c_str());
							s_dst=string_format("%s/%s/%u/%s/%s",dal_base_path[1].c_str(),DNK_BIPHASIC_PREFIX,year,dal_groupid.c_str(),r1.second[0].c_str());
							if (CopyFile((char*)s_src.c_str(),(char*)s_dst.c_str())==ERROR){
								MkPath(string_format("%s/%s/%u/%s/",dal_base_path[1].c_str(),DNK_BIPHASIC_PREFIX,year,dal_groupid.c_str()).c_str(), 0xffffffff);
								if (CopyFile((char*)s_src.c_str(),(char*)s_dst.c_str())==ERROR){
									GPRINT(NORMAL_LEVEL,"Not restored file %s -> %s, check filesystem\n",s_src.c_str(),s_dst.c_str());
									break;
								}
								else
									GPRINT(NORMAL_LEVEL,"Restored file %s -> %s\n",s_src.c_str(),s_dst.c_str());
							}
							else
								GPRINT(NORMAL_LEVEL,"Restored file %s -> %s\n",s_src.c_str(),s_dst.c_str());
						}
					}
				}
				if ((res2.first==ERROR)){
					GPRINT(NORMAL_LEVEL,"Error diff set s2 s1\n");
						return ERROR;
				}
				else{
					for(u32 n=0;n<res2.second.size();n++){
						r2=rdb.command("GET 1:%s",res2.second[n].c_str());
						if (r2.first==NO_ERROR){
							TIME_T ts_start=stoul(res2.second[n]);
							struct tm * ptm=GMTIME(&ts_start);
							u32 year=ptm->tm_year+1900;
							s_src=string_format("%s/%s/%u/%s/%s",dal_base_path[1].c_str(),DNK_BIPHASIC_PREFIX,year,dal_groupid.c_str(),r2.second[0].c_str());
							s_dst=string_format("%s/%s/%u/%s/%s",dal_base_path[0].c_str(),DNK_BIPHASIC_PREFIX,year,dal_groupid.c_str(),r2.second[0].c_str());
							if (CopyFile((char*)s_src.c_str(),(char*)s_dst.c_str())==ERROR){
								MkPath(string_format("%s/%s/%u/%s/",dal_base_path[0].c_str(),DNK_BIPHASIC_PREFIX,year,dal_groupid.c_str()).c_str(), 0xffffffff);
								if (CopyFile((char*)s_src.c_str(),(char*)s_dst.c_str())==ERROR){
									GPRINT(NORMAL_LEVEL,"Not restored file %s -> %s, check filesystem\n",s_src.c_str(),s_dst.c_str());
									break;
								}
								else
									GPRINT(NORMAL_LEVEL,"Restored file %s -> %s\n",s_src.c_str(),s_dst.c_str());
							}
							else
								GPRINT(NORMAL_LEVEL,"Restored file %s -> %s\n",s_src.c_str(),s_dst.c_str());
							//GPRINT(NORMAL_LEVEL,"Restored file %s -> %s\n",s_src.c_str(),s_dst.c_str());
						}


					}
				}
			}
		}
		GPRINT(NORMAL_LEVEL,"DB fragments in two dir sync ok\n");
		return NO_ERROR;
	}
#endif
