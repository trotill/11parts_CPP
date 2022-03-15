/*
 * watchdog.h
 *
 *  Created on: 29 нояб. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_PERIPHERY_WATCHDOG_H_
#define SRC_ENGINE_PERIPHERY_WATCHDOG_H_

#include "engine/print.h"
#include <linux/watchdog.h>

class watchdog:public GprintT  {
	public:
	watchdog(string dev,u32 timeout,u32 enable,eDebugTp lvl);
	eErrorTp keepalive(u32 timeout);
	~watchdog(void);
	private:
		int wdt=0;
		int print_wdt_cntr=0;
		u32 timeo;

};

class wadtchdog_thread{
	public:
	wadtchdog_thread(u8 * thr_name_ch,u32 max_tick){
		this->thr_name=(char*)thr_name_ch;
		this->max_tick=max_tick;
		enable=true;
		source_tick=0;
	}
	string thr_name="noname";
	bool enable=false;
	u32 max_tick=40;
	u32 source_tick=0;
};
#endif /* SRC_ENGINE_PERIPHERY_WATCHDOG_H_ */
