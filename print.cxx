/*
 * print.cxx
 *
 *  Created on: 17 дек. 2018 г.
 *      Author: root
 */

#include "engine/print.h"
#include "engine/memadmin.h"
#include "engine/lib/11p_string.h"
#include "engine/global.h"

u32 GprintT::GPRINT(eDebugTp level,char * format, ...)
{
	 if (level>debug_level)
		 return ERROR;

    int size = 200;
    u32 slen=0;
    char * chstr;
    int n;
    BufHandler OutBuf;
    va_list va;
    eErrorTp err=NO_ERROR;
    sInterThrMsgHeader Mheader;


	InitBuf(&OutBuf);
	AllocBuf(&OutBuf,size);
	if (SourceStr!="")
	{
		snprintf((char*)OutBuf.buf,size,"[%s]",(char*)SourceStr.c_str());
		AddCStringToBuf(&OutBuf);
	}
	if (ObjPref!="")
	{
		snprintf((char*)OutBuf.buf,size,"<%s>",(char*)ObjPref.c_str());
		AddCStringToBuf(&OutBuf);
	}

    while (1) {
        //str.resize(size);
    	va_start(va, format);
        n = vsnprintf((char *)OutBuf.buf, OutBuf.free_size-1, format, va);
        va_end(va);

        if (n > -1 && n < (int)OutBuf.free_size) {
        	if (AddCStringToBuf(&OutBuf)!=ERROR)
        	{
        		if (AddZeroToBuf(&OutBuf)!=ERROR)
        			break;
        		//printf("Error add cstring\n");
        	}

        }
        if (IncreaseBufSize(&OutBuf,size)==ERROR)
        {
        	printf("Error  Increase\n");
        	err=ERROR;
        	break;
        }

    }

    if (err==NO_ERROR)
    {
#ifndef _WO_CNT_DEP
    	mutex_lock(CnT->ConsLogMutex);
#endif
    	WriteToFile((char*)OutBuf.buf_base);
    	printf("%s",OutBuf.buf_base);
#ifndef _WO_CNT_DEP
    	mutex_unlock(CnT->ConsLogMutex);
#endif
    }
    FreeBuf(&OutBuf);

	return err;
}
