/*
 * memadmin.h
 *
 *  Created on: 02 февр. 2014 г.
 *      Author: root
 */

#ifndef MEMADMIN_H_
#define MEMADMIN_H_

#include <engine/types.h>
#include <engine/buffer.h>


#define FIFO_PREBUF_LEN MAX_RCV_CLNT_BUF
#define QCOUNT_MAX_SIZE 1000000
#define INTER_MSG_LEN_TYPE u32




class fifo_element
{
	public:
	fifo_element(void) noexcept;
	u8 * GetBuf(u32 & size);
	eErrorTp SetBuf(u8 * srcbuf,u32 size);
	~fifo_element();
	u32 bufsize=0;
	private:
		u8 * buf=NULL;

};

class fragfifo
{
	public:
		fragfifo();
		~fragfifo();
		eErrorTp PushBuf(u8 * buf, u32 size);
		u8 * GetLastBuf(u32 & size);
		int GetCountElements();
		int GetTotalSize();
		int GetTotalSizeLastElements(int countElements);
	private:
		int qcount=0;
		u32 qcount_bsize=0;
		u32 qcount_max_bsize=QCOUNT_MAX_SIZE;
		list<fifo_element> fifo;
		Mutex_t FifoMutex;
		//fifo_element felement;
};

u32 GetBufFreeSize(BufHandler * buf);
eErrorTp AddToBuf(BufHandler * dstbuf, u8 * srcbuf, u32 srcsize);
eErrorTp InitBuf(BufHandler * buf);
eErrorTp ClearBuf(BufHandler * buf);
eErrorTp AllocBuf(BufHandler * buf,u32 size);
eErrorTp FreeBuf(BufHandler * buf);
eErrorTp SafeMoveBuf(BufHandler * dstbuf, BufHandler * srcbuf, u32 srcmovesize);
eErrorTp SafeIncPointerBuf(BufHandler * buf, u32 inc);
eErrorTp SafeIncTotalLenBuf(BufHandler * buf, u32 inc);
eErrorTp SafeCopyBufToBaseBuf(BufHandler * dstbuf);
eErrorTp CopyFragmentBuf(BufHandler * dstbuf, BufHandler * srcbuf, u32 srcfragsize);
eErrorTp CopyToBuf(BufHandler * dstbuf, u8 * srcbuf, u32 srcsize);
eErrorTp BufClone(BufHandler * bufSrc,BufHandler * bufDst);
eErrorTp SafeMoveToStartPointerBuf(BufHandler * buf);
u8 * AddBufDataToField(u8 * buf,u8 * fielddata,u32 fieldlen);
eErrorTp BufToBufHandler(BufHandler * dstbuf, u8 * sbuf,u32 sbuf_len, u32 maxsize);
eErrorTp IncreaseBufSize(BufHandler * buf,u32 size);
eErrorTp CreateBuf(BufHandler * buf,u32 size);
eErrorTp CreateForFifoSendCast(BufHandler * buf,u32 size);
eErrorTp TestIsFreeBuf(BufHandler * buf);

#endif /* MEMADMIN_H_ */
