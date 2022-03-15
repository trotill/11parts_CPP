/*
 * syslib.h
 *
 *  Created on: 3 сент. 2020 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_MINISW_SERVICE_SYSLIB_H_
#define SRVIOT_SRC_ENGINE_MINISW_SERVICE_SYSLIB_H_

#include <wordexp.h>
#include <sys/types.h>
#include "engine/basic.h"
#include "engine/lib/11p_process.h"
#include "engine/lib/11p_string.h"
#include "engine/lib/11p_json.h"
#include "engine/lib/11p_files.h"

extern int proclink_stdout[];
extern int proclink_stderr[];
char **split_commandline(const char *cmdline, int *argc);

eErrorTp Exe(pid_t & pID,string prg,string args);
eErrorTp Shedule(string prog,string arg);

#endif /* SRVIOT_SRC_ENGINE_MINISW_SERVICE_SYSLIB_H_ */
