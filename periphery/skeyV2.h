#ifndef SRVIOT_SRC_ENGINE_PERIPHERY_SKEY_V2_H_
#define SRVIOT_SRC_ENGINE_PERIPHERY_SKEY_V2_H_


#include <engine/types.h>

//#ifndef CNODAKG

//#endif

#include <engine/algo/MD5.h>
#include <engine/algo/sha256.h>
#include <engine/algo/aes.h>
#include <engine/algo/base64.h>
#include <engine/lib/11p_files.h>
#include <engine/lib/11p_string.h>


#ifdef _SECURE_ENABLE
std::string sf(const std::string fmt, ...);
void printhex(u8 * buf,u32 len,u16 loop);

//#define SEC_DEBUG 1


namespace epsec{

	class secure{
		public:
		secure(){
		    secCombinationHex= HexToU64(_SECURE_COMBINATION);
		}
		eErrorTp signAuth(string & licKey);
		eErrorTp checkAuthSign(string & licKey);
		eErrorTp getLicKey(string & key);
		string encryptLicenseData(string & licData);
		string decryptLicenseData(string & cryptLicData);
		eErrorTp setLicKey(string & key);
		eErrorTp checkKey(string key);
		eErrorTp checkUDEVSDxx();
		eErrorTp checkBin(char * fname,u8 comBit);
#ifdef CNODAKG
		string genKey(string devUID,string secureSNP);
#endif
		void setDeviceUID(string devUid){
			deviceUID=devUid;
		}
		string getDeviceUID();

		string writeJSON(Json::Value & val){
			 Json::StreamWriterBuilder builder;
			 builder["precision"] = 5;
			 builder["commentStyle"] = "None";
			 builder["indentation"] = "";
			 return Json::writeString(builder, val);
		}

		eErrorTp jsonParseFile(Json::Value & json,string filename)
		{
			Json::Reader rd;
			string data;
			if (existsSync(filename)==NO_ERROR){
				ReadStringFile((char*)filename.c_str(),data);
				if (rd.parse(data.c_str(),json)){
					return NO_ERROR;
				}
				else
					return ERROR;
			}
			return ERROR;
		}

		eErrorTp jsonParseString(Json::Value & json,char * data)
		{
			Json::Reader rd;

			if (rd.parse(data,json)){
				return NO_ERROR;
			}
			else
				return ERROR;

			return ERROR;
		}
		eErrorTp logPull(){
			if (!useLog)
				return ERROR;

			//if (existsSync(logFname)==NO_ERROR){
				//string logData=ReadStringFile(logFname);
				//logRoot=parseJSON(logData);
			string logData;
			string signData;
			eErrorTp err=ERROR;
			if (ReadStringFile((char*)logFname.c_str(),logData)==NO_ERROR)//(jsonParseFile(logRoot,logFname)==NO_ERROR){
			{
				string signFile=logFname+".sign";
				if (ReadStringFile((char*)signFile.c_str(),signData)==NO_ERROR){
				    string md5s=md5(logData+secureUid+_SECURE_COMBINATION);
					string enc=base64_encode((const u8*)md5s.c_str(), md5s.size());
					if (signData==enc){
						err=jsonParseString(logRoot,(char*)logData.c_str());
					}
				}
			}


			if(err==ERROR){
				logRoot["d"]["count"]=0;
				logRoot["d"]["genHistory"]=Json::Value(Json::arrayValue);
			}
			//}
			return err;
		}
		eErrorTp logPush(){
			if (!useLog)
				return ERROR;
			//if (existsSync(logFname)==NO_ERROR){
			string logData=writeJSON(logRoot);
			if (WriteStringFile(logFname.c_str(),logData)==NO_ERROR){
				string signFile=logFname+".sign";
				string md5s=md5(logData+secureUid+_SECURE_COMBINATION);
				string enc=base64_encode((const u8*)md5s.c_str(), md5s.size());
				return WriteStringFile(signFile.c_str(),enc);
			}
			//}
			return ERROR;
		}
		string logGet(){
			return writeJSON(logRoot);
		}
		private:
		//eErrorTp check(char * fname);
		Json::Value logRoot;
		string logFname="/www/pages/sys/skeyLog.set";
		bool useLog=true;

		eErrorTp logGenLic(string uid,string licPart){
			if (!useLog)
				return ERROR;
			u32 count=logRoot["d"]["count"].asUInt();
			count++;
			logRoot["d"]["count"]=count;
			u32 hisSize=logRoot["d"]["genHistory"].size();
			logRoot["d"]["genHistory"][hisSize]["ts"]=TIME((u32*)NULL);
			logRoot["d"]["genHistory"][hisSize]["uid"]=uid+"...";
			logRoot["d"]["genHistory"][hisSize]["lic"]=licPart+"...";
			logRoot["d"]["genHistory"][hisSize]["num"]=count;

			return NO_ERROR;
		}
		eErrorTp check(char * fname,u8 comBit);
		eErrorTp findMMC();
		eErrorTp findDMI();
		eErrorTp findSCSI();
		eErrorTp findEthernet();
		eErrorTp findDRM();
		eErrorTp findCPU();
		 string deviceUID;
		 string secureUid=_SECURE_UID;
		 string good="<<Good ";
		 string licFileName="/license.set";
		 string luck="luck hacker!!!!>>";
		 //string secCombination=_SECURE_COMBINATION;
		 u64 secCombinationHex;
	};
}

	eErrorTp testSkey();
#endif
#endif
