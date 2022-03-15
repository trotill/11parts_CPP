/*
 * minisw.h
 *
 *  Created on: 28 сент. 2017 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_MINISW_MINISW_H_
#define SRC_ENGINE_MINISW_MINISW_H_

//#include "ipc_socket.h"
//#include "guards.h"
//#include "engine/algo/crc.h"

#include "engine/global.h"
#include "engine/ipc_socket.h"
#include "engine/proto/json_proto.h"

#define EV_SENDER_METHOD_EVENT 0
#define EV_SENDER_METHOD_RAW 1
#define USB_RESET_SW "usb_reset"
#define WDT_PROCESS_NAME "evwdt"
#define NODE_PROCESS_NAME "evnode"
#define CNODA_PROCESS_NAME "evcnoda"
#define WATCHDOG "watchdog_stub"
#define LOAD_SETTINGS_PROCESS_NAME "gset"
#define SAVE_SETTINGS_PROCESS_NAME "sset"
#define SETTINGS_CLEAN_PROCESS_NAME "setclean"
#define SETTINGS_GET_PRIVATE "gprivate"
#define TO_FACTORY "factory"
#define SAFE_LOGGER_FW "safe_logger"
#define IMAGE_CHECK "imcheck"
#define SNMP_AGENT "snmpagnt"

//IPC_SocketT
class EvSender:public IPC_SocketT{
	public:
	EvSender(string & ToProcess,char * event, char * args,int method,int packtype);

};


int usb_reset_main(int argc, char **argv);
int imcheck_main(int argc, char **argv);
int snmp_main(int argc, char **argv);
int load_settings(int argc, char **argv);
int save_settings(int argc, char **argv);
int settings_clean_main(int argc, char **argv);
int settings_private(int argc, char **argv);
int to_factory(int argc, char **argv);
int watchdog_stub(int argc, char **argv);
int safe_logger(int argc, char **argv);
eErrorTp EventSender(int argc,char *argv[],string SWname);

eErrorTp SendSystemFromCnodaToUI(string dMessageJson);
eErrorTp SendSystemFromCnodaToUI(string dMessageJson,string user);
eErrorTp SendEventFromCnodaToUI(string dMessageJson,string user);
eErrorTp SendEventFromCnodaToUI(string dMessageJson);

#endif /* SRC_ENGINE_MINISW_MINISW_H_ */
