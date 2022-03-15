/*
 * factory_reset.h
 *
 *  Created on: 23 дек. 2018 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_LOOP_FACTORY_RESET_H_
#define SRVIOT_SRC_ENGINE_LOOP_FACTORY_RESET_H_


#include "engine/thread.h"
#include "custom_project/custom_project.h"
#include "engine/periphery/gpio.h"

class Factory_reset  : public ThreadT {
	public:
	Factory_reset(eDebugTp debug_level);
	virtual ~Factory_reset (void);
	private:
		buffer FifoPreBuf;
		virtual eErrorTp Loop(void* thisPtr);
		u32 loopdelay=100;//mS
		u32 detectinterval=200;//iteration
		u32 reset_to_factory_on_gpio=0;
		u32 reset_to_factory_on_key=0;
		u32 reset_gpio=FACTORY_RESET_GPIO;
		u32 reset_key=KEY_SPACE;
		int reset_value=2;
		int fd_input_dev=-1;
		struct input_event ie;
		char * reset_input_dev="/dev/input/event0";
};


#endif /* SRVIOT_SRC_ENGINE_LOOP_FACTORY_RESET_H_ */
