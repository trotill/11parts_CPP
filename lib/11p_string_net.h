/*
 * 11p_string_net.h
 *
 *  Created on: 16 янв. 2019 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_LIB_11P_STRING_NET_H_
#define SRC_ENGINE_LIB_11P_STRING_NET_H_

#include "engine/basic.h"
#include "engine/memadmin.h"
#include <ifaddrs.h>
#include <netdb.h>
#include <linux/if_link.h>

eErrorTp ConvStringIP_SinAddr(string sip,struct sockaddr_in & sa);
eErrorTp ConvSinAddr_StringIP(struct sockaddr_in & sa,string & sip);

class NetinfoT{
	public:
	NetinfoT(){
	}
	class __ifaddrs{
	public:
		string iface="";
		string ip="";
		string mask="";
		string broadcast="";
		string dstaddr="";
		unsigned int flags;
	}info;

	eErrorTp GetInfoByNetNameIPv4(string netname){
		return GetInfoByNetName(netname,AF_INET);
	}

	eErrorTp GetInfoByNetNameIPv6(string netname){
		return GetInfoByNetName(netname,AF_INET6);
	}

	eErrorTp GetInfoByIPv4(string ip){
		return GetInfoByIP(ip,AF_INET);
	}

	eErrorTp GetInfoByIPv6(string ip){
		return GetInfoByIP(ip,AF_INET6);
	}

	private:
	eErrorTp GetInfoByNetName(string netname,u32 family){
			struct ifaddrs *addrs=NULL,*tmp=NULL;
			//printf("GetInfoByNetName 1\n");
			if (getifaddrs(&addrs)!=0)
				return ERROR;
			if (addrs==NULL)
				return ERROR;
			//printf("GetInfoByNetName 2\n");
			int s;
			tmp = addrs;
			//printf("GetInfoByNetName addrs 0x%08x\n",*addrs);
			//printf("GetInfoByNetName addrs 0x%08x\n",*addrs);
			char host[NI_MAXHOST];
			void * tmpAddrPtr = NULL;

			while (tmp) {
				 if (tmp->ifa_addr == NULL){
					 tmp = tmp->ifa_next;
					 continue;
				 }

				 if (tmp->ifa_addr->sa_family == family)
				 {
					 //printf("netname %s tmp->ifa_name %s\n",netname.c_str(),tmp->ifa_name);
					 if (netname==tmp->ifa_name){
						 info.iface=tmp->ifa_name;
						 tmpAddrPtr = &((struct sockaddr_in *)tmp->ifa_addr)->sin_addr;
						 char addressBuffer[INET_ADDRSTRLEN];
						 inet_ntop(family, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
						 info.ip=addressBuffer;
						 tmpAddrPtr = &((struct sockaddr_in *)tmp->ifa_netmask)->sin_addr;
						 inet_ntop(family, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
						 info.mask=addressBuffer;
						 tmpAddrPtr = &((struct sockaddr_in *)tmp->ifa_ifu.ifu_broadaddr)->sin_addr;
						 inet_ntop(family, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
						 info.broadcast=addressBuffer;
						 tmpAddrPtr = &((struct sockaddr_in *)tmp->ifa_ifu.ifu_dstaddr)->sin_addr;
						 inet_ntop(family, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
						 info.dstaddr=addressBuffer;
						 info.flags=tmp->ifa_flags;
					 }
				 }
				 tmp = tmp->ifa_next;
			}

			freeifaddrs(addrs);
			// printf("exit GetInfoByNetName\n");
			return NO_ERROR;
		}

	eErrorTp GetInfoByIP(string ip,u32 family){
		struct ifaddrs *addrs,*tmp;
		getifaddrs(&addrs);
		int s;
		tmp = addrs;
		char host[NI_MAXHOST];
		void * tmpAddrPtr = NULL;
		while (tmp) {
			 if (tmp->ifa_addr == NULL)
			      continue;

			 if (tmp->ifa_addr->sa_family == family)
			 {
				 tmpAddrPtr = &((struct sockaddr_in *)tmp->ifa_addr)->sin_addr;
				 char addressBuffer[INET_ADDRSTRLEN];
				 inet_ntop(family, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

				 if (ip==addressBuffer){
					 info.ip=ip;
					 info.iface=tmp->ifa_name;
					 tmpAddrPtr = &((struct sockaddr_in *)tmp->ifa_netmask)->sin_addr;
					 inet_ntop(family, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
					 info.mask=addressBuffer;
					 tmpAddrPtr = &((struct sockaddr_in *)tmp->ifa_ifu.ifu_broadaddr)->sin_addr;
					 inet_ntop(family, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
					 info.broadcast=addressBuffer;
					 tmpAddrPtr = &((struct sockaddr_in *)tmp->ifa_ifu.ifu_dstaddr)->sin_addr;
					 inet_ntop(family, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
					 info.dstaddr=addressBuffer;
					 info.flags=tmp->ifa_flags;
				 }
			 }
			 tmp = tmp->ifa_next;
		}

		freeifaddrs(addrs);
		return NO_ERROR;
	}
};

int IpToStr (u32 in,char * buffer,u32 max_buflen);

#endif /* SRC_ENGINE_LIB_11P_STRING_NET_H_ */
