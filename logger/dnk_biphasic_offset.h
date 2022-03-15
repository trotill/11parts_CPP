/*
 * dnk_biphasic_offset.h
 *
 *  Created on: 19 сент. 2019 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_LOGGER_DNK_BIPHASIC_OFFSET_H_
#define SRC_ENGINE_LOGGER_DNK_BIPHASIC_OFFSET_H_


#include "safe_logger_srvalgo.h"
#include "engine/algo/crc.h"
#include "engine/lib/11p_bin.h"
#ifdef _HIREDIS
#include "engine/proto/redis.h"
// public PrintT



#define DNK_BIPHASIC_BUILD_FILE_NAME "uncomplete"
#define METHOD_FILLDB 0
#define METHOD_COMPARE_FILES 1


class dnk_biphasic_offset: public algo_abstract,GprintT {
	public:
	dnk_biphasic_offset(Json::Value & config);
	~dnk_biphasic_offset(){
		GPRINT(NORMAL_LEVEL,"algo:dnk_biphasic_offset is down\n");
	}

	eErrorTp buildlog();
	eErrorTp tick(void);
	eErrorTp save(u8 * buf);
	private:
	class fninfo{
		public:
		TIME_T  start=0;
		TIME_T stop=0;
		u32  items=0;
		u32 crc32=0;
		char extension[4]={0};
	};

	string GenZipName(TIME_T & ts_start,TIME_T & ts_stop,u32  count,u32 & crc32){
		return string_format("%u_%u:%u[%08x].z",ts_start,ts_stop,count,crc32);
	}

	eErrorTp DB_RemoveOldestFilesByTime(u32 time_window){
		GPRINT(NORMAL_LEVEL,"DB_RemoveOldestFilesByTime not support use DB_RemoveOldestFilesByItems\n");
		return NO_ERROR;
	}

	eErrorTp RemoveItemInDB(TIME_T & key){
		if (rdb.command("DEL %u",key).first==NO_ERROR){
			if (rdb.command("ZREM ts %u",key).first==NO_ERROR){
				return NO_ERROR;
			}
		}
		return ERROR;
	}

	eErrorTp DB_RemoveUncompleteGroup(u32 und_cntr,u8 storid){
		if (storid==0){
			std::pair <eErrorTp,vector<string>> res=rdb.command("DEL u%u",und_cntr);
			return res.first;
		}
		else
			return NO_ERROR;
	}

	eErrorTp DB_ChangeUncomplete(u32 * und_cntr,u8 storid){
		if (storid==0){
			std::pair <eErrorTp,vector<string>> res=rdb.command("SET u %u",und_cntr[0]);
			return res.first;
		}
		else
			return NO_ERROR;
	}

	eErrorTp DB_AddUncomplData(string & data,u64 & ts,u32 * und_cntr){

		std::pair <eErrorTp,vector<string>> res=rdb.command("ZADD u%u %llu %s",und_cntr[0],ts,data.c_str());
		if (res.first==ERROR)
			return ERROR;


		//string s=string_format("");
		return NO_ERROR;
	}

	eErrorTp DB_RemoveOneOldestFile(void){
		u32 items;
		string fn;
		TIME_T ts_start;
		string rmfilepath;
		if (DB_GetOldestMessageFile(fn,ts_start,items)==ERROR)
			return ERROR;

		struct tm * ptm=GMTIME(&ts_start);
		u32 year=ptm->tm_year+1900;

		for (u8 n=0;n<total_storage;n++){
			  rmfilepath=string_format("%s/%s/%d/%s/%s",dal_base_path[n].c_str(),DNK_BIPHASIC_PREFIX,year,dal_groupid.c_str(),fn.c_str());
			  remove(rmfilepath.c_str());
			  if (existsSync(rmfilepath)==NO_ERROR){
				  GPRINT(NORMAL_LEVEL,"Error FS oldest %s not remove\n",rmfilepath.c_str());
				  return ERROR;
			  }
			 // GPRINT(NORMAL_LEVEL,"Remove oldest %s, total items %d\n",rmfilepath.c_str(),total_item_on_storage);
		 }
#if 1
		 if (RemoveItemInDB(ts_start)==NO_ERROR){
			 if (total_item_on_storage>=items){
				  total_item_on_storage-=items;
				  GPRINT(NORMAL_LEVEL,"Remove oldest %s, total items %d\n",fn.c_str(),total_item_on_storage);
				  return NO_ERROR;
			  }
			 else{
				  GPRINT(NORMAL_LEVEL,"Critical error, unsync total_item_on_storage\n");
				  return ERROR;
			 }
		 }
#else
		 GPRINT(NORMAL_LEVEL,"Remove oldest %s, total items %d\n",fn.c_str(),total_item_on_storage);
		 total_item_on_storage-=items;
#endif
		return NO_ERROR;
	}

	eErrorTp DB_RemoveOldestFilesByLowFreeSize(void){

	    sFolderInfo folder_info;
	    u32 try_max=10;
	    u32 try_cnt=0;

	    //printf("allow_remove_if_low_space %d\n",allow_remove_if_low_space);
	    if (allow_remove_if_low_space==false)
	    	return NO_ERROR;

	    if ((GetFolderInfo(&folder_info,(char*)dal_base_path[0].c_str())!=ERROR)&&(folder_info.FreeSizeMB<min_free_space_MB))	{
			while (folder_info.FreeSizeMB<min_free_space_MB){
				if (GetFolderInfo(&folder_info,(char*)dal_base_path[0].c_str())!=ERROR){
					GPRINT(NORMAL_LEVEL,"Folder %s free size (%dMB) < allowed min (%dMB), remove oldest\n",dal_base_path[0].c_str(),folder_info.FreeSizeMB,min_free_space_MB);
					if (DB_RemoveOneOldestFile()==ERROR)
						break;
				}
				try_cnt++;
				if (try_cnt>=try_max)
					break;
			}
	    }

		return NO_ERROR;
	}

	eErrorTp DB_RemoveOldestFilesByItems(u32 items_threshold,u32 remove_min){
		if (total_item_on_storage<items_threshold)
			return NO_ERROR;

		u32 items_window=items_threshold-remove_min;
		GPRINT(NORMAL_LEVEL,"Check oldest files total %u items_threshold %u items_window %u\n",total_item_on_storage,items_threshold,items_window);
		while (total_item_on_storage>items_window){
			DB_RemoveOneOldestFile();
		}
		return NO_ERROR;
	}
	eErrorTp DB_GetOldestMessageFile(string & filename,TIME_T & ts_start,u32 & items){
		std::pair <eErrorTp,vector<string>> res=rdb.command("SORT ts LIMIT 0 1 ASC");
		items=0;
		filename="";
		if ((res.first!=ERROR)&&(res.second.size()==1)){
			std::pair <eErrorTp,vector<string>> r=rdb.command("HMGET %s te cn ext crc",res.second[0].c_str());
			if (r.first!=ERROR){
				ts_start=stoul(res.second[0]);
				TIME_T ts_stop=stoul(r.second[0]);
				u32 crc32=HexToInt(r.second[3]);
				items=stoul(r.second[1]);
				filename=GenZipName(ts_start,ts_stop,items,crc32);
			}
			else
				return ERROR;
		}
		else
			return ERROR;

		return NO_ERROR;
	}

	eErrorTp DB_GetMessagesCount(u32 & count){
		std::pair <eErrorTp,vector<string>> res=rdb.command("SORT ts");
		std::pair <eErrorTp,vector<string>> resh;
		eErrorTp err=NO_ERROR;
		count=0;
		if (res.first==NO_ERROR){
			for (u32 n=0;n<res.second.size();n++){
				resh=rdb.command("HMGET %s cn",res.second[n].c_str());
				if (resh.second.size()==0)
					return ERROR;

				if (resh.first==NO_ERROR)
					count+=stoul(resh.second[0]);
				else{
					err=ERROR;
					break;
				}
			}
		}
		return err;
	}

	eErrorTp AddToDB_METHOD_FILLDB(char * fname,TIME_T & start,TIME_T & stop,u32 & count, char * extension, u32 &crc){
		eErrorTp err=NO_ERROR;

		if ((rdb.command("ZADD ts %u %u",start,start).first==ERROR)
		  ||(rdb.command("hmset %u te %u cn %u ext %s crc %08x",start,stop,count,extension,crc).first==ERROR)){
				GPRINT(NORMAL_LEVEL,"Error add file info %s\n",fname);
				err=ERROR;
		}
		return err;
	}

	eErrorTp AddToDB_METHOD_COMPARE_FILES(u8 stid,char * fname,TIME_T & start,TIME_T & stop,u32 & count, char * extension, u32 &crc){
		eErrorTp err=NO_ERROR;

		if ((rdb.command("SADD s%d %u",stid,start).first==ERROR)
			||(rdb.command("SET %d:%u %s",stid,start,fname).first==ERROR)){
				GPRINT(NORMAL_LEVEL,"Error add file info %s\n",fname);
				err=ERROR;
		}
		return err;
	}

	eErrorTp FinalizeUncompleteFiles(u8 stid,string year){
		dal_path[stid]=string_format("%s/%s/%s/%s",dal_base_path[stid].c_str(),DNK_BIPHASIC_PREFIX,year.c_str(),dal_groupid.c_str());
		dal_format_table_file[stid]=dal_path[stid]+'/'+FORMAT_TABLE_FILENAME;

		string unc_file;
		if (SearchUncompleteLast(dal_path[stid],stid,unc_file)!=ERROR){
			GPRINT(NORMAL_LEVEL,"Try finalize uncomplete file %s\n",unc_file.c_str());
			if (FinalizeFile(dal_path[stid],unc_file,stid)==NO_ERROR)
				GPRINT(NORMAL_LEVEL,"Finalize ok file %s restored\n",unc_file.c_str());
		}
		printf("unc_file %s stid %d dal_path %s \n",unc_file.c_str(),stid,dal_path[stid].c_str());
		stor[stid].select_file=string_format("%s/%s%d",dal_path[stid].c_str(),DNK_BIPHASIC_BUILD_FILE_NAME,undefined_counter[stid]);
		file[stid].file_set_file_name(stor[stid].select_file,true);
		RemoveTempotaryFragmentAll(stid);
		return NO_ERROR;
	}
	eErrorTp DB_FillAndScanDAL(u8 stid,u8 method){
		eErrorTp err=NO_ERROR;
		DIR *dp;
		DIR *dt;
		struct dirent *entry;
		u32 numberOfEntries=0;

		string dir=string_format("%s/%s/",dal_base_path[stid].c_str(),DNK_BIPHASIC_PREFIX);
		string dir_dt;
		string year="";
		GPRINT(NORMAL_LEVEL,"Start fill DB for stor %d\n",stid);
		if ((dp = opendir(dir.c_str())) != NULL) {
					while((entry = readdir(dp)) != NULL) {

						if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
							continue;

						year=entry->d_name;
						dir_dt=string_format("%s/%s/%s/%s",dal_base_path[stid].c_str(),DNK_BIPHASIC_PREFIX,year.c_str(),dal_groupid.c_str());
						if (method==METHOD_COMPARE_FILES){
							FinalizeUncompleteFiles(stid,year);
						}
						if ((dt = opendir(dir_dt.c_str())) != NULL) {
							while((entry = readdir(dt)) != NULL) {
								fninfo fn;
								if (ParseDalFilename(entry->d_name, fn)==NO_ERROR){
									if (method==METHOD_FILLDB){
										if (AddToDB_METHOD_FILLDB(entry->d_name,fn.start,fn.stop,fn.items, fn.extension,fn.crc32)==ERROR)
											err=ERROR;
									}else{
										if (AddToDB_METHOD_COMPARE_FILES(stid,entry->d_name,fn.start,fn.stop,fn.items, fn.extension,fn.crc32)==ERROR)
											err=ERROR;
									}
								}
							}
							closedir(dt);
						}
						numberOfEntries++;
					}
					closedir(dp);
				}
		GPRINT(NORMAL_LEVEL,"Finish fill DB for stor %d\n",stid);
		return err;
	}
	eErrorTp ParseDalFilename(char * fname, fninfo & fn){
		///www/pages/sys/JDB/2019/sensors/1569418523_1569418529:50[90051c47].z
		buffer buf((u32)strlen(fname)+1);
		strcpy((char*)buf.p(),fname);
		int flen=(int)strlen(fname)-1;
		u32 start=0;
		u32 counter=0;
		for (int n=flen;n>=0;n--){
			if ((buf.p()[n]=='.')){
				if (counter!=0)
					break;
				buf.p()[n]=0;
				start=n+1;
				strcpy((char*)fn.extension,(char*)&buf.p()[start]);
				counter++;
			}
			else
			if ((buf.p()[n]==']')){
				if (counter!=1)
					break;
				buf.p()[n]=0;
				counter++;
			}
			else
			if ((buf.p()[n]=='[')){
				if (counter!=2)
					break;
				buf.p()[n]=0;
				start=n+1;
				counter++;
				fn.crc32=HexToInt((char*)&buf.p()[start]);
			}
			else
			if ((buf.p()[n]==':')){
				if (counter!=3)
					break;
				buf.p()[n]=0;
				start=n+1;
				fn.items=atoi((char*)&buf.p()[start]);
				counter++;
			}
			else
			if ((buf.p()[n]=='_')){
				if ((counter!=4))
					break;
				buf.p()[n]=0;
				start=n+1;
				fn.stop=atol((char*)&buf.p()[start]);
				counter++;
			}
			else
			if ((buf.p()[n]=='/')){
				if ((counter!=5))
					break;
				buf.p()[n]=0;
				start=n+1;
				fn.start=atol((char*)&buf.p()[start]);
				counter++;
				break;

			}
			else
			if ((n==0)){
				if ((counter!=5))
					break;
				fn.start=atol((char*)&buf.p()[n]);
				counter++;
				break;
			}
		}

		if (counter==6)
			return NO_ERROR;
		else
			return ERROR;
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
	eErrorTp ExtractFormat(string & json, string & value){
		Json::Value root;
		Json::Value rvalue;
		Json::Reader reader;
		//Json::FastWriter fwr;
		//Json::StyledWriter sfwr;
		//string sumstr="";
		//printf("m1\n");
		value="";
		//id="undefined";
		if (reader.parse(json,root)){
			//printf("m2\n");
			if ((root.isMember("t"))&&(root.isMember("d"))){
				//u64 t=root["t"].asUInt64();
				//rvalue[0]=t;
				//rvalue[1][1][2]="test";
				//rvalue[1][1][3]="test1";
				//rvalue[1][2]=1;
				//u8 crc8=0;
				//printf("m3\n");
				ExtractFormatRecurs(root,rvalue);
				value=FastWriteJSON(rvalue);
				//cout << "Format "<<sfwr.write(rvalue)<<endl;
				//printf("m4\n");
			//	crc8=CalcCRC8(value.c_str(),value.size(),0xff);
				//printf("m5\n");
				//id=sf("%02x",crc8);

			}
		}
		else{
			GPRINT(NORMAL_LEVEL,"ExtractID_N_Value error json parse\n");
			return ERROR;
		}
		return NO_ERROR;
	}

	eErrorTp ExtractValueRecurs(Json::Value & root,Json::Value & arrvalue){
			u32 nidx=0;
			for (const auto& key : root.getMemberNames()){
				if (root[key].isObject()){
					ExtractValueRecurs(root[key],arrvalue[nidx]);
				}
				else{
					arrvalue[nidx]=root[key];
				}
				nidx++;
			}
			return NO_ERROR;
	}
//проверка наличия FORMAT_TABLE_FILENAME
//если таблицы формата нет, то создает таблицу
//заодно создает папки
	eErrorTp CheckAndSaveFormat(TIME_T & t,string & json){
		struct tm * ptm=GMTIME(&t);
		u32 year=ptm->tm_year+1900;
		if (year!=old_year)
			format_table_found=false;

		if ((format_table_found==false)){

			for (u8 n=0;n<total_storage;n++){
				//dal_path[n]=string_format("%s/%s/%d/%s",dal_base_path[n].c_str(),DNK_BIPHASIC_PREFIX,year,dal_groupid.c_str());
				//dal_format_table_file[n]=dal_path[n]+'/'+FORMAT_TABLE_FILENAME;

				FinalizeUncompleteFiles(n,IntToStr(year));
				GPRINT(NORMAL_LEVEL,"Search format file %s\n",dal_format_table_file[n].c_str());
				if (existsSync(dal_format_table_file[n])==ERROR){
						string format;
						if (ExtractFormat(json, format)==NO_ERROR){
							GPRINT(NORMAL_LEVEL,"Try create format file %s\n",dal_format_table_file[n].c_str());
							MkPath(dal_path[n].c_str(),0xffffffff);

							u32 crc32=Crc32Buf((u8*)dal_format_table_file[n].c_str(),dal_format_table_file[n].size());
							if (WriteStringFile((char*)dal_format_table_file[n].c_str(),format)==ERROR){
								GPRINT(NORMAL_LEVEL,"Error create file %s\n",dal_format_table_file[n].c_str());
							}
							else{
								string crc_table=dal_format_table_file[n]+".crc";
								string crc_str=string_format("%08x",crc32);
								WriteStringFile((char*)crc_table.c_str(),crc_str);
								string copy_table=dal_format_table_file[n]+"_copy";
								WriteStringFile((char*)copy_table.c_str(),format);
								crc_table=copy_table+".crc";
								WriteStringFile((char*)crc_table.c_str(),crc_str);
								GPRINT(NORMAL_LEVEL,"Created format file %s\n",dal_format_table_file[n].c_str());
								format_table_found=true;
							}
						}
						else{
							GPRINT(NORMAL_LEVEL,"Error create format file %s,bad json\n",dal_format_table_file[n].c_str());
						}
				}
				else{
					format_table_found=true;
					GPRINT(NORMAL_LEVEL,"Found format file %s\n",dal_format_table_file[n].c_str());
				}
			}

		}
		old_year=year;
		return NO_ERROR;
	}

	eErrorTp TermWriteLogic(void){
		eErrorTp  err=NO_ERROR;
		for (u8 n=0;n<total_storage;n++){
			file[n].file_sync();
			if (FinalizeFile(dal_path[n],stor[n].select_file,n)==ERROR)
				err=ERROR;
			stor[n].select_file=string_format("%s/%s%d",dal_path[n].c_str(),DNK_BIPHASIC_BUILD_FILE_NAME,undefined_counter[n]);
			DB_ChangeUncomplete(undefined_counter,n);
			file[n].file_set_file_name(stor[n].select_file,true);

			GPRINT(NORMAL_LEVEL,"Create new files %s \n",stor[n].select_file.c_str());

		}
		//printf("\n");
		return err;
	}

	eErrorTp RemoveTempotaryFragment(u8 start, u8 stop,u8 storid){
		string rmname;

		for (u32 n=start;n<stop;n++){
				rmname=string_format("%s/%s%d",dal_path[storid].c_str(),DNK_BIPHASIC_BUILD_FILE_NAME,n);
				if (existsSync(rmname)==NO_ERROR){
					remove(rmname.c_str());
					GPRINT(MEDIUM_LEVEL,"Remove %s\n",rmname.c_str());
				}
				DB_RemoveUncompleteGroup(n,storid);
		}
		return NO_ERROR;
	}
	eErrorTp SearchUncompleteLast(string base_path,u8 storid,string & uncompl_file){
		string name;
		struct stat statbuf;
		TIME_T min_time=std::numeric_limits<TIME_T>::max();
		//int min_n=std::numeric_limits<int>::max();
		TIME_T max_time=0;
		//int max_n=0;
		uncompl_file="";
		for (u32 n=0;n<undefined_counter_max[storid];n++){

			name=string_format("%s/%s%d",base_path.c_str(),DNK_BIPHASIC_BUILD_FILE_NAME,n);
			//printf("test unc %s\n",name.c_str());
			if (lstat((char*)name.c_str(), &statbuf)!=0){
				continue;
			}
			//printf("found unc %s\n",name.c_str());
			//printf("min_time=%d tbuf.st_ctim.tv_sec=%d \n",min_time,statbuf.st_ctim.tv_sec);
			//printf("max_time=%d statbuf.st_ctim.tv_sec=%d \n",max_time,statbuf.st_ctim.tv_sec);
			if (min_time>statbuf.st_ctim.tv_sec){
				min_time=statbuf.st_ctim.tv_sec;
				//min_n=n;
			}
			if (max_time<statbuf.st_ctim.tv_sec){
				max_time=statbuf.st_ctim.tv_sec;
				uncompl_file=name;
				//max_n=n;
			}
		}
		//printf("min_time=%d max_time=%d\n",min_n,max_n);

		if (uncompl_file.size()==0)
			return ERROR;
		else
			return NO_ERROR;
		//if ((min_time==std::numeric_limits<int>::max())||(max_time==0)){
		//	uncompl_file="";
		//	return ERROR;
		//}
		//else{
		//	uncompl_file=string_format("%s/%s%d",base_path.c_str(),DNK_BIPHASIC_BUILD_FILE_NAME,max_n);
		//	return NO_ERROR;
		//}
	}
	void RemoveTempotaryFragmentAll(u8 storid){
			RemoveTempotaryFragment(0,undefined_counter_max[storid],storid);
	}
	eErrorTp RemoveTempotaryFragmentLogic(u8 storid){
		undefined_counter[storid]++;

		if (undefined_counter[storid]>=undefined_counter_max[storid]){
			RemoveTempotaryFragment(0,undefined_counter_max[storid]/2,storid);
			undefined_counter[storid]=0;
		}
		if (undefined_counter[storid]==(undefined_counter_max[storid]/2)){
			RemoveTempotaryFragment(undefined_counter_max[storid]/2,undefined_counter_max[storid],storid);
		}

		return NO_ERROR;
	}

	eErrorTp FinalizeFile(string dbpath,string & filepath,u8 storid){
		string data="";
		u32 crc32=0;
		if (ReadStringFile((char*)filepath.c_str(),data)==NO_ERROR){
			Json::Value data_root;
			Json::Reader read;

			data[data.size()-1]=']';
			//printf("m1\n");
			if (!read.parse(data,data_root)){
				GPRINT(NORMAL_LEVEL,"Error parse %s\n",filepath.c_str());
				RemoveTempotaryFragmentLogic(storid);
				return ERROR;
			}
			//printf("m2\n");
			if ((data_root[0].isArray()==false)||(data_root[data_root.size()-1].isArray()==false)){
				GPRINT(NORMAL_LEVEL,"Error parse %s\n",filepath.c_str());
				RemoveTempotaryFragmentLogic(storid);
				return ERROR;
			}
			//printf("m3\n");
			Json::Value header_root;
			if (ReadJsonFormatFile(header_root,dal_format_table_file)==ERROR){
				RemoveTempotaryFragmentLogic(storid);
				return ERROR;
			}
			//printf("m4\n");
			Json::Value result_root;
			MakeObjectFromDataAndHeader(data_root,header_root,result_root,true);
			if ((!result_root.isMember("t"))||(!result_root["t"].isArray())){
				RemoveTempotaryFragmentLogic(storid);
				return ERROR;
			}
			//printf("m5\n");
			GPRINT(MEDIUM_LEVEL,"Parse json ok %s items %d\n",filepath.c_str(),result_root["t"].size());
			TIME_T ts_start=result_root["t"][0].asInt();
			TIME_T ts_stop=result_root["t"][result_root["t"].size()-1].asInt();
			//printf("Compl data %s size (->%d<-)\n",data.c_str(),data.size());
			string compressed_data = gzip::compress(data.c_str(), data.size());
		//	printf("m6\n");
			if (dal_enable_crc)
				crc32=Crc32Buf((u8*)compressed_data.c_str(),compressed_data.size());
			//Search first TS

			//string finfile=string_format("%s/%d_%d:%d[%08x].z",dbpath.c_str(),ts_start,ts_stop,result_root["t"].size(),crc32);
			u32 count=result_root["t"].size();
			string finfile=dbpath+'/'+GenZipName(ts_start,ts_stop,count,crc32);
			GPRINT(NORMAL_LEVEL,"Finalize %s to %s\n",filepath.c_str(),finfile.c_str());
			//для использования пространства из под undefined, нужно сначала удалить этот файл, а потом писать zip
			RemoveTempotaryFragmentLogic(storid);
			//GPRINT(NORMAL_LEVEL,"Remove %s\n",filepath.c_str());
			if (WriteStringFile((char*)finfile.c_str(),compressed_data)==ERROR){
				GPRINT(NORMAL_LEVEL,"Error write %s\n",finfile.c_str());
				if (WriteStringFile((char*)filepath.c_str(),data)==NO_ERROR){
					GPRINT(NORMAL_LEVEL,"File %s restored\n",filepath.c_str());
				}
				else{
					GPRINT(NORMAL_LEVEL,"Error restore %s, data is lost!!!!\n",filepath.c_str());
				}
			}
			else{
				if (storid==0){
					if (AddToDB_METHOD_FILLDB((char*)finfile.c_str(),ts_start,ts_stop,count, "z",crc32)==NO_ERROR){
						total_item_on_storage+=count;
					}
				}
			}

		}
		else{
			GPRINT(NORMAL_LEVEL,"Error read file %s\n",(char*)filepath.c_str());
			return ERROR;
		}
		return NO_ERROR;
	}
	eErrorTp ExtractValue(string & json, string & value,TIME_T & extract_tstamp,u64 & extract_ts64){
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
				CheckAndSaveFormat(extract_tstamp,json);
				ExtractValueRecurs(root,rvalue);
				value=FastWriteJSON(rvalue);
				delCh(value, (char)0x0a);
			}
		}
		else{
			GPRINT(NORMAL_LEVEL,"ExtractID_N_Value error json parse\n");
			return ERROR;
		}
		return NO_ERROR;
	}
	eErrorTp clean_oldest_block(u32 stid,u32 cnt);
	eErrorTp move_up_idx();
	eErrorTp sync_blocks();
	eErrorTp analytic(void);
	u32 calc_lines_in_segment(searched_file_list * block );
	eErrorTp seg_info(u8 stid);
	static const u8 total_storage=2;
	string dal_prefix="-dal.log";
	string dal_base_path[total_storage]={"/www/pages/sys","/www/pages/sys_ex"};
	string dal_path[total_storage];
	string dal_format_table_file[total_storage];

	u32 dal_max_blocks=10000;
	u32 dal_lines_in_block=100;
	int dal_autofin_delta_s=3600;
	u32 dal_remove_window=0;
	bool dal_enable_crc=false;
	u32 dal_remove_treshold=0;
	string dal_groupid="undefined";
	bool dal_compress_enable=true;
	bool dal_cache_stings=true;
	u32 source_logid=0;
	u32 line_counter=0;
	u32 total_item_on_storage=0;
	int sync_time=1;
	u32 offset_time=1;
	filesys file[total_storage];
	TIME_T old_ts=0,old_ts2=0,new_ts=0;
	string tmpdir;
	bool format_table_found=false;
	u32 old_year=0;
	u32 undefined_counter[total_storage]={0,0};
	u32 undefined_counter_max[total_storage]={20,20};
	TIME_T last_save_ts[total_storage]={0,0};
	redis rdb;
	u32 min_free_space_MB=20;
	bool allow_remove_if_low_space=false;
	class{
		public:
		//u32  count=0;
		//u32  min_idx=0xffffffff;
		//u32  max_idx=0;
		//u32  total_size=0;
		//vector<searched_file_list> files;
		//searched_file_list * last_block=NULL;
		//searched_file_list * first_block=NULL;
		//u32 total_lines=0;
		string select_file;

	} stor[total_storage];

};

#endif
#endif /* SRC_ENGINE_LOGGER_DNK_BIPHASIC_OFFSET_H_ */
