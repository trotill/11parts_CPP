/*
 * files.h
 *
 *  Created on: 17 мая 2018 г.
 *      Author: root
 */

#ifndef FILES_H_
#define FILES_H_

#include "engine/basic.h"
#include "engine/buffer.h"

typedef struct sStorageInfo
{
  u32 FreeSizeMB;
  u32 TotalSizeMB;
  u8  PercentageOfUse;
  u8 IsPresent;
  u8 IsMount;
}sStorageInfo;

typedef struct sFolderInfo
{
  u32 FreeSizeMB;
  u32 TotalSizeMB;
  u32  PercentageOfUse;

}sFolderInfo;


eErrorTp GetStorageInfo(sStorageInfo * storage,char * MountDev,char * MountPoint);
eErrorTp GetFolderInfo(sFolderInfo * folder,char * folder_path);

class searched_file_list{
	public:
	searched_file_list(string name,string fname,struct stat & st);
	searched_file_list(){
	}
	string name="";//file name
	string fname="";//full path name
	u32 idx=0;
	struct stat st;
};

class filesys{
	public:
	~filesys();
	u32 SearchFilesInDir(char *dir,vector<searched_file_list> & files,string mask);
	eErrorTp file_set_file_name(string fname,bool cached);
	eErrorTp file_close(void);
	eErrorTp file_sync(void);
	u32 file_reset_write_cntr(void);
	eErrorTp file_add_string(string & data);
	u32 get_cache_size(){
		return write_cnt;
	}
	eErrorTp write_cached(void);



	class fclass {
		public:
		u32 desc;
		string fname;
	};
	string filename;
	string fopenfname;
	u32 write_cnt=0;

	bool write_wcache=false;
	u32 max_cached_string=500;
	vector<string> write_wcache_str;
	//vector<fclass> f;
	FILE * file_desc=NULL;
};

eErrorTp SearchFile(string fname);
eErrorTp SearchFile(const char * fname);
eErrorTp existsSync(const char * fname);
eErrorTp existsSyncPipe(const char * fname);
eErrorTp existsSync(string  fname);
eErrorTp existsDir(string dir);
eErrorTp GetExtensionStr(string & fname, string & extension);
u32 GetFileSize(const char * filename);

eErrorTp MkPath(const char *path, mode_t mode);
eErrorTp DoMkdir(const char *path, mode_t mode);
int rm( const char *path, const struct stat *s, int flag, struct FTW *f );
eErrorTp DeletePath(string & str);
eErrorTp equalFiles(string sin1, string sin2);
eErrorTp CopyFile(char * FileNameSrc, char * FileNameDst);
eErrorTp Mount(const char *__special_file, const char *__dir,
				  const char *__fstype, unsigned long int __rwflag,
				  const char *__data);
eErrorTp Umount (const char *__special_file);
eErrorTp UmountForce (const char *__special_file);
bool SearchExtension(const string& s, const string& suffix);
eErrorTp SearchFilesByMask(char * dir,char * suffix,vector <string> & result);
string GetMD5_SumDD(string filename,u32 readsize,u32 blocksize);
eErrorTp CheckFreeSpaceInStorageForFile(char * file,char * MountPoint);
//eErrorTp CreateFile(string name,string & data);
eErrorTp CreateFile(char * fname,string & data);
eErrorTp ReadStringFile(char * fname,string & data);
eErrorTp readBinFile(string fname,buffer & data,u32 limitSize);
u32 readDevFile(string fname,buffer & data,u32 limitSize);
eErrorTp ReadStringFileSync(char * fname,string & data,u32 wait_delay_ms);
eErrorTp ReadStringFile(char * fname,string & data,u32 max_len);
eErrorTp UngzipFile(char * fname,string & result);
eErrorTp WriteStringFile(char * name,string & data);
eErrorTp WriteStringFileZip(char * name,string & data);
std::string base_name(std::string const & path);

#endif /* FILES_H_ */
