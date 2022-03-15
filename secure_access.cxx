/*
 * secure_access.cxx
 *
 *  Created on: 15 янв. 2019 г.
 *      Author: root
 */

#include "auth.h"

SecureAccessT::SecureAccessT(string grp,u32 max)
	 {
		 group=grp;
		 max_account=max;
		 sAlist=new sAuthAccess[max];
		 SourceStr="Secure";
		 ObjPref="Shared";
		 debug_level=NORMAL_LEVEL;

		 //ToDo
		 //Delete admin, only debug
		 string CodedPassword;
		 string CodedUser;
		 if (LoadAdminAccount(sAlist)==ERROR)
		 {
			// sAlist[0].UserName="admin";
			// sAlist[0].Password="f6fdffe48c908deb0f4c3bd36c032e72";
			// printf("Default admin/admin for group %s\n",group.c_str());
			// EncodeSecureAlgo(sAlist[0].UserName,sAlist[0].Password,sAlist[0].CipherUser,sAlist[0].CipherPassword);
			 printf("Disabled Default admin/admin for group %s, set [account.%s.set] file\n",group.c_str(),group.c_str());
		 }else
		 {
			 printf("fill access group %s success\n",group.c_str());
		 }


		 for (u32 i=0;i<max;i++)
			 if (sAlist[i].CipherUser!="")
				 GPRINT(NORMAL_LEVEL,"Load account for user %s group %s\n",sAlist[i].CipherUser.c_str(),group.c_str());

	 }
SecureAccessT::~SecureAccessT()
	 {
		//printf("SecureAccessT::~SecureAccessT()\n");
		//for (u32 n=0;n<max_account;n++)
		 delete [] sAlist;
		// zr();
	 }

eErrorTp SecureAccessT::Change(string & password,string & oldlogin,string & newlogin)
	 {
		 for (u32 i=0;i<max_account;i++)
		 {
			 if ((sAlist[i].UserName=="")||(sAlist[i].UserName==oldlogin))
			 {
				// printf("user %s passwd %s\n",sAlist[i].UserName.c_str(),sAlist[i].CipherPassword.c_str());
				 sAlist[i].UserName=newlogin;
				 sAlist[i].Password=password;
				 EncodeSecureAlgo(sAlist[i].UserName,sAlist[i].Password,sAlist[i].CipherUser,sAlist[i].CipherPassword);
				 ChangeAdminAccount(oldlogin,sAlist[i].CipherUser,sAlist[i].CipherPassword);
				 return NO_ERROR;
			 }
		 }

		 return ERROR;
	 }

eErrorTp SecureAccessT::Add(string & password,string & oldlogin,string & newlogin)
	 {
		 for (u32 i=0;i<max_account;i++)
		 {
			 if ((sAlist[i].UserName=="")||(sAlist[i].UserName==oldlogin))
			 {
				// printf("user %s passwd %s\n",sAlist[i].UserName.c_str(),sAlist[i].CipherPassword.c_str());
				 sAlist[i].UserName=newlogin;
				 sAlist[i].Password=password;
				 EncodeSecureAlgo(sAlist[i].UserName,sAlist[i].Password,sAlist[i].CipherUser,sAlist[i].CipherPassword);
				 SaveAdminAccount(sAlist[i].CipherUser,sAlist[i].CipherPassword);
				 return NO_ERROR;
			 }
		 }

		 return ERROR;
	 }

eErrorTp SecureAccessT::Check(string & password, string & login)
	 {
		 for (u32 i=0;i<max_account;i++)
		 {
			// printf("user %s CipherUser %s\n",sAlist[i].UserName.c_str(),sAlist[i].CipherUser.c_str());
			 printf("alogin %s apass %s\n",sAlist[i].UserName.c_str(),login.c_str());
			 if ((sAlist[i].UserName==login)||(sAlist[i].CipherUser==login))
			 {
				 printf("apass %s cpass %s newpass %s\n",sAlist[i].CipherPassword.c_str(),sAlist[i].Password.c_str(),password.c_str());
				if ((sAlist[i].CipherPassword==password)||(sAlist[i].Password==password))
					return NO_ERROR;
			 }
		 }
		 return ERROR;
	 }

string SecureAccessT::GetAccounts(void)
	 {
		 stringstream ss;
		 ss << '{';
		 for (u32 i=0;i<max_account;i++)
		 {
			 	 if (sAlist[i].UserName!="")
			 	 {
			 		 if (ss.str().size()>2) ss<<',';
			 		 	 ss<<"\""<<sAlist[i].UserName<<"\":{\"login\":\""<<sAlist[i].UserName<<"\",\"passwd\":\""<<sAlist[i].Password<<"\",\"group\":\""<<group<<"\"}";
			 	 }
		 }
		 ss<<'}';
		 printf("Get accounts %s\n",ss.str().c_str());
		 return ss.str();
	 }

