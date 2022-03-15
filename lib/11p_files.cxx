/*
 * files.cxx
 *
 *  Created on: 17 мая 2018 г.
 *      Author: root
 */

#include "11p_files.h"
#include "11p_string.h"
#include "11p_bin.h"
#include "11p_process.h"
#include "engine/gzip/decompress.hpp"
#include "engine/gzip/compress.hpp"
#include <libmount/libmount.h>



#define MAX_SIZE_STRING_FILES 2000000
/**
Brief: gets information about the status of drive
Params: storage - pointer on storage info structure
		MountDev - mount storage
		MountPoint - mount point
Return: NO_ERROR
*/

bool checkMountStatus(char * dev,char * mPoint){
	struct libmnt_fs *fs;
	struct libmnt_context *cxt = mnt_new_context();
	int rc=0;
	mnt_context_set_source(cxt, dev);//dev/sda
	mnt_context_set_target(cxt, mPoint);//mountpoint
	fs = mnt_context_get_fs(cxt);
	if (fs != NULL)
	{
		if (mnt_context_is_fs_mounted(cxt, fs, &rc)){
			rc = 1;
		}
		//else
			//printf("can't find fs in mtab\n");
	}
	//else
	//	printf("Can't get fs from mnt context\n");

	mnt_free_context(cxt);
	return (rc)?true:false;
}


eErrorTp GetStorageInfo(sStorageInfo * storage,char * MountDev,char * MountPoint)
{
	struct statvfs fiData;
	ifstream f;
	double calk;

			f.open(MountDev);
			//printf("%s %d\n",MountDev,f.good());
			if (f.good()){
				f.close();
				//if ((statvfs(MountPoint,&fiData)) >= 0 )
				if (checkMountStatus(MountDev,MountPoint))
				{
					if ((statvfs(MountPoint,&fiData)) >= 0 ){
					   if (fiData.f_fsid==0)
					   {
						   memset(storage,0,sizeof(sStorageInfo));
						   storage->IsMount=UMOUNT_STATE;
					   }
					   else
					   {
						   storage->IsMount=MOUNT_STATE;
						   calk=(double)((double)fiData.f_blocks*(double)fiData.f_bsize)/1048575;
						   storage->TotalSizeMB=int(calk);
						   calk=(double)((double)fiData.f_bfree*(double)fiData.f_bsize)/1048575;
						   storage->FreeSizeMB=int(calk);
						   calk=(double)(((double)storage->TotalSizeMB-(double)storage->FreeSizeMB)/(double)storage->TotalSizeMB)*100;
						   storage->PercentageOfUse=calk;

					   }
					}
				}
				else
				{
					memset(storage,0,sizeof(sStorageInfo));
				}
				storage->IsPresent=1;
			}
			else{
				 memset(storage,0,sizeof(sStorageInfo));
			}

			//

			return NO_ERROR;
}

eErrorTp GetFolderInfo(sFolderInfo * storage,char * MountPoint)
{
	struct statvfs fiData;
	ifstream f;
	double calk;


	if ((statvfs(MountPoint,&fiData)) >= 0 )
	{//zr();
	// printf("f_blocks %llu f_bsize %llu f_bavail %llu f_bfree %llu\n",fiData.f_blocks,fiData.f_bsize,fiData.f_bavail,fiData.f_bfree);
	  // if (fiData.f_fsid==0) Нельзя ориентироваться на fsid в tmpfs он не работает
	  // {//zr();
		//   memset(storage,0,sizeof(sFolderInfo));
	//	   return ERROR;
	 //  }
	  // else
	   {

		   calk=(double)((double)fiData.f_blocks*(double)fiData.f_bsize)/1048575;
		   storage->TotalSizeMB=int(calk);
		   calk=(double)((double)fiData.f_bavail*(double)fiData.f_bsize)/1048575;
		   storage->FreeSizeMB=int(calk);
		   calk=(double)(((double)storage->TotalSizeMB-(double)storage->FreeSizeMB)/(double)storage->TotalSizeMB)*100;
		   storage->PercentageOfUse=calk;
		   return NO_ERROR;
	   }
	}
	else
	{//zr();
		memset(storage,0,sizeof(sStorageInfo));
		return ERROR;
	}

	return NO_ERROR;
}


