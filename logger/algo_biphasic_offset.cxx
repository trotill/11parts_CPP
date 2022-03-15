/*
 * safe_logger_srvalgo.cxx
 *
 *  Created on: 14 янв. 2019 г.
 *      Author: root
 */


#include "algo_biphasic_offset.h"
#include "engine/lib/11p_time.h"

algo_biphasic_offset::algo_biphasic_offset(eDebugTp debug_lvl,string * aal_path,u32 aal_max_blocks,u32 aal_lines_in_block,u32 aal_sync_time,u32 aal_offset_time,u8 style,string tmpdir){
		SourceStr="algo_biphasic";
		debug_level=debug_lvl;
		ObjPref="slogger";

		MkPath(aal_path[0].c_str(), 0xffffffff);
		MkPath(aal_path[1].c_str(), 0xffffffff);
		aal_save_style=style;
		this->aal_path[0]=aal_path[0];
		this->aal_path[1]=aal_path[1];
		this->aal_max_blocks=aal_max_blocks;
		this->aal_lines_in_block=aal_lines_in_block;
		this->sync_time=aal_sync_time;
		this->offset_time=aal_offset_time;
		this->tmpdir=tmpdir;
		if (aal_max_blocks>99999) {
			GPRINT(NORMAL_LEVEL,"Error:Not correct aal_max_blocks!!!, delay 10s...\n");
			sleep(10);
			return;
		}


		analytic();
		GPRINT(NORMAL_LEVEL,"algo:biphasic_offset started\n");
		//file[0].file_set_file_name;
		//=file[1].file_set_file_name

	}

