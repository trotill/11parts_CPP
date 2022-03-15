/*
 * guard_loop.h
 *
 *  Created on: 27 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_LOOP_GUARD_LOOP_H_
#define SRC_ENGINE_LOOP_GUARD_LOOP_H_

#include "engine/thread.h"
#include "custom_project/custom_project.h"
#include "engine/periphery/gpio.h"

class Guard_loop  : public ThreadT {
	public:
	Guard_loop(eDebugTp debug_level);
	virtual ~Guard_loop (void);

	private:
		virtual eErrorTp Loop(void* thisPtr);
		buffer FifoPreBuf;
		u32 loopdelay=100;//mS
		u32 GetUptime(void);
		eErrorTp extStorageAdmin();
		eErrorTp extUmount();
		eErrorTp GetTemperature(void);
		eErrorTp CalcWorkTime(bool force);
		void tot_init(void);
		//TIME_T tot_time=0;
		SettingsAdm Sm;
		u32 saved_motohour=0;
		u32 motohour_base=0;
		u32 source_work_ts=0;
		u32 old_source_work_ts=0;
		Json::Value extStorageDev;
		string extStorageDevMounted;
		string extStorageMountPoint="/mnt";
		u8 extStorageFound=0;
		u8 doMount=1;
		u8 extStorageMount=0;
		u8 extStorageEnable=0;
		u32 uptime=0;

		//string tot_file;
		u32 tempinterval=5000;//send temp every 5sec
		u32 totinterval=600000;//send and save every 10min
		bool gulo_disable_tot=false;
		string cpu_temp_sysfs_path="/sys/class/hwmon/hwmon0/temp1_input";
};



#endif /* SRC_ENGINE_LOOP_GUARD_LOOP_H_ */
