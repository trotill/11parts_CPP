/*
 * safe_storage.h
 *
 *  Created on: 17 дек. 2018 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_LOOP_SAFE_STORAGE_H_
#define SRVIOT_SRC_ENGINE_LOOP_SAFE_STORAGE_H_

#include "engine/thread.h"
#include "custom_project/custom_project.h"

class Safe_storage  : public ThreadT {
	public:
	Safe_storage(eDebugTp debug_level);
	virtual ~Safe_storage (void);
	eErrorTp CompareAndSaveSettings(SettingsAdm * Sm);
	eErrorTp RollOutArchiveConfigs(string json);
	eErrorTp initSignSettings();
	private:
		buffer FifoPreBuf;
		virtual eErrorTp Loop(void* thisPtr);
		SettingsAdm Sm;
		u32 loopdelay=100;//mS
		u32 syncinterval=600*1000;//iteration
		u32 saveinterval=20*1000;//iteration
		u32 sast_blockRebootRollout=0;

};



#endif /* SRVIOT_SRC_ENGINE_LOOP_SAFE_STORAGE_H_ */
