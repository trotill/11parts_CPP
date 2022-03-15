/*
 * watchdog.cxx
 *
 *  Created on: 29 дек. 2018 г.
 *      Author: root
 */


#include "watchdog.h"
#include "engine/proto/json_proto.h"
//#include "engine/global.h"


watchdog::watchdog(string dev,u32 timeout,u32 enable,eDebugTp lvl){
		SourceStr="WDT";
		ObjPref="watchdog";
		//debug_level=NORMAL_LEVEL;
		//printf("watchdog::watchdog enable %d\n",enable);
		if (enable==0){
			GPRINT(NORMAL_LEVEL,"WDT disabled\n");
			return;
		}
		GPRINT(NORMAL_LEVEL,"WDT enabled\n");
		timeo=timeout;
		//u32 lvl=(u32)MEDIUM_LEVEL;
		//JSON_ReadConfigField(CnT->json_cfg,"watchdog_loglevel",lvl);
		debug_level=(eDebugTp)lvl;

		wdt=open(dev.c_str(),O_WRONLY);
		ioctl(wdt, WDIOC_SETTIMEOUT, &timeo);
		ioctl(wdt, WDIOC_KEEPALIVE, 0);
	}

watchdog::~watchdog(void){
	if (wdt!=0)
		close(wdt);
}

eErrorTp watchdog::keepalive(u32 timeout){
		if (wdt!=0){
			if (timeout!=0)
				ioctl(wdt, WDIOC_SETTIMEOUT, &timeout);
			ioctl(wdt, WDIOC_KEEPALIVE, 0);
			print_wdt_cntr++;
			if (print_wdt_cntr>=100){
				GPRINT(HARD_LEVEL,"Sent 100 keepalive, ok\n");
				print_wdt_cntr=0;
			}

		}
		return NO_ERROR;
}