eErrorTp SearchFile(string fname)
{
	return existsSync((char*)fname.c_str());
}

eErrorTp SearchFile(const char * fname)
{
	return existsSync(fname);
}

eErrorTp GetExtensionStr(string & fname, string & extension)
{
	u32 slen=fname.size();
	u32 i;
	for (i=slen-2;i>0;i--)
	{
		if (fname[i]=='.')
		{
			extension=&fname.c_str()[i+1];
			return NO_ERROR;
		}
	}
	extension=" ";
	return ERROR;
}

u32 GetFileSize(const char * filename)
{
	u32 size;
	struct stat buf;

	if (lstat(filename,&buf)==0)
	{
		size=buf.st_size;
		return size;
	}
	else
		return 0;
}

eErrorTp DoMkdir(const char *path, mode_t mode)
{
	struct stat            st;
    eErrorTp        status = NO_ERROR;

    if (stat(path, &st) != 0)
    {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            status = ERROR;
    }
    else if (!S_ISDIR(st.st_mode))
    {
       // errno = ENOTDIR;
        status = ERROR;
    }

    return(status);
}

/**
** mkpath - ensure all directories in path exist
** Algorithm takes the pessimistic view and works top-down to ensure
** each directory in path exists, rather than optimistically creating
** the last element and working backwards.
** mode - 0xffffffff for default
*/
eErrorTp MkPath(const char *path, mode_t mode)
{
    char           *pp;
    char           *sp;
    eErrorTp       status;
    char           *copypath = strdup(path);

    status = NO_ERROR;
    pp = copypath;
    while (status == NO_ERROR && (sp = strchr(pp, '/')) != 0)
    {
        if (sp != pp)
        {
            /* Neither root nor double slash in path */
            *sp = '\0';
            status = DoMkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == NO_ERROR)
        status = DoMkdir(path, mode);
    free(copypath);

    return (status);
}

/* Call unlink or rmdir on the path, as appropriate. */
int
rm( const char *path, const struct stat *s, int flag, struct FTW *f )
{
    int status;
    int (*rm_func)( const char * );

    switch( flag ) {
    default:     rm_func = unlink; break;
    case FTW_DP: rm_func = rmdir;
    }
    if( status = rm_func( path ), status != 0 )
        perror( path );
    //else
      //  puts( path );
    return status;
}

eErrorTp DeletePath(string & str)
{
	if( nftw( str.c_str(), rm, 10, FTW_DEPTH )) {
           return ERROR;
       }
	return NO_ERROR;

}

eErrorTp equalFiles(string sin1, string sin2)
		{
			ifstream in1(sin1.c_str(), ios::binary);
			ifstream in2(sin2.c_str(), ios::binary); ;

		    ifstream::pos_type size1, size2;

		    size1 = in1.seekg(0, ifstream::end).tellg();
		    in1.seekg(0, ifstream::beg);

		    size2 = in2.seekg(0, ifstream::end).tellg();
		    in2.seekg(0, ifstream::beg);

		    if(size1 != size2)
		    {
		    	in2.close();
		    	in1.close();
		        return ERROR;
		    }
		    if ((size1==size2)&&(size1==0))
		    	return ERROR;

		    static const size_t BLOCKSIZE = 4096;
		    size_t remaining = size1;

		    while(remaining)
		    {
		        char buffer1[BLOCKSIZE], buffer2[BLOCKSIZE];
		        size_t size = std::min(BLOCKSIZE, remaining);

		        in1.read(buffer1, size);
		        in2.read(buffer2, size);

		        if(0 != memcmp(buffer1, buffer2, size))
		        {
			    	in2.close();
			    	in1.close();
		            return ERROR;
		        }

		        remaining -= size;
		    }

	    	in2.close();
	    	in1.close();
		    return NO_ERROR;
		}

eErrorTp CopyFile(char * FileNameSrc, char * FileNameDst)
{
	eErrorTp err=NO_ERROR;

	std::ifstream  src(FileNameSrc, std::ios::binary);
	if ((!src)||src.fail())
	{
		 //Print->GPRINT(NORMAL_LEVEL,"Error src [%s] not copy to [%s]\n",FileNameSrc,FileNameDst);

		 return ERROR;
	}

	std::ofstream  dst(FileNameDst,std::ios::binary);
	if ((!dst)||dst.fail())
	{
		src.close();
		//Print->GPRINT(NORMAL_LEVEL,"Error create dst [%s]\n",FileNameDst);
		return ERROR;
	}

	dst << src.rdbuf();

	if (dst.fail()||src.fail())
	 {
		err=ERROR;
		//Print->GPRINT(NORMAL_LEVEL,"Error copy [%s] to [%s]\n",FileNameSrc,FileNameDst);
	 }
	else
	{
		 //Print->GPRINT(MEDIUM_LEVEL,"Copy [%s] to [%s]\n",FileNameSrc,FileNameDst);
	}
	src.close();
	dst.close();

	return err;
}

searched_file_list::searched_file_list(string name,string fname,struct stat & st){
	this->name=name;
	this->fname=fname;
	memcpy(&this->st,&st,sizeof(st));
	//this->fsize=fsize;
}

filesys::~filesys(){
	if (file_desc!=NULL){
		write_cached();
		fclose(file_desc);
	}

	//printf("filesys is down\n");
}


u32 filesys::SearchFilesInDir(char *dir,vector<searched_file_list> & files,string mask)
	{
		DIR *dp;
		struct dirent *entry;
		struct stat statbuf;
		u32 numberOfEntries=0;
		u32 fname_max_size=2000;
		std::unique_ptr<char[]> name(new char[fname_max_size]);
		string str;
		u16 offs=snprintf(name.get(),fname_max_size,"%s/",dir);

		//printf("pos %d\n",pos);
		files.clear();
		if ((dp = opendir(dir)) != NULL) {
					while((entry = readdir(dp)) != NULL) {

						if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
							continue;

						str=entry->d_name;
						//printf("find %d\n",str.find(mask));
						//printf("in %s find %s\n",entry->d_name,mask.c_str());
						if ((signed int)str.find(mask)==-1)
							continue;
						strcpy(&name.get()[offs],entry->d_name);
						lstat((char*)name.get(), &statbuf);


						//printf("emplace\n");
						files.emplace_back(string(entry->d_name),string(name.get()),statbuf);
							//Delete obsolete files
						numberOfEntries++;
					}
					closedir(dp);
				}

		return numberOfEntries;
	}

eErrorTp filesys::file_set_file_name(string fname,bool cached){
		filename=fname;
		write_wcache=cached;
		return NO_ERROR;
	}

eErrorTp filesys::file_close(void)
	{
		if (file_desc!=NULL){
			fclose(file_desc);
			file_desc=NULL;
			return NO_ERROR;
		}
		return ERROR;
	}

eErrorTp filesys::file_sync(void)
	{
		//printf("file_sync\n");
		if (file_desc!=NULL){
			write_cached();
			//printf("fclose %s\n",filename.c_str());
			fclose(file_desc);
			write_cnt=0;
			file_desc=fopen(filename.c_str(),"a");
			//printf("fopen %s\n",filename.c_str());
			if (file_desc==NULL){
				return ERROR;
			}
		}
		return NO_ERROR;
	}

u32 filesys::file_reset_write_cntr(void){
		u32 w=write_cnt;
		write_cnt=0;
		return w;
	}


eErrorTp filesys::file_add_string(string & data){
	//	if ()
		u32 ssyze;
		//printf("file_add_string\n");
		if ((fopenfname!=filename)&&(file_desc!=NULL)){
			//скидывает буфер и создает новый файл
			write_cached();
			fclose(file_desc);
			write_cnt=0;
			file_desc=NULL;
		}


		if (file_desc==NULL){
			write_cnt=0;
			//printf("file_add_string fopen\n");
			file_desc=fopen(filename.c_str(),"a");
			fopenfname=filename;
			if (file_desc==NULL)
				return ERROR;
			//if (SearchFile(file)==ERROR)
		}

		if (data.size()==0){
			write_cached();
			//printf("file_add_string (data.size()==0) fclose\n");
			fclose(file_desc);
			write_cnt=0;
			return NO_ERROR;
		}
		ssyze=write_wcache_str.size();
		if (write_wcache){
			//if ((ssyze<max_cached_string)){
				write_wcache_str.emplace_back(data);
				//printf("file_add_string to cache\n");
			//}
			//else{
				if ((ssyze>=(max_cached_string-1))){
					printf("Warning: cache overflow max_size %d, data discharging into file %s\n",max_cached_string,filename.c_str());
					write_cached();
				}
				//fwrite(data.c_str(),data.size(),1,file_desc);

			//}
		}
		else{
			//printf("file_add_string fwrite\n");
			fwrite(data.c_str(),data.size(),1,file_desc);
			//write_cnt++;
		}
		write_cnt++;
		return NO_ERROR;
	}


eErrorTp filesys::write_cached(void){
		if ((file_desc==0)||(write_wcache==false))
			return ERROR;

		for (u32 n=0;n<write_wcache_str.size();n++){
			//printf("      wr:%s to %s\n",write_wcache_str[n].c_str(),filename.c_str());
			fwrite(write_wcache_str[n].c_str(),write_wcache_str[n].size(),1,file_desc);
			sync();
			//write_cnt++;
		}

		//printf("Write cached %d string to %s\n",write_wcache_str.size(),filename.c_str());
		write_wcache_str.clear();
		return NO_ERROR;
	}



//classic mount, can't force mount rw, this func use util-linux libmount
/**
Brief: classic mount, can't force mount rw, this func use util-linux libmount
Params: __special_file - source mounted file
		__dir - destination directory
		__fstype - type of  filesystem
		__rwflag - mounting flags
		__data - mounting options
Return: NO_ERROR if ok, otherwise ERROR
*/
eErrorTp Mount(const char *__special_file, const char *__dir,
				  const char *__fstype, unsigned long int __rwflag,
				  const char *__data)
{

	eErrorTp err=NO_ERROR;
	if (existsSync(__special_file)==ERROR)
		return ERROR;

	struct libmnt_context *cxt = mnt_new_context();

	if (existsSync(__dir)==ERROR)
		MkPath(__dir,0xffffffff);


	mnt_context_set_options(cxt, __data);
	mnt_context_set_mflags(cxt, __rwflag);
	mnt_context_set_target(cxt, __dir);
	mnt_context_set_source(cxt, __special_file);
	if (__fstype!=NULL)
		mnt_context_set_fstype(cxt, __fstype);

	if (!mnt_context_mount(cxt))
	{
		err=NO_ERROR;
	}
	else
		err=ERROR;

	mnt_free_context(cxt);
	return err;
}

/**
Brief: classic unmount
Params: __special_file - source unmounted file
Return: NO_ERROR if ok, otherwise ERROR
*/
eErrorTp Umount (const char *__special_file)
{
	eErrorTp err=NO_ERROR;
	struct libmnt_context *cxt = mnt_new_context();

	if (mnt_context_set_target(cxt, __special_file))
		err=ERROR;
	else
	{
		if (mnt_context_umount(cxt)!=0)
			err=ERROR;
	}
	mnt_free_context(cxt);
	return err;
}

/**
Brief: force mount
Params: __special_file - source unmounted file
Return: NO_ERROR if ok, otherwise ERROR
*/
eErrorTp UmountForce (const char *__special_file)
{
	eErrorTp err=NO_ERROR;
	struct libmnt_context *cxt = mnt_new_context();
	if (mnt_context_set_target(cxt, __special_file))
		err=ERROR;
	else
	{
		mnt_context_enable_force(cxt, true);
		if (mnt_context_umount(cxt)!=0)
			err=ERROR;
	}
	mnt_free_context(cxt);
	return err;
}

/**
Brief: checks file systme options
Params: __dir - directory path
		findopts - checking options
Return: NO_ERROR if ok, otherwise ERROR
*/
eErrorTp CheckFSOpts(const char *__dir,const char * findopts)
{
	eErrorTp err=ERROR;

	 libmnt_table * mountTable_ = mnt_new_table();
	 if (mountTable_!=NULL)
	 {
		 mnt_table_parse_file(mountTable_, "/proc/mounts");
		 libmnt_fs *entry = mnt_table_find_target(mountTable_,__dir,MNT_ITER_FORWARD);
		 if (entry!=NULL)
		 {
			 const char * opt=mnt_fs_get_options(entry);

			 if (mnt_match_options(opt,findopts)==1)
				 err= NO_ERROR;

			 mnt_free_fs(entry);
		 }
		 mnt_free_table(mountTable_);
	 }
	 return err;
}

///It specifies the file exists or not
/**
Brief: looking for file
Params: fname - file name
Return: NO_ERROR if ok, otherwise ERROR
*/

eErrorTp existsSync(const char * fname)
{

	FILE * fil=NULL;

	fil=fopen(fname,"rb");

	if (fil==NULL) return ERROR;

	fclose(fil);
	return NO_ERROR;
}

eErrorTp existsSyncPipe(const char * fname)
{

	int fil=0;

	fil=open(fname,O_NONBLOCK | O_RDWR);

	if (fil<=0)
		return ERROR;

	close(fil);
	return NO_ERROR;
}

eErrorTp existsSync(string  fname)
{
	return existsSync(fname.c_str());
}

eErrorTp existsDir(string dir){

	struct stat info;

	if( stat( dir.c_str(), &info ) != 0 )
	    return ERROR;
	else if( info.st_mode & S_IFDIR )  // S_ISDIR() doesn't exist on my windows
	    return NO_ERROR;
	else
	   return ERROR;
}

bool SearchExtension(const string& s, const string& suffix)
{
    return (s.size() >= suffix.size()) && equal(suffix.rbegin(), suffix.rend(), s.rbegin());
}

eErrorTp SearchFilesByMask(char * dir,char * suffix,vector <string> & result){
	DIR *d = opendir(dir);
	u32 cnt=0;
	//Mask example .update
	if(!d)
	    {
	        return ERROR;
	    }
	 dirent *entry;
	 while ((entry = readdir(d))!=NULL)
	 {
	    if(SearchExtension(entry->d_name, suffix))
	    {
	    	//string r=string_format("%s/%s",dir,entry->d_name);
	    	//printf("push %s\n",r.c_str());
	    	result.push_back(entry->d_name);
	    	cnt++;
	           // cout << entry->d_name << endl;
	    }
	 }
	 closedir(d);

	if (cnt>0)
		return NO_ERROR;
	else
		return ERROR;
}

string GetMD5_SumDD(string filename,u32 readsize,u32 blocksize){
	if (existsSync(filename.c_str())==ERROR)
		return "";

	u32 readsize_inbl=readsize/blocksize;
	if ((readsize%blocksize)!=0)
		readsize_inbl++;
	string scr=string_format("dd if=%s bs=4M|head -c %u|md5sum",filename.c_str(),readsize);
	string sumout=BashResult((char*)scr.c_str());
	//printf("sumout %s\n",sumout.c_str());
	return sumout.substr(0,sumout.find(" "));
}

eErrorTp CheckFreeSpaceInStorageForFile(char * file,char * MountPoint){
	sFolderInfo  storage;
	//printf("File %s\n",file);
	if (existsSync(file)==ERROR)
		return ERROR;

	u32 fsize=GetFileSize(file);
	u32 needsizeMB=(fsize+(fsize/10))/0x100000;//reserved 10%

	GetFolderInfo(&storage,MountPoint);
	//GetStorageInfo(&storage,MountDev,MountPoint);
	u32 freesz=storage.FreeSizeMB;
	printf("MountPoint %s free size %d needsize %u\n",MountPoint,freesz,needsizeMB);
	if (needsizeMB<=freesz){
		return NO_ERROR;
	}

	return ERROR;
}

eErrorTp CreateFile(char * fname,string & data){
	ofstream file(fname,std::ios::binary);
	if (file.fail())
		return ERROR;
	file << data;
	if (file.fail())
		return ERROR;
	file.close();
	return NO_ERROR;
}
//MAX_SIZE_STRING_FILES

eErrorTp ReadStringFileSync(char * fname,string & data,u32 wait_delay_ms){
	u32 wdm=wait_delay_ms/100;
	for (u8 n=0;n<wdm;n++){
	  if (ReadStringFile(fname,data,MAX_SIZE_STRING_FILES)==NO_ERROR)
		 return NO_ERROR;
	  sync();
	  mdelay(100);
	}
	return ERROR;
}

u32 readDevFile(string fname,buffer & data,u32 limitSize)
{
    FILE * file=NULL;
    file=fopen(fname.c_str(),"r");
    if (file==NULL)
        return ERROR;

    data.create(limitSize);
    u32 nread;
    nread = fread(data.p(), 1, limitSize, file);
    fclose(file);

    return nread;
}

eErrorTp readBinFile(string fname,buffer & data,u32 limitSize)
{
	FILE * file=NULL;
	file=fopen(fname.c_str(),"r");
	if (file==NULL)
		return ERROR;

	u32 maxLen=GetFileSize(fname.c_str());
	if (maxLen>limitSize)
		return ERROR;

	data.create(maxLen);
	u32 nread;
	nread = fread(data.p(), 1, maxLen, file);
	fclose(file);

	if (nread==0)
		return ERROR;

	return NO_ERROR;
}

eErrorTp ReadStringFile(char * fname,string & data){
	return ReadStringFile(fname,data,MAX_SIZE_STRING_FILES);
}
eErrorTp ReadStringFile(char * fname,string & data,u32 max_len){
	FILE * file=NULL;
	file=fopen(fname,"r");
	if (file==NULL)
		return ERROR;

	buffer b(max_len);
	u32 nread;
	nread = fread(b.p(), 1, max_len, file);
	fclose(file);
	if (nread==max_len)
		return ERROR;

	if (nread==0)
		return ERROR;

	b.p()[max_len-1]=0;//protect
	data=(char*)b.p();
	//printf("Read %s size %d\n",fname,nread);
	//ifstream file;
	//data="";
	/*file.open(fname);
	if (file.is_open())
	{
		data.assign( (std::istreambuf_iterator<char>(file) ),
		                (std::istreambuf_iterator<char>()    ) );
		//file.read()

		file.close();
		return NO_ERROR;
	}*/
	return NO_ERROR;
}

#ifdef _SAFE_LOGGER
eErrorTp UngzipFile(char * fname,string & result){
	FILE * file=NULL;
	file=fopen(fname,"r");
	if (file==NULL)
		return ERROR;

	buffer b(MAX_SIZE_STRING_FILES);
	std::size_t nread;
	nread = fread(b.p(), 1, MAX_SIZE_STRING_FILES, file);
	fclose(file);
	if (nread==MAX_SIZE_STRING_FILES)
		return ERROR;

	if (nread==0)
		return ERROR;
//inline std::string decompress(const char* data, std::size_t size)
	result=gzip::decompress((const char*)b.p(), nread);
	if (result.size()==0)
		return ERROR;
	return NO_ERROR;
}
#endif
eErrorTp WriteStringFile(char * fname,string & data){
	return CreateFile(fname,data);
}


eErrorTp WriteStringFileZip(char * fname,string & data){
	string compressed_data = gzip::compress(data.c_str(), data.size());
	return CreateFile(fname,compressed_data);
}

std::string base_name(std::string const & path)
{
  return path.substr(path.find_last_of("/\\") + 1);
}