eErrorTp SecureAccessT::ChangeAdminAccount(string & oldusername,string & username,string & password)
	 {
		 Json::Value root;
		 string JSONstr;
		 Json::Reader reader;
		 //Json::FastWriter writer;
		 		//string json;
		 string acc_name=string_format("%s.%s",ACCOUNT_FILE_NAME,group.c_str());
		 string str;
		 if ((Sm.LoadJSONInStorage(JSONstr,(char*)acc_name.c_str())==ERROR)||(reader.parse( JSONstr.c_str(), root )==false)){
			 //str=string_format("{'%s':{'user':'%s','pass':'%s','group':'%s'}}", username.c_str(), username.c_str() ,password.c_str() ,group.c_str() );
			 //PrepareForJsonC(str);
			 Json::Value newAcc;//если есть ошибка, создаем новый файл аккаунта
			 newAcc[username]["user"]=username;
			 newAcc[username]["pass"]=password;
			 newAcc[username]["group"]=group;
			 str=FastWriteJSON(newAcc);
		 }
		 else{
			 if (root.isMember(oldusername.c_str()))
				root.removeMember(oldusername.c_str());

			 if (oneUserMode){
				 root.clear();
			 }
			 root[username.c_str()]["pass"]=password;
			 root[username.c_str()]["user"]=username;
			 root[username.c_str()]["group"]=group;
			 str=FastWriteJSON(root);
		 }
		 return Sm.SaveSetting(str,acc_name);
	 }

eErrorTp SecureAccessT::SaveAdminAccount(string & username,string & password)
	 {
		 Json::Value root;
		 string JSONstr;
		 Json::Reader reader;
		 //Json::FastWriter writer;
		 		//string json;
		 string acc_name=string_format("%s.%s",ACCOUNT_FILE_NAME,group.c_str());
		 string str;
		 if ((Sm.LoadJSONInStorage(JSONstr,(char*)acc_name.c_str())==ERROR)||(reader.parse( JSONstr.c_str(), root )==false)){
			Json::Value newAcc;//если есть ошибка, создаем новый файл аккаунта
			newAcc[username]["user"]=username;
			newAcc[username]["pass"]=password;
			newAcc[username]["group"]=group;
			str=FastWriteJSON(newAcc);
			 // str=string_format("{'%s':{'user':'%s','pass':'%s','group':'%s'}}", username.c_str(), username.c_str() ,password.c_str() ,group.c_str() );
			// PrepareForJsonC(str);
		 }
		 else{//если ошибки нет, добавляем в файл аккаунта
			 if (oneUserMode){
				 root.clear();
			 }
			 root[username.c_str()]["pass"]=password;
			 root[username.c_str()]["user"]=username;
			 root[username.c_str()]["group"]=group;
			 str=FastWriteJSON(root);
		 }
		 return Sm.SaveSetting(str,acc_name);
	 }

eErrorTp SecureAccessT::LoadAdminAccount(sAuthAccess * acc)
	 {
		 	string JSONstr;
		 	Json::Value root;
		 	Json::Reader reader;
		 	string acc_name=string_format("%s.%s",ACCOUNT_FILE_NAME,group.c_str());
		 	if (Sm.LoadJSONInStorage(JSONstr,(char*)acc_name.c_str())==ERROR)
		 		 return ERROR;

		 	u32 i=0;
		 	GPRINT(NORMAL_LEVEL, "Parse (%s)\n", JSONstr.c_str());
		 	if (reader.parse( JSONstr.c_str(), root )==true) {
		 			for( Json::Value::const_iterator itr = root.begin() ; itr != root.end() ; itr++ ) {
		 					 GPRINT(NORMAL_LEVEL, "Found group (%s)\n", itr.key().asString().c_str());
		 					 if (root[itr.key().asString().c_str()].isObject()&&(root[itr.key().asString().c_str()]["pass"].isNull()==false))
		 					 {
		 						 acc[i].CipherUser=itr.key().asString();
		 						 acc[i].CipherPassword=root[itr.key().asString().c_str()]["pass"].asString();
		 						 DecodeSecureAlgo(acc[i].CipherUser,acc[i].CipherPassword,sAlist[i].UserName,sAlist[i].Password);
		 						 i++;
		 					 }
		 					 //Group[i]=new SecureAccess(itr.key().asString().c_str(),MAX_USERS)
		 					 //i++;
		 				 }
		 	}
		 	//if (sGSN.root["node"].isArray())
		 	// printf("Load %s\n",JSONstr.c_str());
		 	// if (JSON_GetField((char*)JSONstr.c_str(),"user",username)==ERROR)
		 	//	 return ERROR;

		 	 //if (JSON_GetField((char*)JSONstr.c_str(),"pass",password)==ERROR)
		 	//	 return ERROR;

		 	 //printf("GOT %s/%s\n",username.c_str(),password.c_str());
	 		 return NO_ERROR;
	 }

eErrorTp SecureAccessT::EncodeSecureAlgo(string & username,string & password,string & codeduser,string & codedpasswd)
	 {
		 codedpasswd=password;
		 codeduser=username;
	 	return NO_ERROR;
	 }

eErrorTp SecureAccessT::DecodeSecureAlgo(string & codeduser,string & codedpasswd,string & username,string & password)
		 {
			 password=codedpasswd;
			 username=codeduser;
		 	return NO_ERROR;
		 }
