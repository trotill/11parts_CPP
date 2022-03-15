/*
 * memadmin.cxx
 *
 *  Created on: 02 февр. 2014 г.
 *      Author: root
 */

#include "engine/memadmin.h"
#include "engine/lib/11p_string.h"

eErrorTp CreateForFifoSendCast(BufHandler * buf,u32 size)
{

	return CreateBuf(buf,(size+sizeof(sInterThrMsgHeader))*2);
}

eErrorTp CreateBuf(BufHandler * buf,u32 size)
{
	InitBuf(buf);
	if (AllocBuf(buf,size)==ERROR) return ERROR;
	//ClearBuf(buf);
	return NO_ERROR;
}

eErrorTp SafeMoveToStartPointerBuf(BufHandler * buf)
{
	buf->buf=buf->buf_base;
	buf->len=buf->base_len;

	return NO_ERROR;
}

eErrorTp SafeIncTotalLenBuf(BufHandler * buf, u32 inc)
{
	if (buf->free_size>=inc)
		{
			buf->base_len+=inc;
			buf->free_size-=inc;
			return NO_ERROR;
		}
	return ERROR;
}

eErrorTp SafeIncPointerBuf(BufHandler * buf, u32 inc)
{

	if (buf->free_size>=inc)
	{
		buf->buf+=inc;
		buf->len=0;
		return NO_ERROR;
	}

	return ERROR;
}

eErrorTp SafeMoveBuf(BufHandler * dstbuf, BufHandler * srcbuf, u32 srcmovesize)
{

	dstbuf->buf+=dstbuf->len;
	memcpy(dstbuf->buf,srcbuf->buf,srcmovesize);

	dstbuf->len=srcmovesize;
	dstbuf->base_len+=srcmovesize;
	srcbuf->len-=srcmovesize;
	srcbuf->buf+=srcmovesize;
	return NO_ERROR;
}


eErrorTp CopyFragmentBuf(BufHandler * dstbuf, BufHandler * srcbuf, u32 srcfragsize)
{

	if (srcbuf->len<srcfragsize)
	{
		CRIT_ERROR
		return ERROR;
	}

	if ((srcfragsize+dstbuf->buf)>(dstbuf->max_size+dstbuf->buf_base))
	{
		CRIT_ERROR
		return ERROR;
	}

	memcpy(&dstbuf->buf_base[dstbuf->base_len],srcbuf->buf,srcfragsize);
	dstbuf->len=srcfragsize;
	dstbuf->base_len+=srcfragsize;
	return NO_ERROR;
}

eErrorTp SafeCopyBufToBaseBuf(BufHandler * dstbuf)
{

	u8 * tmpbuf;

		if (dstbuf->len>dstbuf->max_size)
		{
			CRIT_ERROR;
			return ERROR;
		}

		tmpbuf=(u8*)calloc(dstbuf->len,1);//new u8[dstbuf->len];

		memcpy(tmpbuf,dstbuf->buf,dstbuf->len);
		memcpy(dstbuf->buf_base,tmpbuf,dstbuf->len);

		dstbuf->buf=dstbuf->buf_base;
		dstbuf->base_len=dstbuf->len;
		free(tmpbuf);//delete [] tmpbuf;

	return NO_ERROR;
}

u32 GetBufFreeSize(BufHandler * buf)
{
	if (buf->base_len>buf->max_size)
	{
		CRIT_ERROR;
		buf->base_len=buf->max_size;
		buf->buf=buf->buf_base;
		buf->len=0;
	}
	return buf->max_size-buf->base_len;
}


eErrorTp CopyToBuf(BufHandler * dstbuf, u8 * srcbuf, u32 srcsize)
{
	if (GetBufFreeSize(dstbuf)<srcsize)
	{
		CRIT_ERROR;
		return ERROR;
	}

	memcpy(dstbuf->buf,srcbuf,srcsize);

	dstbuf->base_len-=dstbuf->len;
	dstbuf->free_size+=dstbuf->len;

	dstbuf->len=srcsize;
	dstbuf->base_len+=srcsize;
	dstbuf->free_size-=srcsize;
	return NO_ERROR;
}

