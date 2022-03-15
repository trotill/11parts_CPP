/*
 * necron.h
 *
 *  Created on: 10 дек. 2019 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_MINISW_NECRON_NECRON_H_
#define SRC_ENGINE_MINISW_NECRON_NECRON_H_

#include "engine/minisw/service/syslib.h"
#include "engine/periphery/watchdog.h"
//#define ROLL_OUT_FACTORY_DEBUG


eErrorTp StartPthread(Thread_t * thr,void *(*__start_routine) (void *),void * X);
eErrorTp StopPthread(Thread_t * thr);

#endif /* SRC_ENGINE_MINISW_NECRON_NECRON_H_ */
