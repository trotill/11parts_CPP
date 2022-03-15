/*
 * 11p_string_net.cxx
 *
 *  Created on: 16 янв. 2019 г.
 *      Author: root
 */
#include "11p_string_net.h"





eErrorTp ConvStringIP_SinAddr(string sip,struct sockaddr_in & sa){

	inet_pton(AF_INET,sip.c_str(), &(sa.sin_addr));
	return NO_ERROR;
}

eErrorTp ConvSinAddr_StringIP(struct sockaddr_in & sa,string & sip){
	char buf[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(sa.sin_addr), buf, INET_ADDRSTRLEN);
	sip=buf;
	return NO_ERROR;
}

int IpToStr (u32 in,char * buffer,u32 max_buflen)
{
  unsigned char bytes[4];
  bytes[0]=in&0xff;
  bytes[1]=(in>>8)&0xff;
  bytes[2]=(in>>16)&0xff;
  bytes[3]=(in>>24)&0xff;
  snprintf (buffer, max_buflen , "%d.%d.%d.%d",
              bytes[0], bytes[1], bytes[2], bytes[3]);

  return NO_ERROR;
}
