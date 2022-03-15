/*
 * crc.h
 *
 *  Created on: 10 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ALGO_CRC_H_
#define SRC_ALGO_CRC_H_

#include "engine/basic.h"

uint8_t CalcCRC8_4(uint8_t initValue, uint8_t Data0, uint8_t Data1, uint8_t Data2, uint8_t Data3);
void CalcCRC8_ShotInit(u8 & crc,uint8_t initValue);
void CalcCRC8_Shot(u8* data, u8 &crc, u16 cnt);
u8 CalcCRC8(u8* data, u16 cnt,  u8 initValue);
u32 Crc32Buf(u8 * buf, size_t len);
void Crc32ShotInit(u32 & crc);
void Crc32Shot(u32 & crc,u8 * buf, size_t len);
void Crc32ShotStop(u32 & crc);

#endif /* SRC_ALGO_CRC_H_ */
