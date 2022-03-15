/*
 * auth.cxx
 *
 *  Created on: 19 дек. 2018 г.
 *      Author: root
 */

#include "auth.h"
#include "engine/algo/MD5.h"


SecureAccessT *  FindOrCreateGroup(SecureAccessT** GroupList,string & groupname)
{
	int i;
	SecureAccessT * SA=NULL;
	for (i=0;i<MAX_GROUP;i++) {
		if (GroupList[i]==NULL)
				break;
		else {
			if (GroupList[i]->group==groupname)
					SA=GroupList[i];
			}
	}
	if (SA==NULL)
		SA=GroupList[i]=new SecureAccessT(groupname,MAX_USERS);
	return SA;
}

u32 GetGroupPriority(string group)
{
	std::ifstream  fil;
	string grp_pr_file=string_format("%s/groups.prio.json",CnT->CACHE_PATH.c_str());
	fil.open(grp_pr_file.c_str());
	string rdata;
	if (fil.is_open())
	{
	  rdata.assign((istreambuf_iterator<char>(fil)),istreambuf_iterator<char>());
	  fil.close();
	  Json::Value root;
	  Json::Reader reader;
	  u32 ngp=0;
	  if ((reader.parse( rdata.c_str(), root )==true)&&(root.isArray())) {
		  while(root[ngp].isNull()==false)
		  {
			 if (root[ngp].asString()==group) {
				 return ngp;
			 }
			 ngp++;
		  }

	  }
	}
	return 1000;
}


#ifdef _SECURE_ENABLE
void secureCheck(epsec::secure & sec){

	//epsec::secure sec;
	string secLicKey;
	if (sec.getLicKey(secLicKey)==NO_ERROR){
		sec.setDeviceUID(CnT->deviceUID);
		CnT->licKey=sec.decryptLicenseData(secLicKey);
		if (sec.checkKey(CnT->licKey)==NO_ERROR){
			if (sec.checkAuthSign(CnT->licKey)==NO_ERROR){
				CnT->authorised=true;
			}
			else{
				CnT->authorised=false;
			}
		}
		else{//ключ устройства не подходит
			CnT->authorised=false;
		}
	}
	else{//отсутствует файл лицензии
		CnT->authorised=false;
	}
}
#endif

eErrorTp InitAuth(GprintT * Obj,SecureAccessT** GroupList)
{
	string group_data;
	std::ifstream  group_file;

	group_file.open(GROUP_FILE_PATH);
	string group_json_str;
	if (group_file.is_open()){
		group_file >> group_json_str;
		group_file.close();
	}
	Json::Reader reader;
	Json::Value root;
	u32 i=0;
	u32 ngp=0;
	bool parsingSuccessful = reader.parse( group_json_str.c_str(), root );

	//printf("Parse %s\n",GROUP_FILE_PATH);
	string reg_name;
	if ((parsingSuccessful)&&(root.size()>0)){
			for( Json::Value::const_iterator itr = root.begin() ; itr != root.end() ; itr++ ) {
				//printf("region %s\n",itr.key().asString().c_str());
				reg_name=itr.key().asString();

			    for( Json::Value::const_iterator itr_grp = root[reg_name.c_str()].begin() ; itr_grp != root[reg_name.c_str()].end() ; itr_grp++ ) {
			    	Obj->GPRINT(NORMAL_LEVEL, "Found group (%s) for region %s\n", itr_grp.key().asString().c_str(),reg_name.c_str());
			    	if ((CnT->json_cfg.isMember("account_groups"))&&
			    			CnT->json_cfg["account_groups"].isArray())
			    	{
			    		Obj->GPRINT(NORMAL_LEVEL, "Found account_groups option\n");
			    		for (u32 n=0;n<CnT->json_cfg["account_groups"].size();n++){
			    			//Obj->GPRINT(NORMAL_LEVEL, "%s %s\n",itr_grp.key().asCString(),CnT->json_cfg["account_groups"][n].asCString());
			    			if (itr_grp.key().asString()==CnT->json_cfg["account_groups"][n].asString()){
					    		GroupList[i]=new SecureAccessT(itr_grp.key().asString().c_str(),MAX_USERS);
					    		i++;
					    		Obj->GPRINT(NORMAL_LEVEL, "Add group %s\n",CnT->json_cfg["account_groups"][n].asCString());
			    			}
			    		}
			    	}
			    	else{
			    		GroupList[i]=new SecureAccessT(itr_grp.key().asString().c_str(),MAX_USERS);
			    		i++;
			    	}
			    }

			}
				//info=root[JSON_DATA_SYMB][JSON_INFO_SYMB].asString();
				//err=NO_ERROR;
		//}

		//printf("Parse success!!!! %s\n",group_json_str.c_str());
	}

	//exit(1);
	//Sm.LoadJSONInStorage(group_data);
#ifdef _SECURE_ENABLE
	epsec::secure sec;
	secureCheck(sec);
#else
	CnT->authorised=true;
#endif

	return NO_ERROR;
}

