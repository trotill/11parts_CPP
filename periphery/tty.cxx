/**
 * tty.cxx
 *
 * Author: gorchakov.ilya
 * Email: ilya.gorchakov@adakta.ru
 */

/**
 * Class for working with UART chip, provides wrapper for standard API
 */

#include "tty.h"
#include <linux/serial.h>
#if 1
eErrorTp TTYclass::Init(string & tty, u32 brate, u8 stopbit, int exflags,u32 openparam){
	 struct termios tios;
	    speed_t speed;
	 // int flags;


	 /* The O_NOCTTY flag tells UNIX that this program doesn't want
	       to be the "controlling terminal" for that port. If you
	       don't specify this then any input (such as keyboard abort
	       signals and so forth) will affect your process
	       Timeouts are ignored in canonical input mode or when the
	       NDELAY option is set on the file via open or fcntl */
	   // flags = O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL;
	//#ifdef O_CLOEXEC
	  //  flags |= O_CLOEXEC;
	//#endif

	    //if (TTYopen(openparam)==ERROR){
	    	//		return ERROR;
	    		//}
	    TTYFname=tty;
	  //  comfd = open(TTYFname.c_str(), flags);
	    if (TTYopen(openparam)==ERROR) {
	    	GPRINT(NORMAL_LEVEL,"Not open port [%s]\n",TTYFname.c_str());
	    			return ERROR;
	    }

	  //  FD_SET(comfd, &readfs);


	    struct termios old_tios;
	    /* Save */
	    tcgetattr(comfd, &old_tios);

	    memset(&tios, 0, sizeof(struct termios));

	    /* C_ISPEED     Input baud (new interface)
	       C_OSPEED     Output baud (new interface)
	    */
	    switch (brate) {
	    case 110:
	        speed = B110;
	        break;
	    case 300:
	        speed = B300;
	        break;
	    case 600:
	        speed = B600;
	        break;
	    case 1200:
	        speed = B1200;
	        break;
	    case 2400:
	        speed = B2400;
	        break;
	    case 4800:
	        speed = B4800;
	        break;
	    case 9600:
	        speed = B9600;
	        break;
	    case 19200:
	        speed = B19200;
	        break;
	    case 38400:
	        speed = B38400;
	        break;
	#ifdef B57600
	    case 57600:
	        speed = B57600;
	        break;
	#endif
	#ifdef B115200
	    case 115200:
	        speed = B115200;
	        break;
	#endif
	#ifdef B230400
	    case 230400:
	        speed = B230400;
	        break;
	#endif
	#ifdef B460800
	    case 460800:
	        speed = B460800;
	        break;
	#endif
	#ifdef B500000
	    case 500000:
	        speed = B500000;
	        break;
	#endif
	#ifdef B576000
	    case 576000:
	        speed = B576000;
	        break;
	#endif
	#ifdef B921600
	    case 921600:
	        speed = B921600;
	        break;
	#endif
	#ifdef B1000000
	    case 1000000:
	        speed = B1000000;
	        break;
	#endif
	#ifdef B1152000
	   case 1152000:
	        speed = B1152000;
	        break;
	#endif
	#ifdef B1500000
	    case 1500000:
	        speed = B1500000;
	        break;
	#endif
	#ifdef B2500000
	    case 2500000:
	        speed = B2500000;
	        break;
	#endif
	#ifdef B3000000
	    case 3000000:
	        speed = B3000000;
	        break;
	#endif
	#ifdef B3500000
	    case 3500000:
	        speed = B3500000;
	        break;
	#endif
	#ifdef B4000000
	    case 4000000:
	        speed = B4000000;
	        break;
	#endif
	    default:
	        speed = B9600;
	        GPRINT(NORMAL_LEVEL,"WARNING Unknown baud rate %d for %s (B9600 used)\n");

	    }

	    /* Set the baud rate */
	    if ((cfsetispeed(&tios, speed) < 0) ||
	        (cfsetospeed(&tios, speed) < 0)) {
	        close(comfd);
	        comfd = -1;
	        return ERROR;
	    }

	    /* C_CFLAG      Control options
	       CLOCAL       Local line - do not change "owner" of port
	       CREAD        Enable receiver
	    */
	    tios.c_cflag |= (CREAD | CLOCAL);
	    /* CSIZE, HUPCL, CRTSCTS (hardware flow control) */

	    /* Set data bits (5, 6, 7, 8 bits)
	       CSIZE        Bit mask for data bits
	    */
	    tios.c_cflag &= ~CSIZE;
	   // tios.c_cflag |= CS8;
	    /* PARENB       Enable parity bit
	       PARODD       Use odd parity instead of even */
	    tios.c_cflag &=~ PARENB;

	    tios.c_cflag |=exflags;

	    /* Stop bit (1 or 2) */
	    if (stopbit == 1)
	        tios.c_cflag &=~ CSTOPB;
	    else /* 2 */
	        tios.c_cflag |= CSTOPB;




	    /* Read the man page of termios if you need more information. */

	    /* This field isn't used on POSIX systems
	       tios.c_line = 0;
	    */

	    /* C_LFLAG      Line options
	       ISIG Enable SIGINTR, SIGSUSP, SIGDSUSP, and SIGQUIT signals
	       ICANON       Enable canonical input (else raw)
	       XCASE        Map uppercase \lowercase (obsolete)
	       ECHO Enable echoing of input characters
	       ECHOE        Echo erase character as BS-SP-BS
	       ECHOK        Echo NL after kill character
	       ECHONL       Echo NL
	       NOFLSH       Disable flushing of input buffers after
	       interrupt or quit characters
	       IEXTEN       Enable extended functions
	       ECHOCTL      Echo control characters as ^char and delete as ~?
	       ECHOPRT      Echo erased character as character erased
	       ECHOKE       BS-SP-BS entire line on line kill
	       FLUSHO       Output being flushed
	       PENDIN       Retype pending input at next read or input char
	       TOSTOP       Send SIGTTOU for background output
	       Canonical input is line-oriented. Input characters are put
	       into a buffer which can be edited interactively by the user
	       until a CR (carriage return) or LF (line feed) character is
	       received.
	       Raw input is unprocessed. Input characters are passed
	       through exactly as they are received, when they are
	       received. Generally you'll deselect the ICANON, ECHO,
	       ECHOE, and ISIG options when using raw input
	    */

	    /* Raw input */
	    tios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	    /* C_IFLAG      Input options
	       Constant     Description
	       INPCK        Enable parity check
	       IGNPAR       Ignore parity errors
	       PARMRK       Mark parity errors
	       ISTRIP       Strip parity bits
	       IXON Enable software flow control (outgoing)
	       IXOFF        Enable software flow control (incoming)
	       IXANY        Allow any character to start flow again
	       IGNBRK       Ignore break condition
	       BRKINT       Send a SIGINT when a break condition is detected
	       INLCR        Map NL to CR
	       IGNCR        Ignore CR
	       ICRNL        Map CR to NL
	       IUCLC        Map uppercase to lowercase
	       IMAXBEL      Echo BEL on input line too long
	    */

	        tios.c_iflag &= ~INPCK;


	    /* Software flow control is disabled */
	    tios.c_iflag &= ~(IXON | IXOFF | IXANY);

	    /* C_OFLAG      Output options
	       OPOST        Postprocess output (not set = raw output)
	       ONLCR        Map NL to CR-NL
	       ONCLR ant others needs OPOST to be enabled
	    */

	    /* Raw ouput */
	    tios.c_oflag &=~ OPOST;

	    /* C_CC         Control characters
	       VMIN         Minimum number of characters to read
	       VTIME        Time to wait for data (tenths of seconds)
	       UNIX serial interface drivers provide the ability to
	       specify character and packet timeouts. Two elements of the
	       c_cc array are used for timeouts: VMIN and VTIME. Timeouts
	       are ignored in canonical input mode or when the NDELAY
	       option is set on the file via open or fcntl.
	       VMIN specifies the minimum number of characters to read. If
	       it is set to 0, then the VTIME value specifies the time to
	       wait for every character read. Note that this does not mean
	       that a read call for N bytes will wait for N characters to
	       come in. Rather, the timeout will apply to the first
	       character and the read call will return the number of
	       characters immediately available (up to the number you
	       request).
	       If VMIN is non-zero, VTIME specifies the time to wait for
	       the first character read. If a character is read within the
	       time given, any read will block (wait) until all VMIN
	       characters are read. That is, once the first character is
	       read, the serial interface driver expects to receive an
	       entire packet of characters (VMIN bytes total). If no
	       character is read within the time allowed, then the call to
	       read returns 0. This method allows you to tell the serial
	       driver you need exactly N bytes and any read call will
	       return 0 or N bytes. However, the timeout only applies to
	       the first character read, so if for some reason the driver
	       misses one character inside the N byte packet then the read
	       call could block forever waiting for additional input
	       characters.
	       VTIME specifies the amount of time to wait for incoming
	       characters in tenths of seconds. If VTIME is set to 0 (the
	       default), reads will block (wait) indefinitely unless the
	       NDELAY option is set on the port with open or fcntl.
	    */
	    /* Unused because we use open with the NDELAY option */
	    tios.c_cc[VMIN] = 0;
	    tios.c_cc[VTIME] = 0;

	   /* struct serial_struct serial;
	    ioctl(comfd, TIOCGSERIAL, &serial);
	    serial.flags |= ASYNC_LOW_LATENCY;
	    serial.xmit_fifo_size = 10; // what is "xmit" ??
	    ioctl(comfd, TIOCSSERIAL, &serial);*/

	    if (tcsetattr(comfd, TCSANOW, &tios) < 0) {
	        close(comfd);
	        comfd = -1;
	        return ERROR;
	    }


	   /* struct serial_struct serial;
	    ioctl(comfd, TIOCGSERIAL, &serial);
	    serial.flags |= ASYNC_LOW_LATENCY;
	    serial.xmit_fifo_size = 1000000; // what is "xmit" ??
	    ioctl(comfd, TIOCSSERIAL, &serial);*/

	    return NO_ERROR;
}

