/*
 * sset.cxx
 *
 *  Created on: 18 дек. 2018 г.
 *      Author: root
 */

#include "engine/settings_adm.h"
#include "engine/global.h"

int save_settings(int argc, char **argv)
{

//	printf("argc %d\n",argc);


	if (argc!=4)
		return -1;

	SettingsAdm Sm;
	Sm.SetLogLevel(NO_USE);
	CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_FIRST]=argv[2];
	CnT->SETTING_STORAGE_MOUNTP[SETTING_STORAGE_RESERVED]=argv[3];

	//FILE * stdin;
	//string JSONstr << cin;
	stringstream JSONstr;
    for (std::string line; std::getline(std::cin, line);) {
    	JSONstr<<line<<endl;
        //printf("[%s]\n",line.c_str());
    }

	//argv[1] - setting name without preambule, settings.update.set, name - settings.update
	//argv[2] - sys path1
	//argv[3] - sys path2
	//example:
	//echo "test 200"|./sset setting.test /www/pages/sys /www/pages/sys_ex/

	string filename=argv[1];

	Sm.GPRINT(HARD_LEVEL,"save\n");

	Sm.SaveSetting(JSONstr.str(),filename);
	Sm.GPRINT(HARD_LEVEL,"save end\n");
	//printf("%s",JSONstr.str().c_str());

	return 0;
}


