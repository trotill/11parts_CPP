/*
 * ipc_socket.cxx
 *
 *  Created on: 15 дек. 2018 г.
 *      Author: root
 */


#include "ipc_socket.h"


eErrorTp IpToString (u32 in,string & str)
{
  unsigned char bytes[4];
  char buffer[20];
  bytes[0]=in&0xff;
  bytes[1]=(in>>8)&0xff;
  bytes[2]=(in>>16)&0xff;
  bytes[3]=(in>>24)&0xff;
  snprintf (buffer, sizeof(buffer) , "%d.%d.%d.%d",
              bytes[0], bytes[1], bytes[2], bytes[3]);

  str=buffer;

  return NO_ERROR;
}

int SockClose(sSock & Sock)
{
	if (Sock.stat==ssCLOSE_SOCK)
		return 0;


	//shutdown( Sock.sockSrc, SHUT_RDWR );
	int err=close(Sock.sockSrc);
	Sock.stat=ssCLOSE_SOCK;
	Sock.sockSrc=0;
	//Sock=0;
	return err;
}

eErrorTp SearchOneFreePorts(string ip,u16& port,int af,int type,int protocol)
{

	u16 probeport=10000;
	sSock Socket;
	port=0;
	struct sockaddr_in SAddr;

	srand(TIME((u32*)NULL));
	u32 ipaddr=inet_addr(ip.c_str());

	SockCreate(Socket,  af, type,protocol);

	while(port==0)
	{
		probeport+=(u16)rand();

		SAddr.sin_family = PF_INET;
		SAddr.sin_port = htons(probeport);//port+1);
		SAddr.sin_addr.s_addr = ipaddr;

		//printf("probe port %d\n",probeport);
		if (bind(Socket.sockSrc , (struct sockaddr *)&SAddr, sizeof(struct sockaddr))==0)
		{
			//printf("probed port %d\n",probeport);
			port=probeport;
			SockClose(Socket);
			return NO_ERROR;
		}
	}
	return ERROR;
}

eErrorTp SearchTwoFreePorts(sSock & Socket1,sSock & Socket2,u16& port1, u16& port2)
{

	u16 probeport=10000;
	u8 truebind=0;
	sSock * Socket;
	struct sockaddr_in SAddr;

	srand(TIME((u32*)NULL));


	while(truebind<2)
	{
		probeport+=(u16)rand();

		SAddr.sin_family = PF_INET;
		SAddr.sin_port = htons(probeport);//port+1);
		SAddr.sin_addr.s_addr = 0;
		if (truebind==0) Socket=&Socket1;
		else
			Socket=&Socket2;

		if (bind(Socket->sockSrc , (struct sockaddr *)&SAddr, sizeof(struct sockaddr))==0)
		{

			truebind++;
			if (truebind==1) port1=probeport;
			if (truebind==2) port2=probeport;
		}
	}
	return NO_ERROR;
}





int SockCloseHLBus(sSock & Sock)
{
	if (Sock.stat==ssCLOSE_SOCK)
		return 0;


	int err=0;
	try
	{
		shutdown( Sock.sockSrc, SHUT_RDWR );
		if ((err=close(Sock.sockSrc))!=0)
			throw err;

		shutdown( Sock.sockDst, SHUT_RDWR );
		if ((err=close(Sock.sockDst))!=0)
			throw err;
	}
	catch(int a)
	{
		return err;
	}

	Sock.stat=ssCLOSE_SOCK;
	Sock.sockSrc=0;
	Sock.sockDst=0;
	//Sock=0;
	return err;
}

eErrorTp SetBlockSockNotBlockMode(sSock & Sock, u32 mode)
{
	int gmode=0;
	if (Sock.stat==ssCLOSE_SOCK) return ERROR;

	gmode=fcntl(Sock.sockSrc, F_GETFL, 0);

	if (mode==1)
		gmode|=O_NONBLOCK;
	else
		gmode&=~O_NONBLOCK;

	fcntl(Sock.sockSrc, F_SETFL, gmode);

	return NO_ERROR;
}