#endif
#if 0
eErrorTp TTYclass::Init(string & tty, u32 brate, u8 stopbit, int exflags,u32 openparam){

	struct termio stbuf;
	u32 speed=brate;
		TTYFname=tty;
		comfd=-1;
		//printf("TTY log_level %d\n",log_level);
		GPRINT(MEDIUM_LEVEL,"Probe init [%s] speed [%d]\n",tty.c_str(),speed);
		if (TTYopen(openparam)==ERROR){
			return ERROR;
		}

		if (ioctl (comfd, TCGETA, &stbuf) < 0) {
			GPRINT(NORMAL_LEVEL,"Not get TTY params\n");
			return ERROR;
		  }

		int c_cflag=exflags|CS8|CREAD;

		if (speed==1000000) c_cflag|=B1000000;
		if (speed==921600) c_cflag|=B921600;
		if (speed==576000) c_cflag|=B576000;
		if (speed==500000) c_cflag|=B500000;
		if (speed==460800) c_cflag|=B460800;
		if (speed==230400) c_cflag|=B230400;
		if (speed==115200) c_cflag|=B115200;
		if (speed==19200) c_cflag|=B19200;
		if (speed==9600) c_cflag|=B9600;
		if (speed==4800) c_cflag|=B4800;
		if (stopbit>1)
			c_cflag|=CSTOPB;

		stbuf.c_lflag    = 0;
		stbuf.c_iflag    = BRKINT;
		stbuf.c_oflag    = 0;
		stbuf.c_cflag    = c_cflag;
		stbuf.c_cc[VMIN] = 0;
		stbuf.c_cc[VTIME] = 0;

		if (ioctl(comfd, TCSETA, &stbuf) < 0) {
			GPRINT(NORMAL_LEVEL,"Not set TTY params\n");
			return ERROR;
		  }

		GPRINT(NORMAL_LEVEL,"Init conrol port [%s]\n",tty.c_str());

		//struct termios oldtio;
		//tcgetattr(comfd,&oldtio);
	    //tcflush(comfd, TCIFLUSH);
	    //tcsetattr(comfd,TCSADRAIN,&oldtio);

	return NO_ERROR;
}
#endif

