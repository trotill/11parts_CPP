/*
 * thread.cxx
 *
 *  Created on: 27 дек. 2018 г.
 *      Author: root
 */

#include "engine/basic.h"


eErrorTp StartPthread(Thread_t * thr,void *(*__start_routine) (void *),void * X){

	pthread_attr_t attr;
	eErrorTp err=NO_ERROR;
	//printf("mr\n");
	if (*thr==0)
	 {
		//printf("m0\n");
		 pthread_attr_init(&attr);
		 pthread_attr_setstacksize(&attr,1000000);
		 pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		// printf("m1\n");
		 if (pthread_create((pthread_t*)thr,&attr,__start_routine,X)!=0)
			err=ERROR;
		// printf("m2\n");
		 pthread_attr_destroy(&attr);
	 }
	 else
		err=ERROR;

	 return err;
}

eErrorTp StopPthread(Thread_t * thr){
	int err=0;
	if (*thr==0) return ERROR;

	err=pthread_cancel(*thr);

	if(!pthread_equal(pthread_self(),*thr)){
			err|=pthread_join(*thr,NULL);
		}

	*thr=0;
	if ((err==0)|(err==ESRCH))
		return NO_ERROR;

	return ERROR;
}