eErrorTp reInitAuth(GprintT * Obj,SecureAccessT** GroupList)
{

	for (u32 i=0;i<MAX_GROUP;i++) {
		//printf("GroupList[%d]=0x%08x\n",i,GroupList[i]);
		if (GroupList[i]!=NULL){
			delete GroupList[i];
			GroupList[i]=NULL;
		}
	}

	return InitAuth(Obj,GroupList);
}

eErrorTp auth_SyncAuth(GprintT * Obj,SecureAccessT** GroupList)
{
	sInterThrMsgHeader Mheader;
	//Mheader.MsgType=srvmSendToLH;
//	Mheader.SrvType=SrvType;
	string acc;

#if 0
	int i;
	for (i=0;i<MAX_GROUP;i++) {
		if (GroupList[i]==NULL)
			break;
	}

	if (i==0) {
		Obj->GPRINT(NORMAL_LEVEL, "Try init accounts\n");
		//Not init accounts
		InitAuth(Obj,GroupList);
		Obj->GPRINT(NORMAL_LEVEL, "Finish init accounts\n");
	}
#endif

	reInitAuth(Obj,GroupList);
	//exit(1);
	//string str=string_format("{'t':[%d,%d],'d':[]}",JSON_PACK_TYPE_AUTH_CLEAR,JSON_PACK_VERSION);
	//PrepareForJsonC(str);

	Obj->GPRINT(NORMAL_LEVEL, "Send auth clear command to web_ui\n");
	shared_fifo->SendDataFromCnodaToWEBSRV("",TO_WEB_SRV_TYPE_AUTH_CLEAR);
	//shared_fifo->SendSharedFifoMessage(srvmSendToLH,"wead",(u8*)str.c_str(),str.size());
	if (CnT->authorised==false){
		//shared_fifo->SendDataFromCnodaToWEBSRV("",TO_WEB_SRV_TYPE_AUTH_CLEAR);
		/*Json::Value needLicRoot;
		Json::Value forceLogoutRoot;
		needLicRoot["license"]["stat"]="needLicense";
		forceLogoutRoot["forceLogout"]["group"]="";
		shared_fifo->SendSystemFromCnodaToUI(FastWriteJSON(needLicRoot));
		shared_fifo->SendSystemFromCnodaToUI(FastWriteJSON(forceLogoutRoot));*/
		shared_fifo->activationMessageNeedLic();
		//return NO_ERROR;
	}
	u32 i;
	for (i=0;i<MAX_GROUP;i++) {

		if (GroupList[i]==NULL)
			break;
		else {
			Obj->GPRINT(NORMAL_LEVEL, "Select GroupList %d\n",i);
			acc=GroupList[i]->GetAccounts();
			//str=string_format("{'t':[%d,%d],'d':%s}",JSON_PACK_TYPE_AUTH,JSON_PACK_VERSION,acc.c_str());
			//PrepareForJsonC(str);
			//shared_fifo->SendSharedFifoMessage(srvmSendToLH,"wead",(u8*)str.c_str(),str.size());
			shared_fifo->SendDataFromCnodaToWEBSRV(acc,TO_WEB_SRV_TYPE_AUTH);
		}
	}

	Obj->GPRINT(NORMAL_LEVEL,"Sync auth success, total group %d\n",i);

	return NO_ERROR;
}

