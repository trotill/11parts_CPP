/*
 * 11p_process.h
 *
 *  Created on: 26 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_LIB_11P_PROCESS_H_
#define SRC_ENGINE_LIB_11P_PROCESS_H_

#include "../basic.h"

eErrorTp KillProcess(char * process,char * killstr,u32 time_ms);
string ExecResult(char* cmd,char* args);
string BashResult(string cmd);
int SearchProcess(const char* name);
u16 GetProgVersion(void);
void RebootSystem(void);
void PowerOffSystem(void);
int pclose2(FILE * fp, pid_t pid);
FILE * popen2(string command, string type, int & pid);

#endif /* SRC_ENGINE_LIB_11P_PROCESS_H_ */