eErrorTp AddToBuf(BufHandler * dstbuf, u8 * srcbuf, u32 srcsize)
{
	if (GetBufFreeSize(dstbuf)<srcsize)
	{
		CRIT_ERROR;
		return ERROR;
	}

	dstbuf->buf+=dstbuf->len;
	memcpy(dstbuf->buf,srcbuf,srcsize);

	dstbuf->len=srcsize;
	dstbuf->base_len+=srcsize;
	return NO_ERROR;
}

eErrorTp BufClone(BufHandler * bufSrc,BufHandler * bufDst)
{
	memcpy(bufDst,bufSrc,sizeof(BufHandler));

	return NO_ERROR;
}

eErrorTp AllocBuf(BufHandler * buf,u32 size)
{
	if (buf->buf_base!=NULL)
	{
		CRIT_ERROR;
		return ERROR;
	}

	buf->buf_base=(u8*)calloc(size,1);//new u8[size];
	if (buf->buf_base==NULL)
	{
		CRIT_ERROR;
		return ERROR;
	}

	memset(buf->buf_base,0,size);
	buf->len=0;
	buf->base_len=0;
	buf->max_size=size;
	buf->free_size=size;
	buf->buf=buf->buf_base;
	return NO_ERROR;
}

eErrorTp IncreaseBufSize(BufHandler * buf,u32 size)
{
	if (buf->buf_base==NULL)
	{
		CRIT_ERROR;
		return ERROR;
	}

	u32 buf_offs=(u32)((size_t)buf->buf-(size_t)buf->buf_base);
	u8 * ptr=NULL;

	ptr=(u8*)realloc((char*)buf->buf_base,buf->max_size+size);

	if (ptr==NULL)
	{
		CRIT_ERROR;
		return ERROR;
	}

	buf->buf_base=ptr;

	//buf->len=0;
	//buf->base_len=0;

	buf->free_size+=size;
	buf->max_size+=size;

	buf->buf=&buf->buf_base[buf_offs];

	return NO_ERROR;
}

eErrorTp InitBuf(BufHandler * buf)
{
	buf->base_len=0;
	buf->buf=NULL;
	buf->buf_base=NULL;
	buf->free_size=0;
	buf->len=0;
	buf->max_size=0;
	return NO_ERROR;
}

eErrorTp ClearBuf(BufHandler * buf)
{
	buf->base_len=0;
	buf->buf=buf->buf_base;
	buf->free_size=buf->max_size;
	buf->len=0;
	return NO_ERROR;
}

eErrorTp BufToBufHandler(BufHandler * dstbuf, u8 * sbuf,u32 sbuf_len, u32 maxsize)
{
	if (sbuf==NULL)
	{
		CRIT_ERROR;
		return ERROR;
	}
	dstbuf->free_size=maxsize-sbuf_len;
	dstbuf->buf_base=sbuf;
	dstbuf->len=sbuf_len;
	dstbuf->base_len=sbuf_len;
	dstbuf->max_size=maxsize;
	dstbuf->buf=dstbuf->buf_base;
	return NO_ERROR;
}

eErrorTp TestIsFreeBuf(BufHandler * buf)
{
	if (buf->buf_base!=NULL) return ERROR;
	return NO_ERROR;
}

eErrorTp FreeBuf(BufHandler * buf)
{
	if (buf->buf_base==NULL)
		return ERROR;

	free(buf->buf_base);//delete [] buf->buf_base;
	buf->buf_base=NULL;
	buf->base_len=0;
	buf->len=0;
	buf->free_size=0;
	return NO_ERROR;
}

eErrorTp AddZeroToBuf(BufHandler * BufH)
{
	BufH->buf[0]=0;
	SafeIncTotalLenBuf(BufH,1);
	SafeIncPointerBuf(BufH,1);
	return NO_ERROR;
}

eErrorTp AddCStringToBuf(BufHandler * BufH)
{
	u32 len;
	BufHandler tBufH;
	len=(u32)strlen((char*)BufH->buf);
	BufClone(BufH,&tBufH);
	if (SafeIncTotalLenBuf(BufH,len)==ERROR)
	{
		//printf("Error SafeIncTotalLenBuf\n");
		BufClone(&tBufH,BufH);
		return ERROR;
	}

	if (SafeIncPointerBuf(BufH,len)==ERROR)
	{
		//printf("Error SafeIncPointerBuf\n");
		BufClone(&tBufH,BufH);
		return ERROR;
	}

	return NO_ERROR;
}

