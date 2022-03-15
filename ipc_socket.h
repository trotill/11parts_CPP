/*
 * ipc_socket.h
 *
 *  Created on: 15 дек. 2018 г.
 *      Author: root
 */

#ifndef SRVIOT_SRC_ENGINE_IPC_SOCKET_H_
#define SRVIOT_SRC_ENGINE_IPC_SOCKET_H_

#include "engine/basic.h"
#include "print.h"
#include "memadmin.h"

class IPC_SocketT:virtual public GprintT{
	public:
	IPC_SocketT(eDebugTp debug_level){
		InitBuf(&RawBuf);
		AllocBuf(&RawBuf,MAX_RCV_CLNT_BUF*2);
	}
	IPC_SocketT(){
			InitBuf(&RawBuf);
			AllocBuf(&RawBuf,MAX_RCV_CLNT_BUF*2);
		}
	~IPC_SocketT(){
		FreeBuf(&RawBuf);
	}
	sSock pSockSrc;
	BufHandler RawBuf;
	bool IsMcast=false;
	u32 ReadNetUDP();
	eErrorTp CreateNET_sock();
    eErrorTp InitLHBus(u16 srcPort,u16 dstPort,char * GroupIP);
    eErrorTp CloseNET_sock();
    eErrorTp SockSendToLH_Bus( char * sbuf, u32 sbuf_size);

};

eErrorTp SetBlockSockNotBlockMode(sSock & Sock, u32 mode);
eErrorTp SockRecv(sSock & Sock , char * sbuf, u32 sbuf_size, ssize_t & bytes_read);
eErrorTp SockCreate(sSock & Sock, int af,int type,int protocol);
eErrorTp SearchOneFreePorts(string ip,u16& port,int af,int type,int protocol);

#endif /* SRVIOT_SRC_ENGINE_IPC_SOCKET_H_ */
