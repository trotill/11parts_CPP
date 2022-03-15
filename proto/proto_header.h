/*
 * proto_header.h
 *
 *  Created on: 28 дек. 2018 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_PROTO_PROTO_HEADER_H_
#define SRVIOT_SRC_ENGINE_PROTO_PROTO_HEADER_H_

#include "engine/types.h"

#pragma pack(push, 1)
typedef struct sBoardGpio
{
	u8 GPIO;
	u8 Val;
}sBoardGpio;

typedef struct siBoardProtoHeader
{
	u8 Type;
	u16 lendata;
	u8 Reserved;//0 - MessageHandler
}siBoardProtoHeader;
#pragma pack(pop)


#define SETTING_EXTENSION_SIGN ".sgn"
#define SETTING_EXTENSION ".set"
#define SETTING_EXTENSION_CRC ".crc"




#pragma pack(push, 1)


typedef struct sProcessResponse
{
	u8 MSG;
	u8 Result;
	u8 * respData;
}sProcessResponse;

#pragma pack(pop)




#endif /* SRVIOT_SRC_ENGINE_PROTO_PROTO_HEADER_H_ */