fifo_element::fifo_element(void) noexcept
	{
		bufsize=0;
		buf=NULL;
		//printf("Create element\n");
	}

u8 * fifo_element::GetBuf(u32 & size)
	{
		size=bufsize;
		// printf("get srcbufsize %d\n",size);
		return buf;
	}


eErrorTp fifo_element::SetBuf(u8 * srcbuf,u32 size)
	{
	  try
	  {
		if (buf==NULL)
			buf=new u8 [size];
		else
		{
			delete [] buf;
			buf=new u8 [size];
		}
	  }
	  catch(const std::bad_alloc& e)
	  {
		  buf=NULL;
		  bufsize=0;
		  return ERROR;
	  }

	  if (buf==NULL)
	  {
		  bufsize=0;
		  return ERROR;
	  }

	  bufsize=size;
	  memcpy(buf,srcbuf,bufsize);
	//  printf("set srcbufsize %d\n",bufsize);
	  return NO_ERROR;
	}

fifo_element::~fifo_element()
	{
		if (buf!=NULL)
		{
			delete [] buf;
		}
		bufsize=0;
		//printf("Del fifo_element\n");
	}


fragfifo::fragfifo()
		{
			//fifo=new list<fifo_element>;
			//fifo=make_shared<list<fifo_element>>();
			qcount=0;
			qcount_max_bsize=QCOUNT_MAX_SIZE;
			qcount_bsize=0;
			fifo.emplace_back();//!!!Do not delete, it s correct!!!
			mutex_init(FifoMutex);
			//printf("Create fragfifo 0x%8x\n",fifo);
		}
fragfifo::~fragfifo()
		{
			mutex_unlock(FifoMutex);
			fifo.clear();
			//delete fifo;
		}

eErrorTp fragfifo::PushBuf(u8 * buf, u32 size)
		{
			fifo_element * pfe;
			list<fifo_element>::iterator it;
			if ((qcount_bsize+size)>qcount_max_bsize)
				return ERROR;
			mutex_lock(FifoMutex);
			fifo.emplace_back();
			it=fifo.end();

			it--;
			pfe=&*it;
			pfe->SetBuf(buf,size);
			qcount++;
			//printf("a1 qcount %d fifo 0x%8x\n",qcount,fifo);
			qcount_bsize+=size;
			mutex_unlock(FifoMutex);

			return NO_ERROR;
		}

u8 * fragfifo::GetLastBuf(u32 & size)
		{
			fifo_element * pfe;
			list<fifo_element>::iterator it;
			u8 * buf;

			//printf("d0 qcount %d fifo 0x%8x\n",qcount,fifo);
			if (qcount<=0)
				return NULL;

			mutex_lock(FifoMutex);
			//printf("d1 qcount %d fifo 0x%8x\n",qcount,fifo);
			fifo.pop_front();
			//printf("d2\n");
			it=fifo.begin();
			pfe=&*it;
			qcount--;

			buf=pfe->GetBuf(size);

			qcount_bsize-=size;
			mutex_unlock(FifoMutex);

			//printf("d1 qcount %d fifo 0x%8x size %d qcount_bsize %d\n",qcount,fifo,size,qcount_bsize);
			if (qcount_bsize<0)
				return NULL;


			return buf;
		}

int fragfifo::GetCountElements()
		{
			return qcount;
		}

int fragfifo::GetTotalSize()
		{
			return GetTotalSizeLastElements(qcount);
		}

int fragfifo::GetTotalSizeLastElements(int countElements)
		{
			u32 size=0;
			u32 getsz=0;
			list<fifo_element>::iterator it;

			mutex_lock(FifoMutex);

			///printf("qc %d cE %d\n",qcount,countElements);
			if (qcount<countElements)
			{
				mutex_unlock(FifoMutex);
				return 0;
			}

			int n=0;
			it=fifo.begin();
			//it++;
			while(it!=fifo.end())
			{
				n++;
				size+=it->bufsize;
				if (n>=countElements) break;
				it++;
			}

			mutex_unlock(FifoMutex);
			return size;
		}
