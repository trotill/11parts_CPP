/*
 * update_admin.h
 *
 *  Created on: 19 авг. 2019 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_LOOP_UPDATE_ADMIN_H_
#define SRC_ENGINE_LOOP_UPDATE_ADMIN_H_
/*
 * накопители где искать обновление
 * upad_storages:[sda1,sdb1]
 *
 * тип файла обновления, по умолчанию все файлы с расширением update считать обновлением
 * upad_firmware_file:update
 *
 * обновлять в интерактивном режиме
 * upad_interactive:0
 *
 * удалить исходный файл обновления, в случае успешного обновления
 * upad_source_remove:1
 *
 * обновлять даже если устройство обновлено этой же прошивкой
 * upad_force:1
 */

#include "engine/thread.h"
#include "custom_project/custom_project.h"
#include "engine/update/update.h"

class Update_admin  : public ThreadT {
	public:
	Update_admin(eDebugTp debug_level);
	virtual ~Update_admin (void);
	//eErrorTp Finalize_update(string firmware_path,string firmware_name,Update_info & updinfo);

	//eErrorTp CheckAndFoundUpdate(
	//		Json::Value & upad_storages,
	//		vector <string> & mount_points,
	//		vector <Update_info> & updinfo_list,
	//		string firmware_path
	//		);
	//eErrorTp CheckAndUpdate(void);
	//eErrorTp FindAndPrepareUpdate(void);
	//eErrorTp UmountAllUpdateStrorages(vector <string> & mount_points);
	//eErrorTp GetLastVersionUpdate(vector <Update_info> & updinfo_list,u32 & updinfo_list_fidx);
	//eErrorTp CheckByPassportFromTarget(Update_info & source_update);
	//eErrorTp CheckHWFromTarget(Update_info & source_update);
	//void RemoveUpdateMarker(void);
	//eErrorTp CheckUpdateMarker(void);
	private:
		virtual eErrorTp Loop(void* thisPtr);
		buffer FifoPreBuf;
		u32 loopdelay=1000;//mS
		UpdateEngine_params ue_params;
		shared_ptr<UpdateEngine> ue;
		bool update_checked=false;
		bool find_update_on_storages=false;

};



#endif /* SRC_ENGINE_LOOP_UPDATE_ADMIN_H_ */
