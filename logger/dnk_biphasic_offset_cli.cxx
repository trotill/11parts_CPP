/*
 * dnk_biphasic_offset_cli.cxx
 *
 *  Created on: 30 сент. 2019 г.
 *      Author: root
 */




#include "dnk_biphasic_offset_cli.h"
#ifdef _HIREDIS
eErrorTp dnk_biphasic_offset_cli::make_selection_by_interval(u64 ts_start_ms,u64 ts_stop_ms,u32 limit,rapidjson::Document & result_root){
		if (fault==ERROR){
			GPRINT(NORMAL_LEVEL,"make_selection_by_interval is fault\n");
			return ERROR;
		}

		GPRINT(MEDIUM_LEVEL,"Make_selection_by_interval ts_start_ms %llu ts_stop_ms %llu point_limit %u\n",ts_start_ms,ts_stop_ms,limit);
		//GPRINT(NORMAL_LEVEL,"make_selection_by_interval M1\n");
		result_root.SetObject();
		std::pair<eErrorTp, vector<string>> res,res2;

		TIME_T ts_stop=(u64)ts_stop_ms/1000;
		TIME_T ts_start=(u64)ts_start_ms/1000;
		TIME_T tss;
		//Json::Reader read;
		string fn_old;
		string format_path[total_storage];
		//Json::Value header_root;
		rapidjson::Document header_root;
		bool format_getting=false;
	//	bool result_root_init=false;
		u32 aidx=0;
		u32 limf=(limit/dal_lines_in_block)+1;

		res=command("ZREVRANGEBYSCORE ts %lu %lu LIMIT 0 1",ts_start,0);

		TIME_T ts_start_tmp=ts_start;
		if ((res.second.size()==0)||(res.first==ERROR)){
			GPRINT(HARD_LEVEL,"ZREVRANGEBYSCORE ts %lu %lu LIMIT 0 1 result is null, try next time %llu\n",ts_start,0,ts_start_ms);
		}
		else //поиск предыдущего фрагмента, для захвата всего диапазона
			ts_start_tmp=stol(res.second[0]);


		res=command("HMGET %u te",ts_start_tmp);
		if ((res.second.size()==0)||(res.first==ERROR)){

			GPRINT(HARD_LEVEL,"HMGET %u te result is null\n",ts_start_tmp);
		}
		else{
			//пропустить фрагмент если его содержимое старее ts_start_tmp
			TIME_T et=stol(res.second[0]);
			if (et<ts_start)
				ts_start_tmp=et;
		}

		if (ts_start_tmp>ts_stop)
			return NO_ERROR;
		//printf("sts %lu ts_start %lu ts_stop %lu limf %u\n",TIME(NULL),ts_start,ts_stop,limf);
		//Search in files
		GPRINT(MEDIUM_LEVEL,"ZRANGEBYSCORE ts %u %u LIMIT 0 %u\n",ts_start_tmp,ts_stop,limf);

		res=command("ZRANGEBYSCORE ts %u %u LIMIT 0 %u",ts_start_tmp,ts_stop,limf);
		if ((res.first==ERROR)||(res.second.size()==0)){
			GetFormatPath(format_path);
			if (ReadJsonFormatFile_rpd(header_root,format_path)==ERROR){
				return ERROR;
			}

			return make_selection_step_by_step_DB_All(ts_start_ms,ts_stop_ms,header_root,result_root);

		}

		u32 ctr=res.second.size();
		GPRINT(MEDIUM_LEVEL,"Founded %d arch files\n",ctr);
		//printf("ts_start_ms %llu ts_stop_ms %llu ctr %u\n",ts_start_ms,ts_stop_ms,ctr);
		for (u32 z=0;z<ctr;z++){

				res2=command("HMGET %s te cn ext crc",res.second[z].c_str());
				TIME_T t_end=stoll(res2.second[0]);
				TIME_T t_st=stoll(res.second[z]);
				string fn=res.second[z]+'_'+res2.second[0]+':'+res2.second[1]+'['+res2.second[3]+"]."+res2.second[2];
				string data;
				if (GetDataFromGzip(data,fn,t_st,format_path)==NO_ERROR){
					rapidjson::Document dr_root;
					dr_root.SetObject();

					rapidjson::Document data_root;
					rapidjson::ParseResult rapid_result=data_root.Parse(data.c_str());
					if (rapid_result.Code()==0){
							if (format_getting==false){
								if (ReadJsonFormatFile_rpd(header_root,format_path)==ERROR){
									return ERROR;
								}
								format_getting=true;
							}
							MakeObjectFromDataAndHeader_rpd(data_root,header_root,dr_root,false);

							if ((!dr_root.HasMember("t"))||(!dr_root["t"].IsArray())){
								return ERROR;
							}

							for (u32 e=0;e<dr_root["m"].Size();e++){
								TIME_T t=dr_root["t"][e].GetInt();
								ts_start_ms=dr_root["m"][e].GetUint64()+1;
								if (t<ts_start){
									GPRINT(MEDIUM_LEVEL,"Skip ts %lu < start_ts %lu\n",t,ts_start);
									continue;
								}

								if (t>ts_stop){
									GPRINT(MEDIUM_LEVEL,"Skip ts %lu > ts_stop %lu\n",t,ts_stop);
									continue;
								}

								//	printf("	point %llu, found %llu\n",dia[r].ts[k],dr_root["m"][e].asUInt64());
								for (auto & key:dr_root.GetObject() ){
									const char* keys=key.name.GetString();
									if (result_root.HasMember(keys)==false){
										rapidjson::Value val(rapidjson::kArrayType);
										rapidjson::Value index(keys, (u32)strlen(keys), result_root.GetAllocator());
										result_root.AddMember(index,val,result_root.GetAllocator());
									}
									//if (dr_root[keys][e].IsString())
									//	printf("add %s\n",dr_root[keys][e].GetString());

								//	if (dr_root[keys][e].IsUint())
									//			printf("add %u\n",dr_root[keys][e].GetUint());

									result_root[keys].PushBack(dr_root[keys][e], result_root.GetAllocator());

								}
								aidx++;
								if (aidx>=limit)
									break;
								//result_root_init=true;
							}
					}
				}
		}

		if (format_getting==false){
			GetFormatPath(format_path);
			if (ReadJsonFormatFile_rpd(header_root,format_path)==ERROR){
					return ERROR;
			}
		}

		make_selection_step_by_step_DB_All(ts_start_ms,ts_stop_ms,header_root,result_root);

		//printf("Search in DB %d\n",result_root["m"].size());
		return NO_ERROR;
	}