TTYclass::TTYclass(string & tty, string & parens, char * name, u32 speed, u8 stopbit, int exflags,eDebugTp log_level)
{
	//
	debug_level=log_level;
	glbERROR=NO_ERROR;
	SourceStr=parens;
	ObjPref=name;
	glbERROR=Init(tty,speed,stopbit,exflags,O_NONBLOCK|O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL);
}

TTYclass::TTYclass(string & tty, string & parens, char * name, u32 speed, u8 stopbit, int exflags,u32 openparam,eDebugTp log_level)
{
	debug_level=log_level;
	glbERROR=NO_ERROR;
	SourceStr=parens;
	ObjPref=name;
	glbERROR=Init(tty,speed,stopbit,exflags,openparam);
}

int TTYclass::setRTS(int level)
{
    int rts;

    if (comfd==-1) return ERROR;

    if (ioctl(comfd, TIOCMGET, &rts) == -1) {
        perror("setRTS(): TIOCMGET");
        return 0;
    }
    if (level)
        rts |= TIOCM_RTS;
    else
        rts &= ~TIOCM_RTS;

    if (ioctl(comfd, TIOCMSET, &rts) == -1) {
        perror("setRTS(): TIOCMSET");
        return 0;
    }
    return 1;
}

eErrorTp TTYclass::ConfRS485(){
	struct serial_rs485 rs485conf;
	if (comfd==-1)
		return ERROR;

	ioctl ( comfd, TIOCGRS485, &rs485conf );
	//SER_RS485_RTS_ON_SEND
	rs485conf.flags = ( SER_RS485_ENABLED|SER_RS485_RTS_ON_SEND|SER_RS485_RTS_ON_SEND);// |(~(SER_RS485_RTS_ON_SEND))| ~(SER_RS485_RTS_AFTER_SEND) | ~(SER_RS485_RX_DURING_TX));
	ioctl ( comfd, TIOCSRS485, &rs485conf );
	return NO_ERROR;
}

