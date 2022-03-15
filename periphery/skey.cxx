/*
 * skey.cxx
 *
 *  Created on: 21 окт. 2019 г.
 *      Author: root
 */

#include "skey.h"
#include <engine/proto/json_proto.h>
#include <engine/print.h>

#include "engine/memadmin.h"
#include "engine/lib/11p_bin.h"
#include "engine/lib/11p_files.h"
#include "engine/lib/11p_time.h"
#include "engine/lib/11p_string.h"

#include "engine/algo/aes.h"
#include "engine/algo/sha256.h"
#include "engine/algo/base64.h"
#include "engine/algo/MD5.h"

#include "engine/settings_adm.h"
#include "custom_project/custom_project.h"

string kent::rdmd(string dev,long int offs,u32 size){
		int file=open(dev.c_str(),0);
		if (file==-1)
			return "";

		u32 nsz=65535;
		if ((size<65535)&&(size!=0))
			nsz=size;

		if (size==0)
			size=0xffffffff;

		buffer buf(nsz);
		u32 sz=size;
		u32 tot_read=0;
		int read_size=0;

		if (lseek(file,offs,SEEK_SET)==-1)
			return "";
		std::string hash_hex_str;
		do {

			if ((size-tot_read)>=65535)
				read_size=read(file,buf.p(),buf.size());
			else
				read_size=read(file,buf.p(),size-tot_read);
			if (read_size>0){
				tot_read+=read_size;
				picosha2::hash256_hex_string(buf.p(), buf.p()+read_size, hash_hex_str);
				//bufferToString((char*)buf.p(),read_size);
			}
		//	printf("rsz %d\n",read_size);
			//if ((read_size==0)||(read_size==-1)||(tot_read>=size))
				//break;
		}while(!((read_size==0)||(read_size==-1)||(tot_read>=size)));//(read_size!=0)||(read_size!=-1)||(tot_read>=size));

		//printf("SHA %s\n",hash_hex_str.c_str());
		close(file);
		return hash_hex_str;
	}