eErrorTp dnk_biphasic_offset_cli::make_selection_step_by_step_rpd(u64 ts_start_ms,u64 ts_stop_ms,u32 interval_ms,TIME_T point_limit,rapidjson::Document & result_root){

		if (fault==ERROR){
			GPRINT(NORMAL_LEVEL,"make_selection_step_by_step is fault\n");
			return ERROR;
		}
		if (interval_ms==0)
			return ERROR;

		GPRINT(MEDIUM_LEVEL,"Make_selection_step_by_step ts_start_ms %llu ts_stop_ms %llu interval_ms %llu point_limit %u\n",ts_start_ms,ts_stop_ms,interval_ms,point_limit);
		result_root.SetObject();
		std::pair<eErrorTp, vector<string>> res_t,res,res2;
		vector<diapason> dia;
		u64 saved_ts_stop_ms=ts_stop_ms;
		TIME_T ts_stop=ts_stop_ms/1000;
		TIME_T ts_start=ts_start_ms/1000;

		TIME_T ctr=((ts_stop_ms-ts_start_ms)/interval_ms)+1;
		if (ctr>point_limit)
			ctr=point_limit;

		TIME_T t_st;
		TIME_T t_end;
		string fn;
		u64 t_end_ms;
		u64 t_st_ms;
		TIME_T  ctr_cntr=0;

		GPRINT(MEDIUM_LEVEL,"\n*****\n**make_selection_step_by_step [start %llu stop %llu cntr %u interval_ms %u]\n",ts_start_ms,ts_stop_ms,ctr,interval_ms);

		while ((ts_start_ms<ts_stop_ms)&&(ctr>=ctr_cntr)){
			ts_start=ts_start_ms/1000;
			ctr_cntr++;
			res=command("ZREVRANGEBYSCORE ts %lu %lu LIMIT 0 1",ts_start,0);

			if ((res.second.size()==0)||(res.first==ERROR)){
				ts_start_ms+=interval_ms;
				GPRINT(HARD_LEVEL,"ZREVRANGEBYSCORE ts %lu %lu LIMIT 0 1 result is null, try next time %llu\n",ts_start,0,ts_start_ms);
				continue;
			}
			t_st=stoll(res.second[0]);

			GPRINT(HARD_LEVEL,"got %d\n",res.second.size());
			res2=command("HMGET %s te cn ext crc",res.second[0].c_str());

			if ((res2.second.size()==0)||(res2.first==ERROR)){
				ts_start_ms+=interval_ms;
				GPRINT(HARD_LEVEL,"HMGET %s te cn ext crc result is null, try next time %llu\n",res.second[0].c_str(),ts_start_ms);
				continue;
			}

			t_end=stoll(res2.second[0]);
			t_end_ms=(u64)t_end*1000;
			t_st_ms=(u64)t_st*1000;
			if ((ts_start>=t_st)&&(ts_start<=t_end)){
					fn=res.second[0]+'_'+res2.second[0]+':'+res2.second[1]+'['+res2.second[3]+"]."+res2.second[2];
					dia.emplace_back(ts_start_ms,ts_start,fn);
					ts_start_ms+=interval_ms;
					GPRINT(HARD_LEVEL,"Collect %s, ts_start enter to diapason\n",fn.c_str());
					//printf("t_st_ms %llu ts_start_ms %llu t_end_ms %llu\n",t_st_ms,ts_start_ms,t_end_ms);
					while((ts_start_ms>=t_st_ms)&&(ts_start_ms<=t_end_ms)){
						dia[dia.size()-1].add(ts_start_ms);
						//printf("* fn %s point %llu>=%llu<=%llu\n",fn.c_str(),t_st_ms,ts_start_ms,t_end_ms);

						ts_start_ms+=interval_ms;
					}
			}
			else{
				ts_start_ms+=interval_ms;
				GPRINT(HARD_LEVEL,"Skip ts_start out in diapason [%u] %u [%u]\n",t_st,ts_start,t_end);
			}
		}

		u32 tp=0;

		bool result_root_init=false;
		u32 aidx=0;
		bool format_getting=false;
		rapidjson::Document header_root;
		string format_path[total_storage];

		for (u32 r=0;r<dia.size();r++){
			string data;

			if (GetDataFromGzip(data,dia[r].fname,dia[r].start_ts,format_path)==NO_ERROR){
				rapidjson::Document dr_root;
				dr_root.SetObject();

				rapidjson::Document data_root;
				rapidjson::ParseResult rapid_result=data_root.Parse(data.c_str());
				if (rapid_result.Code()==0){
						if (format_getting==false){
							if (ReadJsonFormatFile_rpd(header_root,format_path)==ERROR){
								return ERROR;
							}
							format_getting=true;
						}
						MakeObjectFromDataAndHeader_rpd(data_root,header_root,dr_root,false);

						if ((!dr_root.HasMember("t"))||(!dr_root["t"].IsArray())){
							return ERROR;
						}

						u32 k=0;
						u32 diasz=dia[r].ts.size();
						for (u32 e=0;e<dr_root["m"].Size();e++){
							if (((u64)dr_root["m"][e].GetInt64())>=dia[r].ts[k]){
								for (auto& key : dr_root.GetObject())
								{
									const char* keys=key.name.GetString();
									if (result_root.HasMember(keys)==false){
										rapidjson::Value val(rapidjson::kArrayType);
										rapidjson::Value index(keys, (u32)strlen(keys), result_root.GetAllocator());
										result_root.AddMember(index,val,result_root.GetAllocator());
									}
									result_root[keys].PushBack(dr_root[keys][e], result_root.GetAllocator());
								}
								k++;
								if (k>=diasz)
								{
									break;
								}
							}
						}
				}
			}
			tp+=dia[r].ts.size();
		}

		if (format_getting==false){
			GetFormatPath(format_path);
			if (ReadJsonFormatFile_rpd(header_root,format_path)==ERROR){
					return ERROR;
			}
		}
		if (dal_upload_from_DB_method==SBS_UPLOAD_METHOD_IN_DB_ALL)
			make_selection_step_by_step_DB_All(ts_start_ms,saved_ts_stop_ms,header_root,result_root);
		else
			make_selection_step_by_step_DB_step(ts_start_ms,saved_ts_stop_ms,interval_ms,header_root,result_root,aidx);

		return NO_ERROR;
}

