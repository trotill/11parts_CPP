/*
 * 11p_bin.h
 *
 *  Created on: 27 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_LIB_11P_BIN_H_
#define SRC_ENGINE_LIB_11P_BIN_H_

#include "engine/basic.h"

template <typename T>
eErrorTp BufToNParams(BufHandler * BufH, T * Params,u32 ParamCount)
{

	if (BufH->len==(sizeof(T)*ParamCount))
	{
		memcpy(Params,BufH->buf,BufH->len);
		return NO_ERROR;
	}
	else
		return ERROR;
}



void ChangeBit ( u8 nbit, u8 bit, u32 & val );
u8 GetBit ( u8 nbit, u32 val );
int Bits1CountCalc(u8 * value, u32 len);
void PowerOff(void);
u32 bitCopy(u8 * dst_buf,u32 dst_offset_bit,u32 src_size_bit,u8 * src_buf,u32 src_offset);
std::string bufferToString(char* buffer, int bufflen);
void GenMemBUG();
double Round(double data,u8 precision);

#endif /* SRC_ENGINE_LIB_11P_BIN_H_ */
