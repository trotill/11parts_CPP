/*
 * tty.h
 *
 *  Created on: 21 нояб. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_PERIPHERY_TTY_H_
#define SRC_ENGINE_PERIPHERY_TTY_H_

#include "engine/print.h"
 #include <poll.h>

///The class defines access to the COM port
class TTYclass : public GprintT{
	public:
		TTYclass(string & tty, string & parens, char * name, u32 speed, u8 stopbit, int exflags, eDebugTp log_level);
		TTYclass(string & tty, string & parens, char * name, u32 speed, u8 stopbit, int exflags,u32 openparam,eDebugTp log_level);
		eErrorTp Init(string & tty, u32 brate, u8 stopbit, int exflags,u32 openparam);
		~TTYclass(void);
		void TTYclose(void);
		eErrorTp TTYreadChar(u8 & buff);
		u32 TTYreadBuff(u8 * buff);
		u32 TTYnreadBuff(u8 * buff,u32 max_size);
		eErrorTp TTYwriteChar(u8 & buff);
		eErrorTp TTYwriteBuf(u8 * buff, u32 len);
		eErrorTp TTYwriteBufBlocked(u8 * buff, u32 len);
		u32 TTYnreadBuffBlocked(u8 * buff,u32 max_size,u32 timeout);
		eErrorTp TTYClear(void);
		int setRTS(int level);
		eErrorTp ConfRS485();
		int comfd=-1;
		//fd_set readfs;
		eErrorTp glbERROR;
	protected:
		string TTYFname;
		struct pollfd fds;
		struct pollfd fdwr;
		//struct timeval RxTimeout;
		eErrorTp TTYopen(u32 openparam);
};

#endif /* SRC_ENGINE_PERIPHERY_TTY_H_ */
