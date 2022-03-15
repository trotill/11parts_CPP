/*
 * gprivate.cxx
 *
 *  Created on: 18 дек. 2018 г.
 *      Author: root
 */

#include "engine/lib/11p_files.h"
//#include "guards.h"
#include "settings_adm.h"
#include "engine/lib/11p_process.h"
#include "engine/global.h"

eErrorTp burn_private(string folder,string dev,u32 offset)
{
	//cout << "folder " << folder << " device " << dev << " offset " << offset;


	DIR *dp;
	eErrorTp err=ERROR;
	string package;
	struct dirent *entry;
	struct stat statbuf;
	u32 numberOfEntries=0;
	std::ifstream  factory_key;
	string fpath;
	string fname;
	string key;
	if ((dp = opendir(folder.c_str())) != NULL) {
			while((entry = readdir(dp)) != NULL) {
				if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
					continue;
				if (memcmp(entry->d_name,"key",3)!=0)
				    continue;

					fpath=folder+"/"+string(entry->d_name);
					factory_key.open(fpath.c_str());
					if (factory_key.is_open()) {
						key.assign((istreambuf_iterator<char>(factory_key)),istreambuf_iterator<char>());
						factory_key.close();

						Json::Value root;
						Json::Reader reader;
						bool parsingSuccessful = reader.parse( key.c_str(), root );
						if (parsingSuccessful==false) {
							printf("skip broken key %s\n--data%s\n",entry->d_name,key.c_str());
							continue;
						}
						//printf("burn key %s %s to %s offs %d \n",entry->d_name,key.c_str(),dev.c_str(),offset);
						//if (key.size()<2000){

						string package=string_format("private=%s0",key.c_str());
						u32 sz=package.size();
						package[sz-1]=0;
							//printf("burn key %s %s to %s offs %d \n",entry->d_name,key.c_str(),dev.c_str(),offset);
						printf("write package %s to %s offs %d\n",package.c_str(),dev.c_str(),offset);
						ofstream  fil;
						fil.open(dev.c_str(),ios::out |ios::app);
						if (fil.fail()){
							printf("Error write to file [%s]\n",dev.c_str());
							err=ERROR;
							break;
						}

						fil.seekp(offset);
						fil.write((const char *)package.c_str(),package.size()+1);
						if ((!fil)||fil.fail())
						{
							fil.close();
							printf("Error write to file [%s]\n",dev.c_str());
							err=ERROR;
							break;
						}
						fil.close();
						remove(fpath.c_str());
						printf("write success, %s removed!!!\n",fpath.c_str());
						err=NO_ERROR;
						//}
						//else
							//continue;

					}
					else
						continue;


					break;
			}
			closedir(dp);
	}

	return err;
}

eErrorTp factory_flush_private(void)
{
	std::ifstream  factory_cfg;
	string jstxt_cfg;
	eErrorTp err=ERROR;
	//factory_cfg.open("/etc/factory.json");

	factory_cfg.open("/etc/factory.json");
	if (factory_cfg.is_open()) {
	  jstxt_cfg.assign((istreambuf_iterator<char>(factory_cfg)),istreambuf_iterator<char>());
	  factory_cfg.close();
	}
	else {
		printf("/etc/factory.json not open\n");
		return ERROR;
	}

	Json::Value root;
	Json::Reader reader;
	//printf("parsing %s\n",jstxt_cfg.c_str());
	bool parsingSuccessful = reader.parse( jstxt_cfg.c_str(), root );
	if (parsingSuccessful) {
			string mountp=root["mountp"].asString();
			string fstype=root["fstype"].asString();
			string mountdev=root["mountdev"].asString();
			string burndev=root["burndev"].asString();
			u32 burnoffset=root["burnoffset"].asInt();
			string mount=string_format("mount -t %s %s %s",fstype.c_str(),mountdev.c_str(),mountp.c_str());
			printf("%s\n",mount.c_str());
			system(mount.c_str());
			sleep(1);
			sStorageInfo  storage;
			GetStorageInfo(&storage,(char*)mountdev.c_str(),(char*)mountp.c_str());
			if (storage.IsMount)
				err=burn_private(mountp,burndev,burnoffset);
			else
				err=ERROR;

			string umount=string_format("umount %s",mountdev.c_str());
			system(umount.c_str());
			return err;
	}
	else
		return ERROR;
}

int settings_private(int argc, char **argv)
{
	SettingsAdm Sm;
	Sm.SetLogLevel(NORMAL_LEVEL);
	string result=ExecResult("cat","/proc/cmdline | awk -F'private=' '{print $2}' | awk -F' ' '{ print $1 }'|tr -d '\n'");

	bool reboot=true;
	if (argc>1){
		if (strcmp(argv[1],"noreboot")==0)
			reboot=false;
	}

	Sm.GPRINT(NORMAL_LEVEL,"Starting private manager\n");
	//printf("result.size() %d\n",result.size());
	if (result.size()>2) {
		Json::Value root;
		Json::Reader reader;
		string json;
		bool parsingSuccessful = reader.parse( result.c_str(), root );
		if (parsingSuccessful) {
			//Json::FastWriter writer;
			json=FastWriteJSON(root);
			std::ofstream  file;
			string fpath=string_format("%s%s",CACHE_PATH_DEF,"/private");
			file.open(fpath.c_str(),std::ofstream::out);
			if ((!file)||file.fail()) {
				file.close();
				Sm.GPRINT(NORMAL_LEVEL,"Error create file [%s]\n",fpath.c_str());
			}

			file << json;
			file.flush();
			file.close();
			sync();
		}
		else {
			Sm.GPRINT(NORMAL_LEVEL,"Error parse %s\n",result.c_str());
			if (factory_flush_private()==ERROR)
				Sm.GPRINT(NORMAL_LEVEL,"Error factory private flushing\n");
			else{
				if (reboot)
					RebootSystem();
			}
		}
	}
	else{

		Sm.GPRINT(NORMAL_LEVEL,"Not found private\n");
		if (factory_flush_private()==ERROR)
			Sm.GPRINT(NORMAL_LEVEL,"Error factory private flushing\n");
		else{
			if (reboot)
				RebootSystem();
		}
	}


	//printf("Result %s size %d\n",result.c_str(),result.size());

	return 0;
}

