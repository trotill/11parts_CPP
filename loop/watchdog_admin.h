/*
 * watchdog_admin.h
 *
 *  Created on: 23 дек. 2018 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_LOOP_WATCHDOG_ADMIN_H_
#define SRVIOT_SRC_ENGINE_LOOP_WATCHDOG_ADMIN_H_

#include "engine/thread.h"
#include "custom_project/custom_project.h"

class Watchdog_admin  : public ThreadT {
	public:
	Watchdog_admin(eDebugTp debug_level);
	eErrorTp fs_check();
	virtual ~Watchdog_admin (void);
	private:
		virtual eErrorTp Loop(void* thisPtr);
		buffer FifoPreBuf;
		u32 loopdelay=100;//mS
		u32 EnableWDT=0;
		u32 TimeoutWDT=60;
		u32 WDTinterval=50;
		u32 thread_hung_max_tick=50;
		Json::Value DrivesWDT;
		string fifo_file="/run/wdt";
		bool pipe_wdt=false;
		int fifo_fd=0;
		bool CheckDrive=false;
};



#endif /* SRVIOT_SRC_ENGINE_LOOP_WATCHDOG_ADMIN_H_ */
