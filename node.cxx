/*
 * node.cxx
 *
 *  Created on: 18 дек. 2018 г.
 *      Author: root
 */

#include "node.h"

eErrorTp NodeT::LoadAndApplySetting(string setting_name)
{
	string JSONstr;
	if (Sm.LoadJSONInStorage(JSONstr,(char*)setting_name.c_str())==NO_ERROR)
		return ApplySetting(JSONstr);
	else
		GPRINT(NORMAL_LEVEL,"Critical error, not load %s config for device\n",setting_name.c_str());
	return ERROR;

}

eErrorTp NodeT::ApplySetting(string json)
{
	string out;
	PrepareCMDForNode(JS_APPLY_SETTINGS,json,out);
	printf("Send %s\n",out.c_str());
	return SockSendToLH_Bus((char*)out.c_str(), out.size());

}

eErrorTp NodeT::ReqNetInfo(string net,string reqparams)
{
	stringstream ss;
	ss << "{\"o\":\"" <<net<<"\",\"p\":"<<"["<<reqparams<<"]}";
	string out;
	PrepareCMDForNode(JS_REQ_INFO,ss.str(),out);

	//printf("Info %s\n",out.c_str());
	SockSendToLH_Bus((char*)out.c_str(), out.size());

	return NO_ERROR;
}

eErrorTp NodeT::DeInit(string json)
{
	string out;
	PrepareCMDForNode(JS_INIT,json,out);
	//printf("Send %s\n",out.c_str());
	return SockSendToLH_Bus((char*)out.c_str(), out.size());

}

eErrorTp NodeT::ConfigNodeDef()
{

	LoadAndApplySetting("settings.websrv");
	return NO_ERROR;
}

eErrorTp NodeT::Configure(void)
{
	string JSONstr;
	string RouterData;
	if (Sm.LoadSetting(JSONstr,"settings.router")==NO_ERROR)
	{
		printf("LoadJSONInStorage %s\n",JSONstr.c_str());
		ConfigNodeDef();
		ConfigNode((char*)JSONstr.c_str());
		printf("Get router data %s\n",RouterData.c_str());
		return NO_ERROR;
	}
	else
		GPRINT(NORMAL_LEVEL,"Critical error, not found router conf\n");

	return ERROR;
}
