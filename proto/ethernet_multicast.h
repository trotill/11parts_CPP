/*
 * ethernet_multicast.h
 *
 *  Created on: 16 янв. 2019 г.
 *      Author: root
 */
#ifndef ETHERNET_MULTICAST_H_
#define ETHERNET_MULTICAST_H_

#include "engine/basic.h"
#include "engine/print.h"
#include "engine/memadmin.h"
#include "engine/ipc_socket.h"
#include "engine/lib/11p_string_net.h"
 #include <poll.h>

class Ethernet_multicastT:public GprintT{
	public:
	Ethernet_multicastT(eDebugTp debug_level,string GroupIP);
	string GroupIP;
	~Ethernet_multicastT();
	u16 srcPort=0;
	u16 dstPort=0;
	string SourceIP="";
	string SourceNet="";
	bool added_togroup=false;
	eErrorTp InitByNet(u16 srcPort,u16 dstPort,string net);
	eErrorTp InitByIP(u16 srcPort,u16 dstPort,char * SourceIP);
	eErrorTp ForceDisconnect();
	eErrorTp ForceReconnect();
	eErrorTp CheckConnect();
	eErrorTp Send( char * sbuf, u32 sbuf_size);
	//fd_set readset;
	//struct timeval RxTimeout;

	struct pollfd fds;
	u32 Recv(u32 timeout_ms);
	BufHandler RecvBuf;
	private:
	eErrorTp SockCreate(sSock & Sock, int af,int type,int protocol);
	eErrorTp SockClose(sSock & Sock);
	eErrorTp InitSock();
	sSock pSockSrc;
	struct ip_mreq mult;
};

#endif

