/*
 * auth.h
 *
 *  Created on: 19 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_AUTH_H_
#define SRC_ENGINE_AUTH_H_

#include "node.h"
#include "settings_adm.h"

#ifdef _SECURE_ENABLE
#include "engine/periphery/skeyV2.h"
#endif

#define MAX_GROUP 10
#define MAX_USERS 10
#define GROUP_FILE_PATH "/var/run/groups.json"

class sAuthAccess
{
	public:
		string Password;
		string UserName;
		string CipherPassword;
		string CipherUser;
};

class SecureAccessT :public GprintT
{
	public:
	SecureAccessT(string grp,u32 max);
	 ~SecureAccessT();
	 eErrorTp Change(string & password,string & oldlogin,string & newlogin);
	 eErrorTp Add(string & password,string & oldlogin,string & newlogin);
	 eErrorTp Check(string & password, string & login);
	 string GetAccounts(void);
	 u32 max_account;
	 string group;
	 bool oneUserMode=true;//режим одного пользователя, нельзя добавлять и удалать, в каждом аккаунте всегда один пользователь
	 protected:
	 eErrorTp ChangeAdminAccount(string & oldusername,string & username,string & password);
	 eErrorTp SaveAdminAccount(string & username,string & password);
	 eErrorTp LoadAdminAccount(sAuthAccess * acc);
	 eErrorTp EncodeSecureAlgo(string & username,string & password,string & codeduser,string & codedpasswd);
	 eErrorTp DecodeSecureAlgo(string & codeduser,string & codedpasswd,string & username,string & password);
	 sAuthAccess * sAlist;
	 SettingsAdm Sm;

};

eErrorTp auth_ChangePasswordV2(GprintT * Obj,SecureAccessT** GroupList,Json::Value jsonReq,string & json_resp);
eErrorTp auth_ChangePassword(GprintT * Obj,SecureAccessT** GroupList,Json::Value jsonReq,string & json_resp);
eErrorTp auth_SyncAuth(GprintT * Obj,SecureAccessT** GroupList);




#endif /* SRC_ENGINE_AUTH_H_ */