eErrorTp dnk_biphasic_offset_cli::make_selection_step_by_step_DB_All(u64 ts_start_ms,u64 saved_ts_stop_ms,rapidjson::Document & header_root,rapidjson::Document & result_root){
	rapidjson::Document result_db_root;
	result_db_root.SetArray();

	rapidjson::Document dr_root;
	dr_root.SetObject();
	//u64 saved_ts_stop_ms=UINT64_MAX;
	//bool result_root_init=false;
	GPRINT(MEDIUM_LEVEL,"search elements in redis start %llu stop %llu\n",ts_start_ms,saved_ts_stop_ms);
	if ((make_selection_from_db(ts_start_ms,saved_ts_stop_ms,result_db_root)==NO_ERROR)){
		if ((result_db_root.IsArray())&&(result_db_root.Size()!=0)){
			MakeObjectFromDataAndHeader_rpd(result_db_root,header_root,dr_root,false);

			if ((!dr_root.HasMember("t"))||(!dr_root["t"].IsArray())){
					GPRINT(MEDIUM_LEVEL,"temporary elements in redis is broken dump: %s\n",(char*)StyledWriteJSON(&dr_root).c_str());
					return ERROR;
			}
			GPRINT(MEDIUM_LEVEL,"found [%u] temporary elements in redis\n",dr_root["m"].Size());
			for (u32 e=0;e<dr_root["m"].Size();e++){
					for (auto& key : dr_root.GetObject()){
						const char* keys=key.name.GetString();
															//printf("obj %s\n",keys);
						if (result_root.HasMember(keys)==false){
							rapidjson::Value val(rapidjson::kArrayType);
							rapidjson::Value index(keys, (u32)strlen(keys), result_root.GetAllocator());
							result_root.AddMember(index,val,result_root.GetAllocator());
						}
						result_root[keys].PushBack(dr_root[keys][e], result_root.GetAllocator());
					}
			}
			return NO_ERROR;
		}
		else{
			GPRINT(MEDIUM_LEVEL,"not found temporary elements in redis\n");
			return ERROR;
		}
	}
	else{
		GPRINT(MEDIUM_LEVEL,"redis db is broken\n");
		return ERROR;
	}
	return ERROR;
}