eErrorTp SockCreateLHBus(sSock & Sock, int af,int type,int protocol)
{
	if (Sock.stat==ssOPEN_SOCK) return NO_ERROR;

	Sock.sockSrc=socket(af, type, protocol);
	Sock.sockDst=socket(af, type, protocol);

	Sock.stat=ssOPEN_SOCK;
	return NO_ERROR;
}

eErrorTp SockCreate(sSock & Sock, int af,int type,int protocol)
{
	if (Sock.stat==ssOPEN_SOCK) return NO_ERROR;

	Sock.sockSrc=socket(af, type, protocol);

	Sock.stat=ssOPEN_SOCK;
	return NO_ERROR;
}

eErrorTp SockSendLHBus(sSock & Sock , const char * sbuf, u32 sbuf_size)
{

	eErrorTp sockerr=NO_ERROR;

	if (Sock.stat==ssCLOSE_SOCK) return ERROR;

	int outB=0;
		do
		{
			ioctl(Sock.sockDst,TIOCOUTQ,&outB);
			if ((outB>MIN_BUSY_TCPUDP_QUEUE_SIZE)&(Sock.stat==ssOPEN_SOCK))
				mdelay(10);
			else
				break;
		}
		while (1);

	if (Sock.stat!=ssOPEN_SOCK) return ERROR;

	if (send(Sock.sockDst,(char*)sbuf, sbuf_size, 0)==-1)
		sockerr=ERROR;

	 return sockerr;
}

eErrorTp SockSendToLHBus(sSock & Sock , char * sbuf, u32 sbuf_size,const struct sockaddr_in *to)
{

	eErrorTp sockerr=NO_ERROR;
	if (Sock.stat==ssCLOSE_SOCK) return ERROR;

	int outB=0;


		do
		{
			ioctl(Sock.sockDst,TIOCOUTQ,&outB);
			if ((outB>MIN_BUSY_TCPUDP_QUEUE_SIZE)&(Sock.stat==ssOPEN_SOCK))
				mdelay(10);
			else
				break;
		}
		while (1);

		if (Sock.stat!=ssOPEN_SOCK) return ERROR;

		//printf("Send safe to sock 0x%08x size %d to 0x%08x:%d\n",Sock.sockDst,sbuf_size,htonl((u32)to->sin_addr.s_addr),htons(to->sin_port));
		if (sendto(Sock.sockDst,(char*)sbuf, sbuf_size, 0,(sockaddr*)to, sizeof (struct sockaddr_in))!=(int)sbuf_size)
		{
			printf("SockSendToLHBus error\n");
			sockerr=ERROR;
		}
	 return sockerr;
}

eErrorTp SockSendTo(sSock & Sock , char * sbuf, u32 sbuf_size,const struct sockaddr *to)
{

	eErrorTp sockerr=NO_ERROR;
	if (Sock.stat==ssCLOSE_SOCK) return ERROR;

	int outB=0;
		do
		{
			ioctl(Sock.sockSrc,TIOCOUTQ,&outB);
			if ((outB>MIN_BUSY_TCPUDP_QUEUE_SIZE)&(Sock.stat==ssOPEN_SOCK))
				mdelay(10);
			else
				break;
		}
		while (1);

		if (Sock.stat!=ssOPEN_SOCK) return ERROR;

		if (sendto(Sock.sockSrc,(char*)sbuf, sbuf_size, 0, to, sizeof (struct sockaddr_in))!=(int)sbuf_size)
		{
			//printf("Send error\n");
			sockerr=ERROR;
		}
	 return sockerr;
}

eErrorTp SockRecv(sSock & Sock , char * sbuf, u32 sbuf_size, ssize_t & bytes_read)
{

	eErrorTp sockerr=NO_ERROR;

	if (Sock.stat==ssCLOSE_SOCK)
		return ERROR;

	bytes_read=0;
	bytes_read = recv(Sock.sockSrc , (char*)sbuf, sbuf_size, 0);

	int err=errno;

	if ((bytes_read<0)||((err!=0)&&(err!=EAGAIN)))
		sockerr=ERROR;

	if ((bytes_read==0)&&((err==EAGAIN)||(err==0)))
		sockerr=ERROR;

  return sockerr;
}

