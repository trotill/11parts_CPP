/*
 * gpio.h
 *
 *  Created on: 30 июля 2014 г.
 *      Author: root
 */

#ifndef GPIO_H_
#define GPIO_H_
#include "engine/lib/11p_string.h"
#include "engine/lib/11p_process.h"

eErrorTp GpioInInit(u8 gpio);
eErrorTp GpioOutInit(u8 gpio, u8 defval);
eErrorTp GpioOnOff(u8 gpio,u8 val);
u8 GpioInRead(u8 gpio);

class gpio{
	public:
	gpio(string num,string direct,string value);
	~gpio(void);
	eErrorTp set(char * val);
	u32 get(void);
	private:
		int  fd_val;
		int  fd_dir;
		string ngpio;
		u32 access;
};

#endif /* GPIO_H_ */