eErrorTp dnk_biphasic_offset_cli::make_selection_step_by_step_DB_step(u64 ts_start_ms,u64 saved_ts_stop_ms,u64 interval_ms,rapidjson::Document & header_root,rapidjson::Document & result_root,u32 & aidx){
	rapidjson::Document result_db_root;
	result_db_root.SetArray();

	rapidjson::Document dr_root;
	dr_root.SetObject();
	//bool result_root_init=false;
	if ((make_selection_from_db(ts_start_ms,saved_ts_stop_ms,result_db_root)==NO_ERROR)){
		if ((result_db_root.IsArray())&&(result_db_root.Size()!=0)){
			MakeObjectFromDataAndHeader_rpd(result_db_root,header_root,dr_root,false);
			if ((!dr_root.HasMember("t"))||(!dr_root["t"].IsArray())){
					return ERROR;
			}
			GPRINT(MEDIUM_LEVEL,"found [%u] temporary elements in redis\n",dr_root["m"].Size());
			u64 fval=(u64)dr_root["m"][0].GetInt64();
			while(ts_start_ms<fval){
				ts_start_ms+=interval_ms;
			}

			for (u32 e=0;e<dr_root["m"].Size();e++){

					//printf("	 in db %llu ts_start_ms %llu\n",dr_root["m"][e].asUInt64(),ts_start_ms);

					if ((u64)dr_root["m"][e].GetInt64()>=ts_start_ms){
						//printf("	found in db %llu ts_start_ms %llu\n",dr_root["m"][e].asUInt64(),ts_start_ms);
						for (auto & key:dr_root.GetObject() ){
							const char* keys=key.name.GetString();
							if (result_root.HasMember(keys)==false){
								rapidjson::Value val(rapidjson::kArrayType);
								rapidjson::Value index(keys, (u32)strlen(keys), result_root.GetAllocator());
								result_root.AddMember(index,val,result_root.GetAllocator());
							}
							result_root[keys].PushBack(dr_root[keys][e], result_root.GetAllocator());
						}
						ts_start_ms+=interval_ms;
					}
			}
		}
	}
	else{
		GPRINT(MEDIUM_LEVEL,"not found temporary elements in redis\n");
	}

	return NO_ERROR;
}

eErrorTp dnk_biphasic_offset_cli::search_min_max(TIME_T & ts_start,TIME_T & ts_stop){
	std::pair<eErrorTp, vector<string>> resmin,resmin_for_max,resmax;
	resmin=command("ZRANGEBYSCORE ts 0 %u LIMIT 0 1",ts_stop);
	if ((resmin.first==ERROR)||(resmin.second.size()==0))
			return ERROR;

	resmin_for_max=command("ZRANGE ts -1 -1");
	if ((resmin_for_max.first==ERROR)||(resmin_for_max.second.size()==0))
		return ERROR;

	resmax=command("HMGET %s te",resmin_for_max.second[0].c_str());
	if ((resmax.first==ERROR)||(resmax.second.size()==0))
		return ERROR;

	TIME_T minval=stol(resmin.second[0]);
	TIME_T maxval=stol(resmax.second[0]);
	if (ts_stop<minval){
		GPRINT(MEDIUM_LEVEL,"Skip search:ts_stop[%u]<minval[%u]\n",ts_stop,minval);
		return ERROR;
	}

	if (ts_start>maxval){
		GPRINT(MEDIUM_LEVEL,"Skip search:ts_start[%u]<maxval[%u]\n",ts_start,maxval);
		return ERROR;
	}

	if (ts_start<minval)
		ts_start=minval;

	if (ts_stop>maxval)
		ts_stop=maxval;

	if (ts_start>ts_stop){
		GPRINT(MEDIUM_LEVEL,"Skip search:ts_start[%u]>ts_stop[%u]\n",ts_start,ts_stop);
		return ERROR;
	}

	GPRINT(MEDIUM_LEVEL,"Found min %u max %u\n",ts_start,ts_stop);
	//if (((minval>=ts_start)||(ts_start<0))&&((maxval<=ts_stop)||(ts_stop<0))){
	//	ts_start=minval
	//	ts_stop=maxval;
	//	return NO_ERROR;
	//}

	return NO_ERROR;

}