string kent::kesd(){
	if (key_sd==false)
		return "";

	string k="";
	string base_dir="/sys/class/";
	base_dir+="m";
	base_dir+="m";
	base_dir+="c";
	base_dir+="_";
	base_dir+="host";

	string ccc="/c";
	ccc+="i";
	ccc+="d";
	string rrr="/s";
	rrr+="c";
	rrr+="r";
	string sss="";
	string srrr="";
	filesys fs;
	vector<searched_file_list> dirs;
	vector<searched_file_list> files;
	string mask="";
	mask+=base_dir[11];
	mask+=base_dir[12];
	mask+=base_dir[13];
	mask+=string_format("%d",key_sd_num);

	//char *dir,vector<searched_file_list> & files,string mask

	if (fs.SearchFilesInDir((char*)base_dir.c_str(),dirs,mask)>0){
		//printf("SearchFilesInDir %d\n",dirs.size());
		for (searched_file_list & key : dirs){
			//printf("key %s\n",key.fname.c_str());
			if (fs.SearchFilesInDir((char*)key.fname.c_str(),files,mask)>0){
				if (files.size()==1){
					ReadStringFile((char*)string(files[0].fname+ccc).c_str(),sss);
					//printf("cid %s\n",sss.c_str());
					ReadStringFile((char*)string(files[0].fname+rrr).c_str(),srrr);
					//printf("scr %s\n",srrr.c_str());
					string ks=srrr+sss+KENT_SALT;
					//unsigned char binbuf[MD5_DIGEST_LENGTH];
					//unsigned char result[MD5_DIGEST_LENGTH+1]={0};
					k=md5(ks);
					//memcpy(result,binbuf,MD5_DIGEST_LENGTH);
					//k=(char*)result;
					//printf("key %s\n",k.c_str());

				}
			}
		}
	}

	return k;
}
eErrorTp kent::klk(void){
	SettingsAdm sa;
	Json::Value val;

	if (sa.LoadSetting(val,"settings.license")==ERROR)
		return ERROR;
	else
	{
		if (val.isMember("d")&&val["d"].isMember("lic")){
			char * gs=(char*)val["d"]["lic"].asCString();

			f__sk();
			if (__sk==NULL)
				return ERROR;
			if (strcmp(gs,__sk)==0){
				return NO_ERROR;
			}
			//if (back_to_factory_if_lic_false)
				//Customer.ResetDeviceToFactory();
			if (kill_device_if_lic_false)
				kill_device();
			if (reboot_if_lic_false)
				RebootSystem();

			return ERROR;
		}
		else
			return ERROR;
	}
}
eErrorTp kent::place(string k){
	SettingsAdm sa;
	k.erase(std::remove(k.begin(), k.end(), '\n'), k.end());
	if (k==admin_key){
		f__sk();
		if (__sk==NULL)
			return ERROR;

		Json::Value json;
		json["t"].isArray();
		json["t"][0]=JSON_PACK_TYPE_SET_SYSTEM;
		json["t"][1]=JSON_PACK_VERSION;
		json["d"]["type"]="settings";
		json["d"]["page"]="license";
		string s=__sk;
		json["d"]["lic"]=s;

		return sa.SaveSettingToStorage(json,"settings.license");
	}
	return ERROR;
}
eErrorTp kent::autogen(){
	SettingsAdm sa;
	if ((autogen_lic)&&(__sk==NULL)){
		string licf=CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]+"/settings.license.set";
		if (existsSync(licf)==ERROR){
			string mount_cmd=string_format("mount %s /mnt",autogen_stor_dev.c_str());
			system(mount_cmd.c_str());
			string kfile=string_format("/mnt/%s",autogen_lic_key_file.c_str());
			string data;
			if (ReadStringFile(kfile.c_str(),data,1024)==NO_ERROR){
				system("umount /mnt");
				return place(data);
			}
			else{
				system("umount /mnt");
				return ERROR;
			}


		}
	}
	return ERROR;
}
kent::kent(){
	//place("WersEwqw23drov**#7422dcmvf31");
	string data="][";

	if (klk()==NO_ERROR){
		shared_fifo->SendSharedFifoMessage(srvmSKEY,"all",data);
	}
	if (autogen()==NO_ERROR)
		exit(0);

}

kent::~kent(){
	if (__sk!=NULL)
		free(__sk);
}

void kent::f__sk(void){

	if ((key_sd==false)&&(key_cpu_ull==false)&&(devfile.size()==0))
		return;

	string ksd=kesd();

	if ((ksd.size()==0)&&(key_sd))
		return;


	u8 kr[16]={0};
	u8 ksz=(u8)strlen(LIBC);
	if (ksz>16)
		ksz=16;

	memcpy(kr,LIBC,ksz);
	AES aes(128);
	u32 len=0;
	unsigned char *out = aes.EncryptECB((u8*)ksd.c_str(), (u32)ksd.size(), kr, len);
	string enc=base64_encode((const u8*)out, len);
	delete[] out;
	string devsha="";


	if (devfile.size()!=0){
		devsha=rdmd(devfile,devfile_offs,devfile_size);
		if (devsha.size()==0)
			return;

		len=0;
		memcpy(kr,devsha.c_str(),ksz);
		out = aes.EncryptECB((u8*)enc.c_str(), (u32)enc.size(), kr, len);
		enc=base64_encode((const u8*)out, len);
		delete[] out;
	}
	len=0;
	if (admin_key.size()>=ksz)
		memcpy(kr,admin_key.c_str(),ksz);
	else
		memcpy(kr,admin_key.c_str(),admin_key.size());


	out = aes.EncryptECB((u8*)enc.c_str(), (u32)enc.size(), kr, len);
	enc=base64_encode((const u8*)out, len);
	delete[] out;

	__sk=new char[enc.size()+1];
	strcpy(__sk,enc.c_str());
}