eErrorTp SockRecvFrom(sSock & Sock , char * sbuf, u32 sbuf_size, ssize_t & bytes_read,const struct sockaddr * from)
{

	eErrorTp sockerr=NO_ERROR;
	socklen_t fromlen=sizeof(struct sockaddr);
	if (Sock.stat==ssCLOSE_SOCK)
		return ERROR;


	memset((void*)from,0,sizeof(struct sockaddr));
	bytes_read = recvfrom(Sock.sockSrc , (char*)sbuf, sbuf_size, 0,(sockaddr*)from,&fromlen);

	int err=errno;
	if ((bytes_read<0)&&((err!=0)&&(err!=EAGAIN)))
		sockerr=ERROR;

	if ((bytes_read==0)&&((err==EAGAIN)||err==0))
		sockerr=ERROR;

	return sockerr;
}

eErrorTp IPC_SocketT::CloseNET_sock()
{
	if (IsMcast==false)
	{
		 if (SockClose(pSockSrc)==0)
		  return ERROR;
	}
	else
	{
		 if (SockCloseHLBus(pSockSrc)==0)
			 return ERROR;
	}
	return NO_ERROR;
}

eErrorTp IPC_SocketT::CreateNET_sock()
{
	if (IsMcast==false)
	{
		if (SockCreate(pSockSrc,AF_INET, SOCK_DGRAM, IPPROTO_UDP)==ERROR)
		{
			 GPRINT(MEDIUM_LEVEL,"Error create UDP RAW SRC socket\n");
			 pSockSrc.stat=ssCLOSE_SOCK;
			 RawBuf.len=RawBuf.max_size;
			 RawBuf.buf=NULL;
			 return ERROR;
		}
	}
	else
	{
		if (SockCreateLHBus(pSockSrc,AF_INET, SOCK_DGRAM, IPPROTO_UDP)==ERROR)
			{
				 GPRINT(MEDIUM_LEVEL,"Error create UDP RAW SRC socket\n");
				 pSockSrc.stat=ssCLOSE_SOCK;
				 RawBuf.len=RawBuf.max_size;
				 RawBuf.buf=NULL;
				 return ERROR;
			}
		setsockopt( pSockSrc.sockDst , SOL_SOCKET, SO_RCVBUF,(char *)&RawBuf.max_size, sizeof(RawBuf.max_size) );
	}

	setsockopt( pSockSrc.sockSrc , SOL_SOCKET, SO_RCVBUF,(char *)&RawBuf.max_size, sizeof(RawBuf.max_size) );

	return NO_ERROR;
}
u32 IPC_SocketT::ReadNetUDP()
{
	ssize_t bytes_read;
	u16 port;
	u32 bindex=0;
	//eErrorTp sockerr;

	u32 msize,mcsize;
	mcsize=RawBuf.max_size;
	while(1)
	{
		if (pSockSrc.stat==ssCLOSE_SOCK)
		{
			if (CreateNET_sock()==ERROR)
			{
				sleep(1);
				continue;
			}

		}

		SockRecv(pSockSrc,(char*)RawBuf.buf, RawBuf.free_size, bytes_read);

		//printf("b%d\n",bytes_read);
		if (bytes_read<=0)
		{
			RawBuf.len=0;
			break;
		}
		else
		{
			GPRINT(MEDIUM_LEVEL,"bytes_read %d\n",bytes_read);
			RawBuf.len=bytes_read;
			//printf("port [%d]\n",htons(from.sin_port));
		}

		SafeIncTotalLenBuf(&RawBuf,bytes_read);
		if (SafeIncPointerBuf(&RawBuf,bytes_read)==ERROR)
		{
			 GPRINT(NORMAL_LEVEL,"UDP RCV GST Congestion, buffer small.\n");
			 msize=RawBuf.max_size;
			 FreeBuf(&RawBuf);
			 GPRINT(NORMAL_LEVEL,"Buffer size %d increased to  %d\n",RawBuf.max_size,RawBuf.max_size*2);
			 AllocBuf(&RawBuf,msize*2);
			 if (RawBuf.max_size>MAX_RCV_CLNT_OVER_SIZE)
			 {
				 FreeBuf(&RawBuf);
				 AllocBuf(&RawBuf,mcsize);
			 }

			 setsockopt( pSockSrc.sockSrc , SOL_SOCKET, SO_RCVBUF,(char *)&RawBuf.max_size, sizeof(RawBuf.max_size) );

			break;
		}
	}


	return RawBuf.base_len;
}

