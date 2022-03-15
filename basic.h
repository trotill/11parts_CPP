/*
 * basic.h
 *
 *  Created on: 31 июля 2014 г.
 *      Author: root
 */

#ifndef BASIC_H_
#define BASIC_H_

#include <string>
#include <iostream>

#include <vector>
#include <queue>
#include <map>
#include <memory.h>
#include <memory>
#include <string>
#include <list>
#include <algorithm>
#include <regex>
#include <bitset>
using namespace std;
#include <cstring>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <sstream>
#include <functional>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/statvfs.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <termio.h>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/times.h>
#include <dirent.h>
#include <linux/icmp.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <ftw.h>
#include <time.h>
#include <memory>
#include <thread>
#include <pthread.h>
#include <unordered_map>
#include <json/json.h>
#include "engine/lib/rapidjson/document.h"
#include "engine/lib/rapidjson/writer.h"
#include "engine/lib/rapidjson/prettywriter.h"
#include "engine/lib/rapidjson/stringbuffer.h"
#include <stdarg.h>
#include "custom_project/message_types.h"
#define X86
#define os_slash        '/'
#define mdelay(a) usleep(a*1000)
#define delay(a) sleep(a)
#define Mutex_t pthread_mutex_t
#define mutex_init(a) pthread_mutex_init(&a,NULL);
#define mutex_lock(a) pthread_mutex_lock(&a);
#define mutex_unlock(a) pthread_mutex_unlock(&a);

#define Thread_t pthread_t
#define PROGRAMM_NAME_EXE ""
#define FLOAT_SEPARATOR '.'


#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <memory>
#ifndef DEFSTDTYPE
typedef int s32;
#if LONG_MAX==9223372036854775807
typedef long int s64;
#else
typedef long long int s64;
#endif
typedef int int32_t;
typedef int int_32;
typedef short s16;
typedef short int16_t;
typedef short int_16;
typedef char s8;
typedef char int_8;

typedef float f32;

typedef double d64;
typedef unsigned int u32;
#if LONG_MAX==9223372036854775807
typedef unsigned long int u64;
#else
typedef unsigned long long int u64;
#endif
typedef unsigned int uint32_t;
typedef unsigned int uint_32;
typedef unsigned short u16;
typedef unsigned short uint16_t;
typedef unsigned short uint_16;
typedef unsigned char u8;
typedef unsigned char uint8_t;
typedef unsigned char uint_8;
typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
#endif

#define TERM_REQ_RISE 1
#define TERM_STATE_RISE 1
#define ACTIVE_STATE_RISE 0
#define NO_REQ_RISE 0

#define MAX_RCV_CLNT_BUF 0xffff
#define MAX_RCV_CLNT_OVER_SIZE MAX_RCV_CLNT_BUF*4
#define MAX_RCV_CLNT_BUF32 (MAX_RCV_CLNT_BUF>>2)
#define STUB_MAGIC_STRING "stub_mode"

typedef enum {NO_USE,NORMAL_LEVEL,MEDIUM_LEVEL,HARD_LEVEL,PARANOIDAL_LEVEL,FILE_LEVEL=10} eDebugTp;
typedef enum {RUN,QUIT} eQuitTp;
typedef enum {ERROR1=-1,NO_ERROR=0,ERROR=1} eErrorTp;
typedef enum {ssCLOSE_SOCK,ssOPEN_SOCK,ssERROR_SOCK} eSockStateTp;

enum {srvmUNDEF=200,
	srvmSendToLH=201 ,
	srvmJNODA_READY=202,
	srvmWDT=203,
	srvmCHECK_STOR_FOR_UPDATE=204,
	srvmWEB_CLIENT_ACT=205,
	srvmSOCKET_IO=206,
	srvmSendUI_JSON_BUF=207,
	srvmSKEY=208,
	srvmReport=209,
	srvmReportSettings=210,
	srvmJSON_ACTION_FORMAT=211,
	srvmSAST_FORCE_COMP_SAVE=212,
	srvmWEBEVENT=213,
	srvmSAST_ROLLOUT_ARCHIVE_CONFIGS=214,//евент о начале переконфигурации
	srvmAPPLY_ONE=215,
	srvmRAW_EVENT=216,
	srvmMQ_MQTT_READY=217,
	srvmMQ_MESSAGE=218,
	srvmEXT_STOR=219,
	srvmMODBUS_CONFIG=220,
	srvmMODBUS_MESSAGE=221,
	srvmMOSQUITTO_READY=222,
	srvmSAST_ROLLOUT_ARCHIVE_READY=223,//переконфигурация завершена
};
	//srvmReportAction=211,
	//srvmRecvSMS=212};

#define MB 0x100000//1048576

#pragma pack(push, 1)
//Заголовок пакета межпотокового взимодействия
typedef struct sInterThrMsgHeader
{
    u8 deprecated_SrvType;//eSrvTypeTp
	u8 MsgType;//eMsgTypeTp
}sInterThrMsgHeader;
#pragma pack(pop)

#define TIME_T32 u32
#define TIME_T TIME_T32

template <typename T>
TIME_T TIME(T * var){
	time_t tv;

	if (var!=NULL){
	 time_t ttmp=(time_t)*var;
	 tv=time(&ttmp);
	 *var=(T)tv;
	}
	else{
		tv=time(NULL);
	}

	TIME_T ret=(TIME_T)tv;

	return ret;
}

template <typename T>
struct tm *	GMTIME(T * var){
	time_t tv=(time_t)*var;
	return gmtime(&tv);
}
//extern struct tm *localtime (const time_t *__timer)
template <typename T>
void LOCALTIME(T * var,struct tm * res){
	time_t tv=(time_t)*var;
	memcpy(res,localtime(&tv),sizeof(struct tm));
}


#pragma pack(push, 1)
typedef struct BufHandler
{
	u32 free_size;
	u32 base_len;
	u32 max_size;
	u32 len;
	u8 * buf;
	u8 * buf_base;
}BufHandler;

#define PLACE_BUF(a) BufHandler a; a.buf_base=NULL;
#define zr() printf("trace: %s line %d, %s\n",__FUNCTION__,__LINE__,__FILE__);

typedef struct sSock
{
	int sockSrc;//recv
	int sockDst;//send
	eSockStateTp stat;
	struct sockaddr_in DestAddr;//send
	struct sockaddr_in SourceAddr;//recv
}sSock;

#define PROGRAMM_NAME "Cnoda"
#define PROGRAMM_FILE_NAME PROGRAMM_NAME PROGRAMM_NAME_EXE
#define MAX_TCPUDP_QUEUE_SIZE 400000
#define MIN_BUSY_TCPUDP_QUEUE_SIZE 20000
#define MAX_SYSTEM_TIME 0xffffffff
#define UMOUNT_STATE 0
#define MOUNT_STATE 1

extern pthread_mutex_t glbSigMutex;

eErrorTp MiniSW_Run(int argc,char *argv[],string SWname);


#pragma pack(pop)
#endif /* BASIC_H_ */
