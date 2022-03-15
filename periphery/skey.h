/*
 * skey.h
 *
 *  Created on: 16 окт. 2019 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_PERIPHERY_SKEY_H_
#define SRVIOT_SRC_ENGINE_PERIPHERY_SKEY_H_

#ifdef _SKEY
#include <engine/types.h>
#include <engine/basic.h>
#include "custom_project/protected/skey_params.h"

class eskeyp_default_for_example{
	protected:
	bool one_demand_lic=false;
	bool autogen_lic=true;
	string autogen_stor_dev="/dev/mmcblk0p1";
	string autogen_lic_key_file="virus.exe";

	string devfile="";
	u32 devfile_offs=0;
	u32 devfile_size=0;
	bool key_sd=true;
	bool back_to_factory_if_lic_false=false;
	bool kill_device_if_lic_false=false;
	bool reboot_if_lic_false=false;
	u8 key_sd_num=0;
	bool key_cpu_ull=false;
	string admin_key="WersEwqw23drov**#7422dcmvf31";
	char * LIBC="lib_0.0";
	char * KENT_SALT="3m3o4sdfksorr[w[sc]]";
	void kill_device(){

	}
};

class kent:public eskeyp{
	private:
	char * __sk=NULL;
	public:
	string rdmd(string dev,long int offs,u32 size);
	string kesd();
	eErrorTp klk(void);
	eErrorTp place(string k);
	eErrorTp autogen();
	kent();
	~kent();
	void f__sk(void);
};

#endif

#endif /* SRVIOT_SRC_ENGINE_PERIPHERY_SKEY_H_ */
