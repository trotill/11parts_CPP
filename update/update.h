/*
 * update.h
 *
 *  Created on: 19 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_UPDATE_UPDATE_H_
#define SRC_ENGINE_UPDATE_UPDATE_H_
#include "engine/basic.h"
#include "engine/print.h"
#include "engine/fifo.h"

#define UPDATE_ERROR_INCORRECT_PASSPORT 1
#define UPDATE_ERROR_INCORRECT_IMAGE 2
#define UPDATE_ERROR_INCORRECT_DESTINATION 4
#define UPDATE_ERROR_ERROR_FINALIZE 8
#define PASSPORT_MAX_SIZE 100000
#define PASSPORT_OFFSET_VAR1 0xF00000
#define UPDATE_BLOCK_SIZE_DEFAULT 8192
#define UPDATE_FW_TYPE_RAW 0
#define UPDATE_FW_TYPE_GZIP 1


#define UPDATE_EVENT_UPD_SUCCESS_DO_REBOOT "uevt_ok_rb"

#define UPDATE_EVENT_UPD_START_UPDATE "uevt_start_update"

#define UPDATE_EVENT_UPD_END_UPDATE_OK "uevt_end_update_ok"
#define UPDATE_EVENT_UPD_END_UPDATE_ERR "uevt_end_update_err"

#define UPDATE_EVENT_UPD_WRITE_ROOTFS "uevt_write_rfs_img"
#define UPDATE_EVENT_UPD_WRITE_FIT "uevt_write_fit_img"
#define UPDATE_EVENT_UPD_BACKUP_SETT "uevt_backup_sett"
#define UPDATE_EVENT_UPD_ERROR "uevt_error"
#define UPDPREP_EVENT_UPD_COMPLETE "uprep_complete"

#define UPDPREP_EVENT_UPD_START_SEARCH "uprep_start_search"

#define UPDPREP_EVENT_UPD_END_SEARCH "uprep_end_search"
#define UPDPREP_EVENT_UPD_SKIP_IDENTICAL "uprep_identical"
#define UPDPREP_EVENT_UPD_SKIP_ERR_HW "uprep_err_hardware"
#define UPDPREP_EVENT_UPD_REBOOT "uprep_reboot"
#define UPDPREP_EVENT_UPD_HIDE_SYSTEM_BLOCK "hide_system"

#define UPDPREP_EVENT_UPD_WEBCHECK "uprep_wcheck"
#define UPDPREP_EVENT_UPD_WEBCHECK_OK "uprep_wcheck_ok"
#define UPDPREP_EVENT_UPD_WEBCHECK_ERR "uprep_wcheck_err"
#define UPDPREP_EVENT_UPD_WEBFINALIZE "uprep_wfinalize"
#define UPDPREP_EVENT_UPD_WEBFINALIZE_OK "uprep_wfinalize_ok"
#define UPDPREP_EVENT_UPD_WEBFINALIZE_ERR "uprep_wfinalize_err"
//Object MSG uprep
#define UPDPREP_EVENT_UPD_CHECK "check"
#define UPDPREP_EVENT_UPD_FOUND_FW "found_fw"
#define UPDPREP_EVENT_UPD_SELECT_VERS "select_version"
#define UPDPREP_EVENT_UPD_FINALIZE "finalize"
#define UPDPREP_EVENT_UPD_CHECK_OK "check_ok"
#define UPDPREP_EVENT_UPD_CHECK_ERR "check_err"
//Object

class Update_info{
	public:
	string dump(void){
		Json::StyledWriter wr;
		return wr.write(passport);
	}

	Json::Value passport;
	string passport_str;
	u32 status=0;
	string source_file;
};

class Update_Finalize{
	public:
	bool remove_source=false;
	bool update_after=false;
	//string dest_file="/www/pages/update/firmware";
};

//Deprecated, use only nand
typedef enum {runfromUpdater,runfromFactory,runfromUser} enrunfromSt;




class UpdateEngine_params{
	public:
	//тип файла обновления, по умолчанию все файлы с расширением update считать обновлением
	string upad_firmware_file="update";//Это не расширение и не имя файла, это тип обновления. К названию файла отношения не имеет
	//накопители где искать обновление
	string upad_storages_str="[\"sda1\",\"sdb1\"]";
	Json::Value upad_storages;
	//обновлять в интерактивном режиме
	u32 upad_interactive=1;
	//удалить исходный файл обновления, в случае успешного обновления
	u32 upad_source_remove=1;
	//обновлять даже если устройство обновлено этой же прошивкой
	u32 upad_force=0;
};
class UpdateEngine : public GprintT, public UpdateEngine_params {
	public:


		UpdateEngine(eDebugTp debug_level,UpdateEngine_params & params,FifoEngineT * msg_sender);
		eErrorTp DoUpdate(char * json);
		//eErrorTp DO_Update(Update_info & passport,u8 update_type);
		eErrorTp DO_Update(Update_info & passport,u8 update_type,enrunfromSt runFrom,string firmware_path);
		eErrorTp DO_Update_FIT_ROOTx2_RWx1(Update_info & passport,u8 update_type,enrunfromSt runFrom,string FIRMWARE_PATH);
		//eErrorTp UpdateImageStd(string & json_req,string & json_resp);
		eErrorTp FIT_ROOTx2_RWx1_Finalize(Update_info & passport,bool AllowCopyFw,bool remove_source);
		eErrorTp FIT_UBIx2_RWx1_fill_zero(char * PassportDataRawSave,char* PassportDataRaw,string fw,u32 offset);
		eErrorTp FIT_UBIx2_RWx1_backup_passport(char * PassportDataRawSave,string fw,u32 offset);
		eErrorTp Finalize(Update_info & passport,Update_Finalize & params);
		eErrorTp UpdateBackupSettings(Json::Value & passport);
		eErrorTp UpdatePlaceStump(void);
		eErrorTp SaveFinger(string dfold,string finger);
		eErrorTp SavePassport(string dfold,char * passport,string passportname);
		u32 Update_fw_check(string & file,string & dfolder,Update_info & passport);
		//eErrorTp GetImagePassport(string fname, u32 offset);
		eErrorTp GetImagePassport(string fname, u32 offset,char * result,u32 result_size);
		eErrorTp RunUpdateChain(string firmware_path,string firmware_file,string cache_path);
		eErrorTp CheckAndFoundUpdate(
				Json::Value & upad_storages,
				vector <string> & mount_points,
				vector <Update_info> & updinfo_list,
				string dfolder
				);
		eErrorTp UmountAllUpdateStrorages(vector <string> & mount_points);
		eErrorTp GetLastVersionUpdate(vector <Update_info> & updinfo_list,u32 & updinfo_list_fidx);
		eErrorTp CheckHWFromTarget(Update_info & source_update);
		eErrorTp CheckByPassportFromTarget(Update_info & source_update);
		eErrorTp CheckAndUpdate(void);
		eErrorTp CheckUpdateMarker(void);
		eErrorTp Finalize_update(Update_info & updinfo);
		void RemoveUpdateMarker(void);
		eErrorTp FindAndPrepareUpdate(void);
		eErrorTp SendSystemFromCnodaToUI_upd(string str);
		//eErrorTp Finalize(Update_info & passport,Update_Finalize & params);
		//typedef eErrorTp (SSystemFromCnodaToUI) (string);
		//SSystemFromCnodaToUI * SendSystemFromCnodaToUI;
	private:
		char PassportDataRaw[PASSPORT_MAX_SIZE];
		char PassportDataRawSave[PASSPORT_MAX_SIZE];
		FifoEngineT * msender=NULL;

		//eErrorTp (SendSystemFromCnodaToUI) (string dMessageJson);

	//eErrorTp CheckUpdateMarker(void);
	//void RemoveUpdateMarker(void);
};

eErrorTp DoUpdate(char * json);
eErrorTp UpdateImageStd(string & json_req,string & json_resp);
//eErrorTp UpdateImageStd(string & json_req,string & json_resp);

#endif /* SRC_ENGINE_UPDATE_UPDATE_H_ */
