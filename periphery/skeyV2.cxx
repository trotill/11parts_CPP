/*
 * skeyV2.cxx
 *
 *  Created on: 17 февр. 2021 г.
 *      Author: root
 */
#include "skeyV2.h"

#ifdef _SECURE_ENABLE
#ifndef CNODAKG
	#include <engine/global.h>

		eErrorTp epsec::secure::findMMC(){//0-7

			///sys/block/mmcblk2/device/cid
			//if (secCombinationHex=="1"){
				check("/sys/block/mmcblk0/device/cid",0);
				check("/sys/block/mmcblk1/device/cid",1);
				check("/sys/block/mmcblk2/device/cid",2);
				check("/sys/block/mmcblk3/device/cid",3);
				check("/sys/block/mmcblk4/device/cid",4);
				check("/sys/block/mmcblk5/device/cid",5);
			//}
			return NO_ERROR;
		}
		eErrorTp epsec::secure::findDMI(){//8-15
			//if (secCombination=="1"){
				check("/sys/class/dmi/id/product_uuid",8);
				check("/sys/class/dmi/id/product_serial",9);
				check("/sys/class/dmi/id/uevent",10);
			//}
			return NO_ERROR;
		}
		eErrorTp epsec::secure::findEthernet(){//16-23
		    //if (secCombination=="1"){
		        check("/sys/class/net/eth0/address",16);
		        check("/sys/class/net/eth1/address",17);
		        check("/sys/class/net/eth2/address",18);
		        check("/sys/class/net/eth3/address",19);
		    //}
		    return NO_ERROR;
		}
		eErrorTp epsec::secure::findSCSI(){//24-47
			//if (secCombination=="1"){
			    checkBin("/sys/block/sda/device/vpd_pg83",24);
			    checkBin("/sys/block/sda/device/vpd_pg80",25);
			    checkBin("/sys/block/sdb/device/vpd_pg83",26);
			    checkBin("/sys/block/sdb/device/vpd_pg80",27);
			    checkBin("/sys/block/sdc/device/vpd_pg83",28);
			    checkBin("/sys/block/sdc/device/vpd_pg80",29);
			    checkBin("/sys/block/sdd/device/vpd_pg83",30);
			    checkBin("/sys/block/sdd/device/vpd_pg80",31);

				check("/sys/class/scsi_disk/0\:0\:0\:0/device/wwid",32);
				check("/sys/class/scsi_disk/1\:0\:0\:0/device/wwid",33);
				check("/sys/class/scsi_disk/2\:0\:0\:0/device/wwid",34);
				check("/sys/class/scsi_disk/3\:0\:0\:0/device/wwid",35);
				checkUDEVSDxx();//36-37

			//}
			return NO_ERROR;
		}
		eErrorTp epsec::secure::findDRM(){
			//if (existsSync("/sys/class/drm/card0/device/uevent")==NO_ERROR){
			//	string data="DRM";
			//	ReadStringFile("/sys/class/drm/card0/device/uevent",data);
			//	deviceUID=deviceUID+data;
			//}
			//if (secCombination=="1"){//не использовать, меняются местами от загрузки к зарузке card0/card1/card2
				//check("/sys/class/drm/card0/device/uevent");
				//check("/sys/class/drm/card1/device/uevent");
				//check("/sys/class/drm/card2/device/uevent");
			//}
			return NO_ERROR;
		}
		eErrorTp epsec::secure::findCPU(){
			//if (secCombination=="1"){//не использовать cpuinfo, bogomips меняется от загрузки к загрузке
				//check("/proc/cpuinfo");
			//}
			return NO_ERROR;
		}
		string epsec::secure::getDeviceUID(){

			findMMC();
			findDMI();
			findSCSI();
			findDRM();
			findCPU();
			findEthernet();
			deviceUID=deviceUID+string(_SECURE_UID);
			deviceUID=md5(deviceUID);
#ifdef SEC_DEBUG
			printf("Device UID %s\n",deviceUID.c_str());
#endif
			return deviceUID;
		}
	eErrorTp epsec::secure::checkKey(string key){
			string gl=good+luck;
			string decData=sf("%s google.com %s %s %s %s",deviceUID.c_str(),_SECURE_SNP,gl.c_str(),_SECURE_UID,_SECURE_COMBINATION);

			std::vector<unsigned char> hash(picosha2::k_digest_size);
		   picosha2::hash256(decData.begin(), decData.end(), hash.begin(), hash.end());

			std::string decData_SHA256 = picosha2::bytes_to_hex_string(hash.begin(), hash.end())+md5(decData);

#ifdef SEC_DEBUG
			printf("checkKey decData [%s]\n",decData_SHA256.c_str());
			printf("checkKey key [%s]\n",key.c_str());
#endif
			string dec=base64_decode(key);
			AES aes(128);
#ifdef SEC_DEBUG
			printhex((u8*)dec.c_str(),dec.size(),16);
#endif

			string secureSNP_UID=string((char*)_SECURE_SNP)+deviceUID;
			u8 kr[16]={0};

			u8 ksz=secureSNP_UID.size();
			if (ksz>16)
				ksz=16;

#ifdef SEC_DEBUG
			printf("strlen(_SECURE_SNP) %d\n",ksz);
#endif
			memcpy(kr,secureSNP_UID.c_str(),ksz);
			kr[15]^=0xaa;
			kr[11]^=0xcc;
			kr[1]^=0x21;
#ifdef SEC_DEBUG
			printhex(kr,ksz,16);
#endif
			unsigned char *out= (char*)aes.DecryptECB((u8*)dec.c_str(),(u32) dec.size(),(u8*)kr);
			out[dec.size()]=0;
			string outStr=(char*)out;
#ifdef SEC_DEBUG
			printf("checkKey decodedData [%s] needData [%s] [%d/%d]\n",outStr.c_str(),decData_SHA256.c_str(),outStr.size(),decData_SHA256.size());
#endif
			//string outStr=(char*)out;
			if (outStr==decData_SHA256){
#ifdef SEC_DEBUG
				printf("checked ok\n");
#endif
				return NO_ERROR;
			}
#ifdef SEC_DEBUG
			printf("check error\n");
#endif
			delete[] out;
			return ERROR;
		}

	eErrorTp epsec::secure::signAuth(string & licKey){
		string sysDir=CnT->SETTING_STORAGE_MOUNTP[0];
		::filesys fs;
		vector<searched_file_list> fileList;
		fs.SearchFilesInDir((char*)sysDir.c_str(),fileList,"account.");
		for (u32 i=0;i<fileList.size();i++){
			if ((fileList[i].fname.find(".crc")==-1)&&(fileList[i].fname.find(".sign")==-1)){
				printf("fname %s\n",fileList[i].fname.c_str());
				string data;
				::ReadStringFile((char*)fileList[i].fname.c_str(),data);
				string signPre=data+licKey+deviceUID+secureUid+_SECURE_COMBINATION;
				u32 snum=secureUid.size();
				if (snum<signPre.size())
					signPre[snum]=signPre[snum]^0xfa;

				signPre[3]=signPre[3]&0xaa;

				string sign=md5(signPre);
				printf("sign [%s] create %s\n",sign.c_str(),(char*)string(fileList[i].fname+".sign").c_str());
				::WriteStringFile((char*)string(fileList[i].fname+".sign").c_str(),sign);

			}
		}

		return NO_ERROR;
	}
	eErrorTp epsec::secure::checkAuthSign(string & licKey){
		string sysDir=CnT->SETTING_STORAGE_MOUNTP[0];
		::filesys fs;
		vector<searched_file_list> fileList;
		//vector<string> authFList;
		//vector<string> signFList;
		//zr();
		fs.SearchFilesInDir((char*)sysDir.c_str(),fileList,"account.");
		u8 succ=0;
		for (u32 i=0;i<fileList.size();i++){
			//zr();
			string signData;
			string authData;
			if ((fileList[i].fname.find(".crc")==-1)){
				//zr();
				//if (fileList[i].fname.find(".sign")==-1)
				//{
				//	::ReadStringFile((char*)fileList[i].fname.c_str(),signData);
				//}
				if ((fileList[i].fname.find(".set")!=-1)&&(fileList[i].fname.find(".sign")==-1))
				{
					//zr();
					string signFname=fileList[i].fname+".sign";
					//printf("signFname %s\n",signFname.c_str());
					if (::existsSync(signFname)==NO_ERROR){
						//zr();
						::ReadStringFile((char*)fileList[i].fname.c_str(),authData);
						::ReadStringFile((char*)signFname.c_str(),signData);
						string signPre=authData+licKey+deviceUID+secureUid+_SECURE_COMBINATION;
						u32 snum=secureUid.size();
						if (snum<signPre.size())
							signPre[snum]=signPre[snum]^0xfa;

						signPre[3]=signPre[3]&0xaa;

						string sign=md5(signPre);
						//printf("orig %s new %s\n",signData.c_str(),sign.c_str());
						if (sign!=signData){
							return ERROR;
						}
						else{
							succ++;
						}
					}
					else{
						return ERROR;
					}

				}
			}
		}

		//if (fileList.size()==0)
			//return ERROR;
		if (succ!=0)
			return NO_ERROR;

		return ERROR;
	}
	eErrorTp epsec::secure::checkUDEVSDxx(){

		    if (((36<<1)&secCombinationHex)){
		        string data=BashResult("/bin/udevadm info --name=/dev/sda|grep SERIAL");
		        deviceUID=deviceUID+data;
                #ifdef SEC_DEBUG
		            printf("Sequence checkUDEVSDxx read /dev/sda:[%s]\n",data.c_str());
                #endif
		    }
		    if (((37<<1)&secCombinationHex))
		    {
		        string data=BashResult("/bin/udevadm info --name=/dev/sdb|grep SERIAL");
		        deviceUID=deviceUID+data;
                #ifdef SEC_DEBUG
		            printf("Sequence checkUDEVSDxx read /dev/sdb:[%s]\n",data.c_str());
                #endif
		    }
		    return NO_ERROR;
		}
	eErrorTp epsec::secure::check(char * fname,u8 comBit){
		//printf("check %s\n",fname);
		if (!((comBit<<1)&secCombinationHex)){
		    return ERROR;
		}
		if (::existsSync(fname)==NO_ERROR){
			//zr();
			string data="DRM";
			::ReadStringFile(fname,data);
			//printf("data %s\n",data.c_str());
			#ifdef SEC_DEBUG
			printf("Sequence check read %s:[%s]\n",fname,data.c_str());
            #endif
			deviceUID=deviceUID+data;
			//printf("deviceUID %s\n",deviceUID.c_str());
		}
		return NO_ERROR;
	}

	eErrorTp epsec::secure::checkBin(char * fname,u8 comBit){
		    //printf("check %s\n",fname);
		    if (!((comBit<<1)&secCombinationHex)){
		        return ERROR;
		    }
		    if (::existsSync(fname)==NO_ERROR){
		        //zr();
		        string data="DRM";
		        buffer buf;
		        u32 len=::readDevFile(fname,buf,4096);
		        data= base64_encode(buf.p(),len);
		        #ifdef SEC_DEBUG
		            printf("Sequence checkBin read %s:base64[%s]\n",fname,data.c_str());
                #endif
		        //printf("data %s\n",data.c_str());
		        deviceUID=deviceUID+md5(data);
		        //printf("deviceUID %s\n",deviceUID.c_str());
		    }
		    return NO_ERROR;
		}

	string epsec::secure::decryptLicenseData(string & cryptLicData){
		u8 kr[16]={0};
		string dUID=deviceUID;
		u8 ksz=dUID.size();
		if (ksz>16)
		ksz=16;
		dUID[2]=~dUID[2];
		dUID[9]=~dUID[9];
		memcpy(kr,dUID.c_str(),ksz);

		AES aes(128);

		//printf("deviceUID %s cryptLicData [%s]\n",dUID.c_str(),cryptLicData.c_str());
		string dec=base64_decode(cryptLicData);
		unsigned char *out= (char*)aes.DecryptECB((u8*)dec.c_str(),(u32) dec.size(),(u8*)kr);
		//printf("DecryptECB [%s]\n",out);
		//exit(1);
		out[dec.size()]=0;
		string outStr=(char*)out;
		return outStr;
	}

	string epsec::secure::encryptLicenseData(string & licData){
		u8 kr[16]={0};
		string dUID=deviceUID;
		u8 ksz=dUID.size();
		if (ksz>16)
			ksz=16;
		dUID[2]=~dUID[2];
		dUID[9]=~dUID[9];
		memcpy(kr,dUID.c_str(),ksz);
		//printf("Encript [%s]\n",licData.c_str());
		AES aes(128);
		u32 outLen=0;
		unsigned char *out = aes.EncryptECB((u8*)licData.c_str(), (u32)licData.size(), kr, outLen);
		//out[outLen]=0;
		string outStr=base64_encode((u8*)out,outLen);
		//printf("Encript result [%s]\n",outStr.c_str());
		return outStr;
	}

	eErrorTp epsec::secure::getLicKey(string & key){
		string sysDir=CnT->SETTING_STORAGE_MOUNTP[0];
		string licFile=sysDir+licFileName;
		return ::ReadStringFile((char*)licFile.c_str(),key);
	}
	//eErrorTp epsec::secure::setLicKey(string & key){
	//	string sysDir=CnT->SETTING_STORAGE_MOUNTP[0];