TTYclass::~TTYclass(void)
{
	if (glbERROR==NO_ERROR)
	{
		if (comfd!=-1)
			TTYclose();
	}
}

eErrorTp TTYclass::TTYopen(u32 openparam)
{


	if (comfd!=-1) return ERROR;

	if ((comfd = open(TTYFname.c_str(), openparam)) <0)
	{
		GPRINT(NORMAL_LEVEL,"Not open port [%s]\n",TTYFname.c_str());
		return ERROR;
	}

	GPRINT(MEDIUM_LEVEL,"Open port[%s]\n",TTYFname.c_str());


	return NO_ERROR;

}

void TTYclass::TTYclose(void)
{
	GPRINT(MEDIUM_LEVEL,"close port[%s]\n",TTYFname.c_str());
	close(comfd);
	comfd=-1;
}

eErrorTp TTYclass::TTYClear(void)
{
	char buff;
	if (comfd==-1) return ERROR;
	tcflush(comfd, TCIFLUSH);
	return NO_ERROR;
}

eErrorTp TTYclass::TTYreadChar(u8 & buff)
{
	u32 i,j;
	int res;

	if (comfd==-1)
	{
		return ERROR;
	}
	res=read(comfd,&buff,1);
	if (res==-1)
		return ERROR;


	return NO_ERROR;
}

u32 TTYclass::TTYreadBuff(u8 * buff)
{
	u32 i=0;
	int res;
	//printf("TTYreadBuff\n");
	if (comfd==-1)
	{
		return 0;
	}

	while(1){
		res=read(comfd,&buff[i],1);
		if ((res==-1)||(res==0))
			break;
		i++;
	}


	return i;
}

u32 TTYclass::TTYnreadBuff(u8 * buff,u32 max_size)
{
	u32 i=0;
	int res;

	if (comfd==-1)
	{
		return 0;
	}
	//printf("TTYreadBuff max_size %d\n",max_size);
	for (i=0;i<max_size;i++) {
		res=read(comfd,&buff[i],1);
		//if (res!=0)
		//printf("res %d %d\n",res,i);
		if ((res==-1)||(res==0))
			break;
		//i++;
	}
	//if (res>0)
	//  printf("TTYreadBuff %d\n",i);
	return i;
}