eErrorTp IPC_SocketT::InitLHBus(u16 srcPort,u16 dstPort,char * GroupIP)
{

	//printf("sport %d dport %d %s\n",srcPort,dstPort,GroupIP);
    struct ip_mreq mult;
	int opt32 = 1;
	struct timeval tv;
	IsMcast=true;
	memset(&mult,0,sizeof(ip_mreq));

	pSockSrc.stat=ssCLOSE_SOCK;
	if (CreateNET_sock()==ERROR) return ERROR;


	mult.imr_interface.s_addr = htonl(INADDR_LOOPBACK);//INADDR_LOOPBACK);//INADDR_ANY;
	mult.imr_multiaddr.s_addr = inet_addr(GroupIP);

	//pSockSrc.DestAddr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
	pSockSrc.DestAddr.sin_addr.s_addr=mult.imr_multiaddr.s_addr;
	pSockSrc.DestAddr.sin_port=htons(dstPort);
	pSockSrc.DestAddr.sin_family=AF_INET;

	pSockSrc.SourceAddr.sin_addr.s_addr=mult.imr_multiaddr.s_addr;
	pSockSrc.SourceAddr.sin_port=htons(srcPort);
	pSockSrc.SourceAddr.sin_family=AF_INET;


	setsockopt(pSockSrc.sockSrc, SOL_SOCKET, SO_REUSEADDR, &opt32, sizeof(opt32));
	setsockopt(pSockSrc.sockDst, SOL_SOCKET, SO_REUSEADDR, &opt32, sizeof(opt32));

	bind(pSockSrc.sockSrc,(sockaddr *)&pSockSrc.SourceAddr, sizeof(pSockSrc.SourceAddr));

	GPRINT(NORMAL_LEVEL,"Try add member group to multicast %s:%d...\n",GroupIP,srcPort);

	while(1)
	{
		if (setsockopt(pSockSrc.sockSrc,IPPROTO_IP, IP_ADD_MEMBERSHIP, &mult, sizeof(mult))==0)
		{
			GPRINT(NORMAL_LEVEL,"Add member group to multicast\n");

			struct ::in_addr addr;
			bzero(&addr, sizeof(addr));
			addr.s_addr = mult.imr_interface.s_addr;

			setsockopt(pSockSrc.sockDst, IPPROTO_IP, IP_MULTICAST_IF, &addr, sizeof(addr));
			break;
		}
		mdelay(100);
	}

	if (setsockopt( pSockSrc.sockSrc , SOL_SOCKET, SO_RCVBUF,(int *)&RawBuf.max_size, sizeof(RawBuf.max_size) )==-1)
	{
		GPRINT(NORMAL_LEVEL,"Not resize receive buffer for socket\n");
	}

	tv.tv_sec=5;
	tv.tv_usec=0;
	setsockopt(pSockSrc.sockSrc,SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	SetBlockSockNotBlockMode(pSockSrc,1);

	return NO_ERROR;
}

eErrorTp IPC_SocketT::SockSendToLH_Bus( char * sbuf, u32 sbuf_size)
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

		//printf("Send safe to sock 0x%08x size %d to 0x%08x:%d\n",Sock.sockDst,sbuf_size,htonl((u32)to->sin_addr.s_addr),htons(to->sin_port));
		if (sendto(pSockSrc.sockDst,(char*)sbuf, sbuf_size, 0,(sockaddr*)&pSockSrc.DestAddr, sizeof (struct sockaddr_in))!=(int)sbuf_size)
		{
			printf("IPC_SocketT::SockSendToLH_Bus error\n");
			sockerr=ERROR;
		}
	 return sockerr;
}
