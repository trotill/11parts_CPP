/*
 * thread.cxx
 *
 *  Created on: 15 янв. 2019 г.
 *      Author: root
 */

#include "thread.h"


eErrorTp Thread_baseT::Run(void)
	    {

	    	TERMState=ACTIVE_STATE_RISE;
	    	StartPthreadC(&thr, &Thread_baseT::ThrWrapper,(void *)this);
	    	return NO_ERROR;
	    }
eErrorTp Thread_baseT::Stop(void)
	    {

	    	//TERMState=ACTIVE_STATE_RISE;
			StopPthreadC(&thr);
	    	return NO_ERROR;
	    }
Thread_baseT::Thread_baseT(void){
			//SrvType=srvtInterplay;
			LoopServer = std::bind(&Thread_baseT::Loop, this, std::placeholders::_1);
			InitBuf(&FifoPreBuf);
			AllocBuf(&FifoPreBuf,FIFO_PREBUF_LEN);
			GPRINT(NORMAL_LEVEL,"Create Thread_baseT\n");
		}
Thread_baseT::~Thread_baseT(void)
		{
			RunServerQState=QUIT;
			FreeBuf(&FifoPreBuf);
			TERMState=TERM_STATE_RISE;
			GPRINT(NORMAL_LEVEL,"Delete Thread_baseT\n");
		};

eErrorTp Thread_baseT::StopPthreadC(Thread_t * thr)
		{

			int tcopy= (int&)*thr;

			if (StopPthread(thr)==ERROR)
			{
			 	 GPRINT(NORMAL_LEVEL,"Not cancelled pthread [%x]\n",tcopy);
			 	 return ERROR;
			}
			else
			{
				GPRINT(NORMAL_LEVEL,"Cancelled pthread [%x]\n",tcopy);
			    return NO_ERROR;
			}
		}

eErrorTp Thread_baseT::StartPthreadC(Thread_t * thr,void *(*__start_routine) (void *),void * X)
		{
					 if (StartPthread(thr,__start_routine,X)==ERROR)
						 {
						 	 GPRINT(NORMAL_LEVEL,"Not created pthread [%x]\n",*thr);
						 	 return ERROR;
						 }
					 else
					 {
						 GPRINT(NORMAL_LEVEL,"Created pthread [%x]\n",*thr);
						 return NO_ERROR;
					 }

		}

