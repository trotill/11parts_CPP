/*
 * imcheck.cxx
 *
 *  Created on: 21 февр. 2018 г.
 *      Author: root
 */
#include "engine/minisw/imcheck.h"

#include "engine/fifo.h"
#include "engine/lib/11p_string.h"
#include "engine/lib/11p_process.h"
#include "engine/lib/11p_files.h"
#include "engine/lib/11p_bin.h"
#include "engine/minisw/minisw.h"





int imcheck_main(int argc, char **argv)
{

	//argv[1] - source image
	//argv[2] - dest folder for unpack
	//argv[3] - dest image name
	//argv[4] - noremove если не требуется удалять исходный файл, если update - обновить после проверки (используется на производстве),
	// если 4я опция отсутствует или имеет не известное значение, то исходный файл удалится,
	//argv[5] -кэш директория если argv[4] - update

	//./imcheck /mnt/1_1908_31.imx6ull_gw_release_nandonly800.update /www/pages/update firmware noremove
	string source=argv[1];
	string dfolder=argv[2];
	string dimagename=argv[3];
	string cachedir="/var/run";
	Update_Finalize uf;
	string remove_mod="remove";
	Update_info passport;
	UpdateEngine_params ue_params;
	//printf("argc %d [%s]\n",argc,argv[4]);
	uf.remove_source=true;
	uf.update_after=false;
	if (argc>=5){
		if (strcmp(argv[4],"noremove")==0)
			uf.remove_source=false;
		if  (strcmp(argv[4],"update")==0){
			uf.update_after=true;
			uf.remove_source=false;
			printf("update_after=true\n");
		}
	}
	if (argc==6){
		if  (strcmp(argv[4],"update")==0)
			cachedir=argv[5];
	}

	ue_params.upad_interactive=0;
	ue_params.upad_source_remove=uf.remove_source;
	ue_params.upad_force=1;
	UpdateEngine ue(NORMAL_LEVEL,ue_params,NULL);

	SendSystemFromCnodaToUI(UPDPREP_EVENT_UPD_WEBCHECK);

	u32 ret_flags=ue.Update_fw_check(source,dfolder,passport);
	if (ret_flags==0){
		SendSystemFromCnodaToUI(UPDPREP_EVENT_UPD_WEBCHECK_OK);
		SendSystemFromCnodaToUI(UPDPREP_EVENT_UPD_WEBFINALIZE);
		if (ue.Finalize(passport,uf)==ERROR){
			printf("Error finalize\n");
			SendSystemFromCnodaToUI(UPDPREP_EVENT_UPD_WEBFINALIZE_ERR);
			ret_flags|=UPDATE_ERROR_ERROR_FINALIZE;
		}
		else
			SendSystemFromCnodaToUI(UPDPREP_EVENT_UPD_WEBFINALIZE_OK);

	}
	else
		SendSystemFromCnodaToUI(UPDPREP_EVENT_UPD_WEBCHECK_ERR);




	if (ret_flags&UPDATE_ERROR_INCORRECT_PASSPORT)
		printf("Incorrect passport\n");

	if (ret_flags&UPDATE_ERROR_INCORRECT_IMAGE)
		printf("Incorrect image\n");

	if (ret_flags==0){
		printf("Succes, firmware ready for update!!!\n");
		if (uf.update_after){
			if (ue.RunUpdateChain(dfolder,dfolder+'/'+dimagename,cachedir)==ERROR){
				printf("Error update\n");
			}
		}
	}
	else
	{
		SendSystemFromCnodaToUI(UPDPREP_EVENT_UPD_HIDE_SYSTEM_BLOCK);
	}

	exit(ret_flags);

}
