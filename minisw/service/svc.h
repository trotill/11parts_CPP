/*
 * svc.h
 *
 *  Created on: 29 нояб. 2019 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_MINISW_SERVICE_SVC_H_
#define SRVIOT_SRC_ENGINE_MINISW_SERVICE_SVC_H_

#include "engine/logger/safe_logger.h"
#include "engine/logger/safe_logger_client.h"
#include "engine/ipc_socket.h"
#include "engine/lib/11p_bin.h"
#include "engine/minisw/service/syslib.h"

#define SLOGGER_PIPE_PATH "/run/slogger"
#define MAX_ARGS 100
shared_ptr<safelogger_client> process_log;


#endif /* SRVIOT_SRC_ENGINE_MINISW_SERVICE_SVC_H_ */