eErrorTp auth_ChangePassword(GprintT * Obj,SecureAccessT** GroupList,Json::Value jsonReq,string & json_resp)
{
	//{"t":[1,1],"d":{"type":"setpasswd","spass":"admin","slog":"admin","nlog":"trotill","npass":"trotill"}}
	printf("!!!Del my:system %s\n",StyledWriteJSON(jsonReq).c_str());
	string spass,slog,nlog,npass,ngroup,sgroup;
	bool admin_mode=false;
	eErrorTp err=ERROR;

#ifdef _SECURE_ENABLE
	epsec::secure sec;

	secureCheck(sec);
#else
	CnT->authorised=true;
#endif

	if (CnT->authorised==false){
		json_resp="secure_err";
		return ERROR;
	}

	spass=jsonReq["d"]["spass"].asString();
	slog=jsonReq["d"]["slog"].asString();
	nlog=jsonReq["d"]["nlog"].asString();
	npass=jsonReq["d"]["npass"].asString();
	ngroup=jsonReq["d"]["group"].asString();
	sgroup=jsonReq["d"]["sgroup"].asString();
	//JSON_GetFieldInData((char*)json_req.c_str(),"spass",spass);
	//JSON_GetFieldInData((char*)json_req.c_str(),"slog",slog);
	//JSON_GetFieldInData((char*)json_req.c_str(),"nlog",nlog);
	//JSON_GetFieldInData((char*)json_req.c_str(),"npass",npass);
	//JSON_GetFieldInData((char*)json_req.c_str(),"group",ngroup);

	SecureAccessT * SA_dest=NULL;
	SecureAccessT * SA_source=NULL;
	u32 sdst_prio,ssrc_prio=1000;
	//if (JSON_GetFieldInData((char*)json_req.c_str(),"sgroup",sgroup)==NO_ERROR){
	if (jsonReq["d"].isMember("sgroup")){
		admin_mode=true;
		//в этом режиме админ может менять пароль и логин пользователя с более низким
		//приоритетом, не зная пароля пользователя
		SA_source=FindOrCreateGroup(GroupList,sgroup);
		ssrc_prio=GetGroupPriority(sgroup);
		Obj->GPRINT(NORMAL_LEVEL,"Source group %s prio %d\n",sgroup.c_str(),ssrc_prio);
		if ((err=SA_source->Check(spass, slog))==ERROR)
		{
			json_resp="secure_err";
			mdelay(500);
			Obj->GPRINT(NORMAL_LEVEL,"Source auth fault\n");
			return err;
		}
	}

	sdst_prio=GetGroupPriority(ngroup);
	Obj->GPRINT(NORMAL_LEVEL,"Dest group %s prio %d\n",ngroup.c_str(),sdst_prio);
	if (ssrc_prio>sdst_prio)
	{
		json_resp="secure_err_prio";
		mdelay(500);
		Obj->GPRINT(NORMAL_LEVEL,"Priority error\n");
		err=ERROR;
		return err;
	}

	SA_dest=FindOrCreateGroup(GroupList,ngroup);
//	if (SA==NULL)
	//	SA=Group[i]=new SecureAccess(ngroup,MAX_USERS);

	if ((admin_mode==true)||(err=SA_dest->Check(spass, slog))==NO_ERROR)
	{
		Obj->GPRINT(NORMAL_LEVEL,"Auth checked\n");
		err=SA_dest->Change(npass,slog,nlog);

#ifdef _SECURE_ENABLE
		sleep(1);
		//printf("CnT->licKey %s",CnT->licKey.c_str());
		sec.signAuth(CnT->licKey);
#endif
		if (err==NO_ERROR) {
			json_resp="secure_changed";
			err=auth_SyncAuth(Obj,GroupList);
			sleep(1);
			Json::Value respMsg;
			respMsg["forceLogout"]["group"]=ngroup;
			shared_fifo->SendSystemFromCnodaToUI(StyledWriteJSON(respMsg));
			//printf("2 ngroup.c_str() %s\n",ngroup.c_str());
			Obj->GPRINT(NORMAL_LEVEL,"Auth changed and sync for %s\n",ngroup.c_str());
		}
		else{
			Obj->GPRINT(NORMAL_LEVEL,"Auth fault change\n");
		}
	}
	else
	{
		Obj->GPRINT(NORMAL_LEVEL,"Auth fault\n");
		err=ERROR;
	}

	if (err==ERROR)
	{
		Obj->GPRINT(NORMAL_LEVEL,"Auth error, send [secure_err]\n");
		json_resp="secure_err";
		mdelay(500);
	}

	return err;
}

