/*
 * setclean.cxx
 *
 *  Created on: 18 дек. 2018 г.
 *      Author: root
 */

#include "settings_adm.h"
#include "engine/global.h"

eErrorTp FindFileInJSON_List(Json::Value & undellist,string fname)
{
	u32 ngp=0;
	if (undellist.isArray())
			{
				while(undellist[ngp].isNull()==false)
				{
					if (undellist[ngp].asString()==fname)
						return NO_ERROR;

					ngp++;
				}
			}
	return ERROR;
}

u32 rm_files_exclude(char *dir,SettingsAdm * Sm,Json::Value & undellist)
{
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	u32 numberOfEntries=0;

	string fname;
	if ((dp = opendir(dir)) != NULL) {
				while((entry = readdir(dp)) != NULL) {

					if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
						continue;

					if (FindFileInJSON_List(undellist,entry->d_name)==NO_ERROR)
						Sm->GPRINT(NORMAL_LEVEL,"Skip %s\n",entry->d_name);
					else
						//Sm->GPRINT(NORMAL_LEVEL,"Remove %s\n",entry->d_name);
						Sm->RemoveFromSetting4x4(entry->d_name);


					//if (entry->d_name)
					//
					numberOfEntries++;
				}
				closedir(dp);
			}

		return numberOfEntries;
}

int settings_clean_main(int argc, char **argv)
{

	//printf("argc %d\n",argc);
	if (argc!=3)
		return -1;

	SettingsAdm Sm;
	Sm.SetLogLevel(NORMAL_LEVEL);

	CnT->ReadConfigConf(string(argv[1]));

	Json::Value undellist;
	std::ifstream config_doc(string(argv[2]).c_str(), std::ifstream::binary);
	config_doc >> undellist;

	rm_files_exclude((char*)CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST].c_str(),&Sm,undellist);
	//argv[1] - config file
	//argv[2] - undel file

	//example:
	//	./setclean /www/pages/Cnoda/Cnoda.json /etc/necron/undelete.set

	printf("Success\n");
	return 0;
}