eErrorTp dnk_biphasic_offset_cli::make_selection_from_db(u64 ts_start_ms,u64 ts_stop_ms,rapidjson::Document & result_db_root){
			std::pair<eErrorTp, vector<string>> res=command("GET u");
			if (res.first==NO_ERROR){
				std::pair<eErrorTp, vector<string>> res1=command("ZRANGEBYSCORE u%s %llu %llu",res.second[0].c_str(),ts_start_ms,ts_stop_ms);
				if (res1.first==NO_ERROR){
					if (res1.second.size()!=0){
						string s="[";
						for (u32 z=0;z<res1.second.size();z++){
							s=s+res1.second[z]+',';
						}
						s[s.size()-1]=']';
						rapidjson::ParseResult rp =result_db_root.Parse(s.c_str());

						if (rp.Code()==0){
							return NO_ERROR;
						}
						else
							return ERROR;
					}
					return NO_ERROR;
				}
				else
					return ERROR;
			}
			else
				return NO_ERROR;
		}

	eErrorTp dnk_biphasic_offset_cli::GetFormatPath(string * format_path){
		TIME_T t=TIME((u32*)NULL);
		struct tm * ptm=GMTIME(&t);
		u32 year=ptm->tm_year+1900;
		//printf("e\n");
		string dal_path0=string_format("%s/%s/%d/%s",dal_base_path[0].c_str(),DNK_BIPHASIC_PREFIX,year,dal_groupid.c_str());
		//printf("e1\n");
		string dal_path1=string_format("%s/%s/%d/%s",dal_base_path[1].c_str(),DNK_BIPHASIC_PREFIX,year,dal_groupid.c_str());
		//printf("e2\n");
		format_path[0]=dal_path0+'/'+FORMAT_TABLE_FILENAME;
		format_path[1]=dal_path1+'/'+FORMAT_TABLE_FILENAME;
		return NO_ERROR;
	}

	eErrorTp dnk_biphasic_offset_cli::GetDataFromGzip(string & result,string & fname,TIME_T ts,string * format_path){

		struct tm * ptm=GMTIME(&ts);
		u32 year=ptm->tm_year+1900;
		string dal_path0=string_format("%s/%s/%d/%s",dal_base_path[0].c_str(),DNK_BIPHASIC_PREFIX,year,dal_groupid.c_str());
		string dal_path1=string_format("%s/%s/%d/%s",dal_base_path[1].c_str(),DNK_BIPHASIC_PREFIX,year,dal_groupid.c_str());
		string p0=dal_path0+'/'+fname;
		string p1=dal_path1+'/'+fname;
		format_path[0]=dal_path0+'/'+FORMAT_TABLE_FILENAME;
		format_path[1]=dal_path1+'/'+FORMAT_TABLE_FILENAME;

		//if (GetFileSize(p1.c_str()!=0)){
		if (UngzipFile((char*)p0.c_str(),result)==NO_ERROR){
			GPRINT(MEDIUM_LEVEL,"Success gunzip file %s\n",p0.c_str());
			return NO_ERROR;
		}
		else{
			GPRINT(NORMAL_LEVEL,"Error gunzip file %s, try gunzip reserved %s\n",p0.c_str(),p1.c_str());
			if (UngzipFile((char*)p1.c_str(),result)==NO_ERROR)
			{
				GPRINT(MEDIUM_LEVEL,"Success gunzip file %s\n",p1.c_str());
				remove((char*)p0.c_str());
				CopyFile((char*)p1.c_str(),(char*)p0.c_str());
				return NO_ERROR;
			}
			else{
				GPRINT(NORMAL_LEVEL,"Error gunzip file %s\n",p1.c_str());
				return ERROR;
			}
		}
		return ERROR;
	}
#endif