eErrorTp algo_biphasic_offset::buildlog()
	{
		//seg_info(0);

		GPRINT(NORMAL_LEVEL,"Build log\n");
		if (stor[0].count==0){
			GPRINT(NORMAL_LEVEL,"Not build log, 0 blocks\n");
			return ERROR;
		}



		vector<u32> sort;
		//filesys file;
		u32 min;
		int m=-1;//stor[0].min_idx;
	//	sort.push_back(stor[0].min_idx);
		u32 imin=0;
		for (u32 i=0;i<stor[0].count;i++){
			//GPRINT(NORMAL_LEVEL,"%d in %d [%s]\n",i,stor[0].count,stor[0].files[i].name.c_str());
			min=0xffffffff;
			for (u32 z=0;z<stor[0].count;z++){//
				if ((stor[0].files[z].idx<min)&&((int(stor[0].files[z].idx)>m))){
					min=stor[0].files[z].idx;
					imin=z;
				}
			}
			//printf("min %d,imin %d,idx %d\n",min,imin,stor[0].files[imin].idx);
			sort.push_back(imin);
			m=min;
			//sort.push_back(i);
		}



		//for (u32 z=0;z<stor[0].files.size();z++){
		//	printf("FILE %s\n",stor[0].files[sort[z]].fname.c_str());
		//}
		//file.file_set_file_name(tmpdir+"/slog.log",false);
		string fname=tmpdir+"/";
		fname+=tmpfile;

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
		return NO_ERROR;
	}
	eErrorTp algo_biphasic_offset::tick(void){

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
			//TIME_T ts=TIME(NULL);
			if ((new_ts-old_ts2)>offset_time){
				file[1].file_sync();
				old_ts2=0;
				GPRINT(MEDIUM_LEVEL,"SYNC2\n");
			}
		}
		if (aal_compress_enable){

		}
		return NO_ERROR;
	}
	eErrorTp algo_biphasic_offset::save(u8 * buf){

		GPRINT(MEDIUM_LEVEL,"[%s]\n",buf);

		string stb=(char*)buf;
		string stbuf;
		switch (aal_save_style){
			case AAL_SAVE_RAW:
				stbuf=stb+"\n";
				break;
			case AAL_SAVE_RAW_WDATE:{
				struct tm  times;
				stringstream strdate;
				GetDataTime(&times,NULL);
				TimeToString(times,strdate);
				stbuf=strdate.str()+" "+stb+"\n";
			}
			break;
			case AAL_SAVE_QUOTES_COMMA:{
				replace(stb.begin(),stb.end(),'"','\'');
				stbuf=string_format("\"%s\",\r\n",stb.c_str());
			}
			break;
			default:{
				replace(stb.begin(),stb.end(),'"','\'');
				stbuf=string_format("\"%s\",\r\n",stb.c_str());
			}
		}
		//string sttb;
	//	sttb << "\"" << replace(stbuf.begin(),stbuf.end(),'"','\'') << "\",";


		file[0].file_add_string(stbuf);
		file[1].file_add_string(stbuf);

		//printf("time_last %d\n",time_last);


		//file_sync
		line_counter++;
		if (line_counter>=aal_lines_in_block){
			line_counter=0;
			u32 min=stor[0].min_idx;
			u32 max=stor[0].max_idx;
			printf("min %d max %d\n",min,max);

			if (aal_max_blocks!=0)
				move_up_idx();
			else{
				for (u8 n=0;n<total_storage;n++){
					string fn;
					file[n].write_cached();
					file[n].file_close();
					fn=string_format("%s/%010d%s",aal_path[n].c_str(),max,aal_prefix.c_str());
					remove(fn.c_str());
					file[n].file_sync();
				}

			}


			stor[0].select_file=string_format("%s/%010d%s",aal_path[0].c_str(),max,aal_prefix.c_str());
			file[0].file_set_file_name(stor[0].select_file,aal_cache_stings);
			stor[1].select_file=string_format("%s/%010d%s",aal_path[1].c_str(),max,aal_prefix.c_str());
			file[1].file_set_file_name(stor[1].select_file,aal_cache_stings);
		}

		return NO_ERROR;
	}


	eErrorTp algo_biphasic_offset::clean_oldest_block(u32 stid,u32 cnt){
			string fn;
			u32 i=0;
			for (u32 n=0;n<cnt;n++){
				  fn=string_format("%s/%010d%s",aal_path[stid].c_str(),stor[stid].min_idx+n,aal_prefix.c_str());
				  //printf("try remove %s\n",fn.c_str());
				  GPRINT(NORMAL_LEVEL,"Try remove calculated oldest %s\n",fn.c_str());
				  if (SearchFile(fn.c_str())==NO_ERROR){
					  remove(fn.c_str());
					  GPRINT(NORMAL_LEVEL,"Remove oldest%s\n",fn.c_str());
					  stor[stid].min_idx+=n+1;
					  stor[stid].count--;
				  }
			}
		return NO_ERROR;
	}
	eErrorTp algo_biphasic_offset::move_up_idx(){
		stor[0].max_idx++;
		stor[1].max_idx++;
		stor[0].count++;
		stor[1].count++;

		if ((stor[0].count>=(aal_max_blocks+50))||(stor[1].count>=(aal_max_blocks+50))){
				GPRINT(NORMAL_LEVEL,"Detected big break very bad situation, resync all data!!!\n");
				sync_blocks();
		}

		//printf("stor[stid].max_idx(%d)-stor[stid].min_idx(%d)=%d stor[0].count=%d\n",stor[0].max_idx,stor[0].min_idx,stor[0].max_idx-stor[0].min_idx,stor[0].count);
		if ((stor[0].max_idx>=aal_max_blocks)&&(stor[0].count>aal_max_blocks)){
			clean_oldest_block(0,stor[0].count-aal_max_blocks);

		}
		if ((stor[1].max_idx>=aal_max_blocks)&&(stor[1].count>aal_max_blocks)){
			clean_oldest_block(1,stor[1].count-aal_max_blocks);

		}
		return ERROR;
	}




	eErrorTp algo_biphasic_offset::sync_blocks(){
			seg_info(0);
			seg_info(1);
			u32 idp=0;
		if ((stor[0].count!=stor[1].count)||
				 (stor[0].max_idx!=stor[1].max_idx)||
				 (stor[0].min_idx!=stor[1].min_idx)||
				 (stor[0].total_size!=stor[1].total_size)||
				 (stor[0].total_lines!=stor[1].total_lines)){
				GPRINT(NORMAL_LEVEL,"Logs not identical, bad situation!!!\n");

				u8 src=0;
				u8 dst=1;
				string destfn;

			for (u32 k=0;k<2;k++){
				src=k;
				dst=(~k)&1;

				for (u32 n=0;n<stor[src].count;n++){
					bool skip=false;
					for (u32 z=0;z<stor[dst].count;z++){
						if ((stor[src].files[n].name==stor[dst].files[z].name)&&
							(stor[src].files[n].st.st_size==stor[dst].files[z].st.st_size)){
							skip=true;
							break;
						}
					}
					if (skip==false){
						destfn=string_format("%s/%s",aal_path[dst].c_str(),stor[src].files[n].name.c_str());
						CopyFile((char*)stor[src].files[n].fname.c_str(), (char*)destfn.c_str());

						GPRINT(NORMAL_LEVEL,"Backup file %s to %s\n",stor[src].files[n].fname.c_str(),(char*)destfn.c_str());
					}
				}
				seg_info(dst);
			}
		}
		else {
				GPRINT(NORMAL_LEVEL,"Logs are identical, good situation!!!\n");
		}

		//поиск разрывов
		if (stor[0].count>(aal_max_blocks+10)){
			GPRINT(NORMAL_LEVEL,"Detect mistake file name or breaks, found and delete!!!\n");
			u32 del_total=stor[0].count-aal_max_blocks-1;
			u32 min_idx=stor[0].min_idx;
			u32 total_block=stor[0].count;
			remove(stor[0].first_block->fname.c_str());
			GPRINT(NORMAL_LEVEL,"Remove %s!!!\n",stor[0].first_block->fname.c_str());
			remove(stor[1].first_block->fname.c_str());
			GPRINT(NORMAL_LEVEL,"Remove %s!!!\n",stor[1].first_block->fname.c_str());

			for (u32 d=0;d<del_total;d++){
				u32 min=0xffffffff;
				u32 min_id=0;
				for (u32 n=0;n<total_block;n++){
					if ((stor[0].files[n].idx>min_idx)&&(stor[0].files[n].idx<min)){
						min=stor[0].files[n].idx;
						min_id=n;
					}
				}
				min_idx=min;
				remove(stor[0].files[min_id].fname.c_str());
				GPRINT(NORMAL_LEVEL,"Remove %s!!!\n",stor[0].files[min_id].fname.c_str());
				remove(stor[1].files[min_id].fname.c_str());
				GPRINT(NORMAL_LEVEL,"Remove %s!!!\n",stor[1].files[min_id].fname.c_str());
			}
			seg_info(0);
			seg_info(1);
		}

		return NO_ERROR;
}

	eErrorTp algo_biphasic_offset::analytic(void){

		sync_blocks();
		GPRINT(NORMAL_LEVEL,"min_idx %d max_idx %d\n",stor[0].min_idx,stor[1].max_idx);
		if (stor[0].first_block==NULL){
			GPRINT(NORMAL_LEVEL,"Log not found, create new chain\n");
			stor[0].select_file=string_format("%s/0000000000%s",aal_path[0].c_str(),aal_prefix.c_str());
			file[0].file_set_file_name(stor[0].select_file,aal_cache_stings);
			stor[1].select_file=string_format("%s/0000000000%s",aal_path[1].c_str(),aal_prefix.c_str());
			file[1].file_set_file_name(stor[1].select_file,aal_cache_stings);
			line_counter=0;
			return NO_ERROR;
		}

		string fname;
		if ((stor[0].min_idx==stor[1].min_idx)&&(stor[0].max_idx==stor[1].max_idx)){

			line_counter=calc_lines_in_segment(stor[0].last_block);
			printf("line_counter %d\n",line_counter);
			if (line_counter<aal_lines_in_block){
				stor[0].select_file=stor[0].last_block->fname;
				stor[1].select_file=stor[1].last_block->fname;
				file[0].file_set_file_name(stor[0].last_block->fname,aal_cache_stings);
				file[1].file_set_file_name(stor[1].last_block->fname,aal_cache_stings);
			}
			else{
				//u32 max_idx=stor[0].max_idx;
				GPRINT(NORMAL_LEVEL,"Very good, the last block is full\n");

				u32 min=stor[0].min_idx;
				u32 max=stor[0].max_idx;
				move_up_idx();

				stor[0].select_file=string_format("%s/%010d%s",aal_path[0].c_str(),max,aal_prefix.c_str());
				file[0].file_set_file_name(stor[0].select_file,aal_cache_stings);
				stor[1].select_file=string_format("%s/%010d%s",aal_path[1].c_str(),max,aal_prefix.c_str());
				file[1].file_set_file_name(stor[1].select_file,aal_cache_stings);
				line_counter=0;
				//file[1].file_set_file_name(stor[1].last_block->fname);
						//stor[0].last_block->fname;
			}
		}
		else
		{

		}
		return NO_ERROR;
	}



	u32 algo_biphasic_offset::calc_lines_in_segment(searched_file_list * block ){
		//stor[stid].last_block

		if (SearchFile(block->fname.c_str())==ERROR)
			return 0;

		ifstream blk(block->fname.c_str());
		stringstream strStream;

		strStream << blk.rdbuf();
        string mask="\n\r";
        u32 count=0;
        for (std::size_t pos = 0; pos < strStream.str().size(); pos += mask.size())
         {
           pos = strStream.str().find(mask, pos);
           if (pos != std::string::npos)
           {
             ++count;
           }
           else
           {
             break;
           }
         }

		return count;
	}


	eErrorTp algo_biphasic_offset::seg_info(u8 stid){


		filesys fs;
		//aal_prefix="settings";

		stor[stid].min_idx=0xffffffff;
		stor[stid].max_idx=0;
		stor[stid].select_file="";
		stor[stid].files.clear();
		stor[stid].count=fs.SearchFilesInDir((char*)aal_path[stid].c_str(),stor[stid].files,aal_prefix);
		stor[stid].total_lines=stor[stid].count*aal_lines_in_block;
		stor[stid].total_size=0;
		char sdig[11]={0};
		u32 val=0;
		for (u32 i=0;i<stor[stid].files.size();i++){
			//printf("---%s size %d\n",stor[stid].files[i].fname.c_str(),(u32)stor[stid].files[i].st.st_size);
			stor[stid].total_size+=stor[stid].files[i].st.st_size;
			memcpy(sdig,stor[stid].files[i].name.c_str(),10);
			//printf("DIG %s\n",sdig);
			val=atoi(sdig);
			stor[stid].files[i].idx=val;
			if (val<stor[stid].min_idx){
				stor[stid].min_idx=val;
				stor[stid].first_block=&stor[stid].files[i];
			}
			if (val>stor[stid].max_idx){
				stor[stid].max_idx=val;
				stor[stid].last_block=&stor[stid].files[i];
			}

		}
		if (stor[stid].min_idx==0xffffffff)
			stor[stid].min_idx=0;

		if ((stor[stid].first_block!=NULL)&&(stor[stid].last_block==NULL)){
			stor[stid].last_block=stor[stid].first_block;
		}

		printf("total size %d min %d max %d total_block %d\n",stor[stid].total_size,stor[stid].min_idx,stor[stid].max_idx,stor[stid].count);
	//	std::experimental::filesystem::create_directories("C:\\temp");
		//u32 & count,u32 & min_idx,u32 & max_idx,u32 & total_size

		return NO_ERROR;
	}