//	//	string licFile=sysDir+licFileName;
	//	return ::WriteStringFile((char*)licFile.c_str(),key);
	//}
#else
		string epsec::secure::genKey(string devUID,string secureSNP){
			u8 kr[16]={0};
			string secureSNP_UID=secureSNP+devUID;
			u8 ksz=secureSNP_UID.size();
			if (ksz>16)
				ksz=16;

			memcpy(kr,secureSNP_UID.c_str(),ksz);
			kr[15]^=0xaa;
			kr[11]^=0xcc;
			kr[1]^=0x21;
			AES aes(128);
			u32 len=0;
			string gl=good+luck;
			char partUID[6]={0};
			devUID.copy(partUID,5,0);
			devUID=sf("%s google.com %s %s %s %s",devUID.c_str(),secureSNP.c_str(),gl.c_str(),_SECURE_UID,_SECURE_COMBINATION);


			std::vector<unsigned char> hash(picosha2::k_digest_size);
			picosha2::hash256(devUID.begin(), devUID.end(), hash.begin(), hash.end());

			std::string devUID_SHA256 = picosha2::bytes_to_hex_string(hash.begin(), hash.end())+md5(devUID);
			//printf("SHA256 %s\n",hex_str.c_str());

#ifdef SEC_DEBUG
			printf("genKey deviceUID %s\n",deviceUID.c_str());
#endif
			unsigned char *out = aes.EncryptECB((u8*)devUID_SHA256.c_str(), (u32)devUID_SHA256.size(), kr, len);
			out[len]=0;
#ifdef SEC_DEBUG
			printhex(out,len,16);
#endif

			string enc=base64_encode((const u8*)out, len);
			char partEnc[6]={0};
			enc.copy(partEnc,5,0);
			logGenLic(partUID,partEnc);
			delete[] out;
#ifdef SEC_DEBUG
			printf("genKey enc %s\n",enc.c_str());
#endif

			return enc;
		}

#endif
#endif