eErrorTp auth_ChangePasswordV2(GprintT * Obj,SecureAccessT** GroupList,Json::Value jsonReq,string & json_resp)
{
	//{"t":[1,1],"d":{"type":"setpasswd","spass":"admin","slog":"admin","nlog":"trotill","npass":"trotill"}}

	string spass,sourcePasswd,slog,nlog,npass,ngroup,sgroup;


	sourcePasswd=jsonReq["d"]["sourcePasswd"].asString();

	slog=jsonReq["d"]["sourceLogin"].asString();
	nlog=jsonReq["d"]["newLogin"].asString();
	npass=jsonReq["d"]["newPasswd"].asString();
	ngroup=jsonReq["d"]["newGroup"].asString();
	sgroup=jsonReq["d"]["sourceGroup"].asString();
	spass=md5(string(sourcePasswd+slog));
	//JSON_GetFieldInData((char*)json_req.c_str(),"spass",spass);
	//JSON_GetFieldInData((char*)json_req.c_str(),"slog",slog);
	//JSON_GetFieldInData((char*)json_req.c_str(),"nlog",nlog);
	//JSON_GetFieldInData((char*)json_req.c_str(),"npass",npass);
	//JSON_GetFieldInData((char*)json_req.c_str(),"group",ngroup);

	printf("!!!Del my:system %s\n",StyledWriteJSON(jsonReq).c_str());

#ifdef _SECURE_ENABLE
	epsec::secure sec;

	secureCheck(sec);
#else
	CnT->authorised=true;
#endif

	bool admin_mode=false;
	eErrorTp err=ERROR;
	SecureAccessT * SA_dest=NULL;
	SecureAccessT * SA_source=NULL;
	u32 sdst_prio,ssrc_prio=1000;
	//if (JSON_GetFieldInData((char*)json_req.c_str(),"sgroup",sgroup)==NO_ERROR){
	if (jsonReq["d"].isMember("sourceGroup")){
		admin_mode=true;
		//в этом режиме админ может менять пароль и логин пользователя с более низким
		//приоритетом, не зная пароля пользователя
		SA_source=FindOrCreateGroup(GroupList,sgroup);
		ssrc_prio=GetGroupPriority(sgroup);
		Obj->GPRINT(NORMAL_LEVEL,"Source group %s prio %d\n",sgroup.c_str(),ssrc_prio);
		if ((err=SA_source->Check(spass, slog))==ERROR)
		{
			json_resp="secure_err";
			mdelay(500);
			Obj->GPRINT(NORMAL_LEVEL,"Source auth fault\n");
			return err;
		}
	}

	sdst_prio=GetGroupPriority(ngroup);
	Obj->GPRINT(NORMAL_LEVEL,"Dest group %s prio %d\n",ngroup.c_str(),sdst_prio);
	if (ssrc_prio>sdst_prio)
	{
		json_resp="secure_err_prio";
		mdelay(500);
		Obj->GPRINT(NORMAL_LEVEL,"Priority error\n");
		err=ERROR;
		return err;
	}

	SA_dest=FindOrCreateGroup(GroupList,ngroup);
//	if (SA==NULL)
	//	SA=Group[i]=new SecureAccess(ngroup,MAX_USERS);

	if ((admin_mode==true)||(err=SA_dest->Check(spass, slog))==NO_ERROR)
	{
		Obj->GPRINT(NORMAL_LEVEL,"Auth checked\n");
		err=SA_dest->Change(npass,slog,nlog);
#ifdef _SECURE_ENABLE
		sleep(1);
	//	printf("CnT->licKey %s",CnT->licKey.c_str());
		sec.signAuth(CnT->licKey);
#endif
		if (err==NO_ERROR) {
			json_resp="secure_changed";
			err=auth_SyncAuth(Obj,GroupList);
			sleep(2);
			Json::Value respMsg;
			respMsg["forceLogout"]["group"]=ngroup;
			shared_fifo->SendSystemFromCnodaToUI(StyledWriteJSON(respMsg));
			//printf("2 ngroup.c_str() %s\n",ngroup.c_str());
			Obj->GPRINT(NORMAL_LEVEL,"Auth changed and sync for %s\n",ngroup.c_str());
		}
		else{
			Obj->GPRINT(NORMAL_LEVEL,"Auth fault change\n");
		}
	}
	else
	{
		Obj->GPRINT(NORMAL_LEVEL,"Auth fault\n");
		err=ERROR;
	}

	if (err==ERROR)
	{
		Obj->GPRINT(NORMAL_LEVEL,"Auth error, send [secure_err]\n");
		json_resp="secure_err";
		mdelay(500);
	}

	return err;
}

