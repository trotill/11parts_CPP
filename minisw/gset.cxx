/*
 * gset.cxx
 *
 *  Created on: 18 дек. 2018 г.
 *      Author: root
 */

#include "settings_adm.h"
#include "engine/global.h"

int load_settings(int argc, char **argv)
{

	//printf("argc %d\n",argc);
	if (argc!=5)
		return -1;

	//printf("GSET\n");
	SettingsAdm Sm;
	Sm.SetLogLevel(NO_USE);
	//printf("GSET_ttt %x\n",CnT);
	//CnT->DISABLE_OUT=1;
	//printf("GSET11\n");
	CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]=argv[2];
	//printf("GSET12\n");
	CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED]=argv[3];
	CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_DEFAULT]=argv[4];

	//argv[1] - setting name without preambule, settings.update.set, name - settings.update
	//argv[2] - sys path1
	//argv[3] - sys path2
	//argv[4] - sys path3
	//example:
	//	./gset settings.router /www/pages/sys/ /www/pages/sys_ex/ /etc/necron/

	string JSONstr="";
	string filename=argv[1];

	int n = filename.find("settings.");
	//printf("n %d\n",n);
	if (n>=0){
		string sname=filename.substr(sizeof("settings"));
		filename =Sm.GetSettingsName((char*)sname.c_str());
		//printf("n %s\n",filename.c_str());
	}

	Sm.LoadSetting(JSONstr,filename);
	printf("%s",JSONstr.c_str());

	return 0;
}
