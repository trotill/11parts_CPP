/*
 * ethernet_multicast.cxx
 *
 *  Created on: 15 мар. 2019 г.
 *      Author: root
 */

#include "engine/proto/ethernet_multicast.h"

Ethernet_multicastT::Ethernet_multicastT(eDebugTp debug_level,string GroupIP){
		InitBuf(&RecvBuf);
		AllocBuf(&RecvBuf,MAX_RCV_CLNT_BUF*2);
		SourceStr="Ethernet";
		ObjPref="multicast";
		this->GroupIP=GroupIP;
	}

Ethernet_multicastT::~Ethernet_multicastT(){
		SockClose(pSockSrc);
		FreeBuf(&RecvBuf);
	}

	eErrorTp Ethernet_multicastT::InitByNet(u16 srcPort,u16 dstPort,string net){
		string ipaddr;
		NetinfoT ni;
		//string ip,string iface,string mask

		if (ni.GetInfoByNetNameIPv4(net)==ERROR){
			GPRINT(NORMAL_LEVEL,"Error find IP addr, connect not init\n");
			return ERROR;
		}
		GPRINT(NORMAL_LEVEL,"Found iface[%s] with ip[%s]\n",(char*)net.c_str(),(char*)ni.info.ip.c_str());
		SourceNet=net;
		SourceIP=ni.info.ip;
		this->srcPort=srcPort;
		this->dstPort=dstPort;
		if (SourceIP==""){
			GPRINT(NORMAL_LEVEL,"ip addr on %s is empty, break InitByIp\n",(char*)net.c_str());
			return ERROR;
		}

		return InitByIP(srcPort,dstPort,(char*)ni.info.ip.c_str());
		//printf("info %s %s %s\n",ni.info.iface.c_str(),ni.info.mask.c_str(),ni.info.broadcast.c_str());

	}

	eErrorTp Ethernet_multicastT::InitByIP(u16 srcPort,u16 dstPort,char * SourceIP)
	{

		int opt32 = 1;
		struct timeval tv;
		memset(&mult,0,sizeof(ip_mreq));

		if (added_togroup){
			GPRINT(NORMAL_LEVEL,"network IP %s already in group\n",SourceIP);
			return NO_ERROR;
		}

		pSockSrc.stat=ssCLOSE_SOCK;
		if (InitSock()==ERROR) return ERROR;

		this->srcPort=srcPort;
		this->dstPort=dstPort;
		this->SourceIP=SourceIP;
		mult.imr_interface.s_addr = inet_addr(SourceIP);//htonl(INADDR_LOOPBACK);//INADDR_LOOPBACK);//INADDR_ANY;
		mult.imr_multiaddr.s_addr = inet_addr(GroupIP.c_str());

		//pSockSrc.DestAddr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
		pSockSrc.DestAddr.sin_addr.s_addr=mult.imr_multiaddr.s_addr;
		pSockSrc.DestAddr.sin_port=htons(dstPort);
		pSockSrc.DestAddr.sin_family=AF_INET;

		pSockSrc.SourceAddr.sin_addr.s_addr=mult.imr_multiaddr.s_addr;
		pSockSrc.SourceAddr.sin_port=htons(srcPort);
		pSockSrc.SourceAddr.sin_family=AF_INET;


		setsockopt(pSockSrc.sockSrc, SOL_SOCKET, SO_REUSEADDR, &opt32, sizeof(opt32));
		opt32=0;
		setsockopt(pSockSrc.sockDst, IPPROTO_IP, IP_MULTICAST_LOOP, &opt32, sizeof(opt32));
		opt32=1;
		setsockopt(pSockSrc.sockDst, SOL_SOCKET, SO_REUSEADDR, &opt32, sizeof(opt32));

		bind(pSockSrc.sockSrc,(sockaddr *)&pSockSrc.SourceAddr, sizeof(pSockSrc.SourceAddr));

		GPRINT(NORMAL_LEVEL,"Try add member group to multicast %s:%d...\n",GroupIP.c_str(),srcPort);

		//while(1)
		//{


		if (setsockopt(pSockSrc.sockSrc,IPPROTO_IP, IP_ADD_MEMBERSHIP, &mult, sizeof(mult))==0)
			{
				GPRINT(NORMAL_LEVEL,"Add member group to multicast on network IP %s\n",SourceIP);

				struct ::in_addr addr;
				bzero(&addr, sizeof(addr));
				addr.s_addr = mult.imr_interface.s_addr;

				setsockopt(pSockSrc.sockDst, IPPROTO_IP, IP_MULTICAST_IF, &addr, sizeof(addr));
				added_togroup=true;
				//break;
			}
		if (added_togroup==false){
			GPRINT(NORMAL_LEVEL,"Fault add member group to multicast on network IP %s\n",SourceIP);
			SockClose(pSockSrc);
			return ERROR;
		}
			//mdelay(100);
		//}

		if (setsockopt( pSockSrc.sockSrc , SOL_SOCKET, SO_RCVBUF,(int *)&RecvBuf.max_size, sizeof(RecvBuf.max_size) )==-1)
		{
			GPRINT(NORMAL_LEVEL,"Not resize receive buffer for socket\n");
		}

		tv.tv_sec=5;
		tv.tv_usec=0;
		setsockopt(pSockSrc.sockSrc,SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

		SetBlockSockNotBlockMode(pSockSrc,1);

		return NO_ERROR;
	}

	eErrorTp Ethernet_multicastT::ForceDisconnect(){


		SockClose(pSockSrc);
		GPRINT(NORMAL_LEVEL,"ForceDisconnect sip[%s] snet[%s] sport[%d]\n",SourceIP.c_str(),SourceNet.c_str(),srcPort);
		return NO_ERROR;
	}
	eErrorTp Ethernet_multicastT::ForceReconnect(){
		GPRINT(NORMAL_LEVEL,"Force reconnect sip[%s] snet[%s] sport[%d]\n",SourceIP.c_str(),SourceNet.c_str(),srcPort);
		ForceDisconnect();
		return InitByNet(srcPort,dstPort,SourceNet);
	}
	eErrorTp Ethernet_multicastT::CheckConnect(){


		if ((added_togroup==false)){
			if ((SourceIP!="")&&(srcPort!=0)){
				SockClose(pSockSrc);
				GPRINT(NORMAL_LEVEL,"Reconnect network not added in group\n");
				if (SourceNet!=""){
					return InitByNet(srcPort,dstPort,SourceNet);
				}
				else
					return InitByIP(srcPort,dstPort,(char*)SourceIP.c_str());
			}
		}


		if (SourceIP=="")//&&(SourceNet!="")&&(srcPort!=0)){
		{
			if (SourceNet=="")
				return ERROR;
			if (srcPort!=0){
				ForceReconnect();
			}
			else
				return ERROR;
		}


		if (pSockSrc.stat==ssCLOSE_SOCK){
				GPRINT(NORMAL_LEVEL,"Reconnect sock is closed\n");

				SockClose(pSockSrc);
				if (InitByIP(srcPort,dstPort,(char*)SourceIP.c_str())==ERROR)
					return ERROR;
		}




		return NO_ERROR;
	}

	eErrorTp Ethernet_multicastT::Send( char * sbuf, u32 sbuf_size)
	{

		eErrorTp sockerr=NO_ERROR;

		if (CheckConnect()==ERROR) return ERROR;
			//return ERROR;

		int outB=0;


			do
			{
				ioctl(pSockSrc.sockDst,TIOCOUTQ,&outB);
				if ((outB>MIN_BUSY_TCPUDP_QUEUE_SIZE)&(pSockSrc.stat==ssOPEN_SOCK))
					mdelay(10);
				else
					break;
			}
			while (1);

			if (pSockSrc.stat!=ssOPEN_SOCK) return ERROR;

			//printf("Send safe to sock 0x%08x size %d to 0x%08x:%d\n",Sock.sockDst,sbuf_size,htonl((u32)to->sin_addr.s_addr),htons(to->sin_port));
			if (sendto(pSockSrc.sockDst,(char*)sbuf, sbuf_size, 0,(sockaddr*)&pSockSrc.DestAddr, sizeof (struct sockaddr_in))!=(int)sbuf_size)
			{

				SockClose(pSockSrc);/*
				if (InitByIP(srcPort,dstPort,(char*)SourceIP.c_str())==ERROR)
					sockerr=ERROR;
				else{
					if (sendto(pSockSrc.sockDst,(char*)sbuf, sbuf_size, 0,(sockaddr*)&pSockSrc.DestAddr, sizeof (struct sockaddr_in))!=(int)sbuf_size){
						printf("Send error\n");
						sockerr=ERROR;
					}
					else
						sockerr=NO_ERROR;
				}*/

			}
		 return sockerr;
	}

#if 0
	eErrorTp Send(const char * sbuf, u32 sbuf_size)
	{

		eErrorTp sockerr=NO_ERROR;

		if (pSockSrc.stat==ssCLOSE_SOCK) return ERROR;

		int outB=0;
			do
			{
				ioctl(pSockSrc.sockDst,TIOCOUTQ,&outB);
				if ((outB>MIN_BUSY_TCPUDP_QUEUE_SIZE)&(pSockSrc.stat==ssOPEN_SOCK))
					mdelay(10);
				else
					break;
			}
			while (1);

		if (pSockSrc.stat!=ssOPEN_SOCK) return ERROR;


		if (send(pSockSrc.sockDst,(char*)sbuf, sbuf_size, 0)==-1)
			sockerr=ERROR;

		 return sockerr;
	}
#endif


u32 Ethernet_multicastT::Recv(u32 timeout_ms)
	{
		ssize_t bytes_read;
		u16 port;
		u32 bindex=0;
		//eErrorTp sockerr;

		u32 msize,mcsize;
		mcsize=RecvBuf.max_size;
		ClearBuf(&RecvBuf);

		if (CheckConnect()==ERROR) return RecvBuf.base_len;

		if (timeout_ms!=0){
			fds.fd=pSockSrc.sockSrc;
			fds.events = POLLIN;
			int ret = poll( &fds, 1, timeout_ms);
			if (( ret == -1 )||( ret == 0 ))
				return RecvBuf.base_len;
			if ( fds.revents & POLLIN )
				fds.revents = 0;

		}

		while(1)
		{
			if (pSockSrc.stat==ssCLOSE_SOCK)
			{

				if (InitSock()==ERROR)
				{
					sleep(1);
					continue;
				}

			}

			SockRecv(pSockSrc,(char*)RecvBuf.buf, RecvBuf.free_size, bytes_read);

			//printf("b%d\n",bytes_read);
			if (bytes_read<=0)
			{
				RecvBuf.len=0;
				break;
			}
			else
			{
				GPRINT(MEDIUM_LEVEL,"bytes_read %d\n",bytes_read);
				RecvBuf.len=bytes_read;
				//printf("port [%d]\n",htons(from.sin_port));
			}

			SafeIncTotalLenBuf(&RecvBuf,bytes_read);
			if (SafeIncPointerBuf(&RecvBuf,bytes_read)==ERROR)
			{
				 GPRINT(NORMAL_LEVEL,"UDP RCV GST Congestion, buffer small.\n");
				 msize=RecvBuf.max_size;
				 FreeBuf(&RecvBuf);
				 GPRINT(NORMAL_LEVEL,"Buffer size %d increased to  %d\n",RecvBuf.max_size,RecvBuf.max_size*2);
				 AllocBuf(&RecvBuf,msize*2);
				 if (RecvBuf.max_size>MAX_RCV_CLNT_OVER_SIZE)
				 {
					 FreeBuf(&RecvBuf);
					 AllocBuf(&RecvBuf,mcsize);
				 }

				 setsockopt( pSockSrc.sockSrc , SOL_SOCKET, SO_RCVBUF,(char *)&RecvBuf.max_size, sizeof(RecvBuf.max_size) );

				break;
			}
		}

		//printf("RecvBuf.base_len %d\n",RecvBuf.base_len);
		//if (RecvBuf.base_len!=0)
		//printf("1 em->RecvBuf.base_len %d\n",RecvBuf.base_len);
		return RecvBuf.base_len;
	}

eErrorTp Ethernet_multicastT::SockCreate(sSock & Sock, int af,int type,int protocol)
		{
			if (Sock.stat==ssOPEN_SOCK) return NO_ERROR;

			Sock.sockSrc=socket(af, type, protocol);
			Sock.sockDst=socket(af, type, protocol);

			Sock.stat=ssOPEN_SOCK;
			return NO_ERROR;
		}

eErrorTp Ethernet_multicastT::SockClose(sSock & Sock)
		{
			if (Sock.stat!=ssOPEN_SOCK) return ERROR;

			if (added_togroup){
				GPRINT(NORMAL_LEVEL,"Leave group %s\n",GroupIP.c_str());
				setsockopt(pSockSrc.sockSrc, IPPROTO_IP, IP_DROP_MEMBERSHIP,&mult, sizeof(mult));
				added_togroup=false;
			}

			shutdown( Sock.sockSrc, SHUT_RDWR );
			int err1=close(Sock.sockSrc);
			int err2=close(Sock.sockDst);
			Sock.stat=ssCLOSE_SOCK;
			Sock.sockSrc=0;
			Sock.sockDst=0;
			//Sock=0;

			//Sock.stat=ssOPEN_SOCK;

			return NO_ERROR;
		}

eErrorTp Ethernet_multicastT::InitSock()
		{

			if (SockCreate(pSockSrc,AF_INET, SOCK_DGRAM, IPPROTO_UDP)==ERROR)
				{
					 GPRINT(MEDIUM_LEVEL,"Error create UDP RAW SRC socket\n");
					 pSockSrc.stat=ssCLOSE_SOCK;
					 RecvBuf.len=RecvBuf.max_size;
					 RecvBuf.buf=NULL;
					 return ERROR;
				}
			setsockopt( pSockSrc.sockDst , SOL_SOCKET, SO_RCVBUF,(char *)&RecvBuf.max_size, sizeof(RecvBuf.max_size) );
			setsockopt( pSockSrc.sockSrc , SOL_SOCKET, SO_RCVBUF,(char *)&RecvBuf.max_size, sizeof(RecvBuf.max_size) );

			return NO_ERROR;
		}