#if 0
u32 TTYclass::TTYnreadBuffBlocked(u8 * buff,u32 max_size,u32 timeout)
{
	u32 i=0;
	int res;

	if (comfd==-1){
		return 0;
	}

	//RxTimeout.tv_usec = 200000;  /* milliseconds */
	//RxTimeout.tv_sec  = 0;  /* seconds */
	//printf("TTYnreadBuffBlocked\n");
	//select(comfd+1, &readfs, NULL, NULL, &RxTimeout);
	timeout=180;
	while(1){
		fds.fd=comfd;
		fds.events = POLLIN;
		int ret = poll( &fds, 1, timeout);
		if (( ret == -1 )||( ret == 0 )){
			printf("pull out i %d\n",i);
			break;
		}

		if ( fds.revents & POLLIN )
			fds.revents = 0;

	//	for (i=0;i<max_size;i++) {
			res=read(comfd,&buff[i],1);

			//if (res!=0)
			//printf("res %d i %d max_size %d \n",res,i,max_size);
			if ((res!=-1)||(res!=0)){
				i+=res;

				if (max_size<=(i-1)){
					break;
				}
			}
			//i++;
		//}
	}
	//if (res>0)
	 // printf("TTYreadBuff %d\n",i);
	//printf("%d\n",i);
	return i;
}
#endif
#if 1
u32 TTYclass::TTYnreadBuffBlocked(u8 * buff,u32 max_size,u32 timeout)
{
	u32 i=0;
	int res;

	if (comfd==-1){
		return 0;
	}

	//RxTimeout.tv_usec = 200000;  /* milliseconds */
	//RxTimeout.tv_sec  = 0;  /* seconds */
	//printf("TTYnreadBuffBlocked\n");
	//select(comfd+1, &readfs, NULL, NULL, &RxTimeout);
	fds.fd=comfd;
	fds.events = POLLIN;
	int ret = poll( &fds, 1, timeout);
	if (( ret == -1 )||( ret == 0 ))
		return 0;

	if ( fds.revents & POLLIN )
		fds.revents = 0;

	for (i=0;i<max_size;i++) {
		res=read(comfd,&buff[i],1);
		//if (res!=0)
		//printf("res %d %d\n",res,i);
		if ((res==-1)||(res==0))
			break;
		//i++;
	}
	//if (res>0)
	 // printf("TTYreadBuff %d\n",i);
	//printf("%d\n",i);
	return i;
}
#endif

eErrorTp TTYclass::TTYwriteChar(u8 & buff)
{
	u32 i,j;
	int res;
	if (comfd==-1) return ERROR;
	res=write(comfd,&buff,1);
	if (res==-1) return ERROR;

	return NO_ERROR;
}

eErrorTp TTYclass::TTYwriteBuf(u8 * buff, u32 len)
{
	u32 i,j;
		u32 res;
		if (comfd==-1) return ERROR;
		//printhex((u8*)buff,len,16);
		res=write(comfd,buff,len);
		if (res!=len)
		{
			printf("TTYclass::TTYwriteBuf res(%d)!=len(%d)\n",res,len);
			return ERROR;
		}

	return NO_ERROR;
}

eErrorTp TTYclass::TTYwriteBufBlocked(u8 * buff, u32 len)
{
	u32 i,j;
	int res;
	if (comfd==-1) return ERROR;
	//printhex((u8*)buff,len,16);
	int wrc=len;
	int offs=0;

	while(1){
		fdwr.fd=comfd;
		fdwr.events = POLLOUT;
		int ret = poll( &fdwr, 1, 0);
		if (( ret == -1 )||( ret == 0 )){
			usleep(50);
			continue;
		}

		if ( fdwr.revents & POLLOUT )
			fdwr.revents = 0;

		if ((offs+wrc)>(int)len){
			printf("TTYclass::TTYwriteBuf (offs+wrc)(%d)>len(%d)\n",offs+wrc,len);
			return ERROR;
		}
		res=write(comfd,&buff[offs],wrc);


		if (res==-1)
		{
			printf("TTYclass::TTYwriteBuf res -1\n");
			return ERROR;
		}
		if (res>wrc)
		{
			printf("TTYclass::TTYwriteBuf res(%d)>len(%d)\n",res,len);
			return ERROR;
		}
		wrc=wrc-res;
		offs+=res;

		if (wrc==0)
			break;
	}

	return NO_ERROR;
}
