/*
 * 11p_time.h
 *
 *  Created on: 27 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_LIB_11P_TIME_H_
#define SRC_ENGINE_LIB_11P_TIME_H_

#include "engine/basic.h"

#define LOOPTIMER_TIMER_MODE 0//после аларм, авт. сбрасывается и ждет следующего
#define LOOPTIMER_TIMEOUT_MODE 1//после аларм, не сбрасывается и ждет сброса и всегда не срабатывает аларм
#define LOOPTIMER_TIMEOUT_MODE2 2//после аларм, не сбрасывается, ждет сброса и всегда срабатывает аларм

typedef struct sdTime
{
	struct timeval LastTime;
	struct timeval NextTime;
	u32 uSecCompens;
}sdTime;

eErrorTp SetTime(struct tm * timeinfo);
eErrorTp GetDiffHiRes(sdTime & Time, int & diffuSec);
void GetDataTime(struct tm * times,struct timeval * tv);
eErrorTp TimeToString(struct tm & timeinfo, stringstream & strdate);
string TimeFormat(TIME_T t,u8 var);
u64 getSourceTimeHiRes_uS();
u64 getSourceTimeHiRes_mS();
TIME_T TIME();
time_t getLocalTimeStamp(void);

class profiler {
	public:
		profiler(string name){
			gettimeofday(&Time.LastTime,NULL);
			this->name=name;
		}
		profiler(void){
			gettimeofday(&Time.LastTime,NULL);
			this->name="profiler";
		}
		u32 prof(void){
			int diffuSec;
			gettimeofday(&Time.NextTime,NULL);
			GetDiffHiRes(Time,diffuSec);
			Time.LastTime=Time.NextTime;
			return diffuSec;
		}
		u32 prof_show(void){
			u32 prf=prof();
			printf("$%s$ %d us\n",name.c_str(),prf);
			return prf;
		}
	private:
		sdTime Time;
		string name;

};

class LoopTimer{
	public:
	LoopTimer(u32 timeout_ms,u32 loopdelay_ms):timeout_ms(timeout_ms),loopdelay_ms(loopdelay_ms){
		if (loopdelay_ms>=timeout_ms)
			timeout_ms=loopdelay_ms;
	}
	LoopTimer(u32 timeout_ms,u32 loopdelay_ms,u8 mode):timeout_ms(timeout_ms),loopdelay_ms(loopdelay_ms),mode(mode){
			if (loopdelay_ms>=timeout_ms)
				timeout_ms=loopdelay_ms;
		}
	bool alarm(){
		bool res=false;
		if (counter>=timeout_ms){
			res=true;
			if (mode==LOOPTIMER_TIMER_MODE)
				counter=0;
		}
		else
		{
			counter+=loopdelay_ms;
		}
		return res;
	}
	void reset(){
		counter=0;
	}
	private:
	u32 timeout_ms=1000;
	u32 loopdelay_ms=100;
	u32 counter=0;
	u8 mode=LOOPTIMER_TIMER_MODE;
};

class RTC_Timer{
	public:

	RTC_Timer(u64 interv_ms=0xffffffffffffffff,u8 mode=LOOPTIMER_TIMER_MODE){
		set(interv_ms,mode);
	}
	void reset(){

		waittime=getSourceTimeHiRes_mS()+interval;
		block=false;
		//printf("Set waittime %lu\n",waittime);
	}
	//set is obsolete!!!!
	void set(u64 interv_ms,u8 mode=LOOPTIMER_TIMER_MODE){
		this->mode=mode;
		interval=interv_ms;
		reset();
	}
	void setInMSec(u64 interv,u8 mode=LOOPTIMER_TIMER_MODE){
		set(interv,mode);
	}
	void setInSec(u32 interv,u8 mode=LOOPTIMER_TIMER_MODE){
		set(interv*1000,mode);
		//this->mode=mode;
		//interval=interv*1000;
		//reset();
	}

	bool alarm(){
		u64 newtime=getSourceTimeHiRes_mS();
		if ((waittime!=0)&&(waittime>(interval*2))&&((newtime<(waittime-(interval*2))))){
			//corript time
			printf("corrupt time src %llu old %llu interval %llu\n",newtime,waittime,interval);
			waittime=newtime;

		}
		if ((newtime>=waittime)){

			if (!block){
				//printf("newtime %llu waittime %llu interval %llu\n",newtime,waittime,interval);
				waittime=newtime+interval;
				if (mode==LOOPTIMER_TIMEOUT_MODE)
					block=true;
				if (mode==LOOPTIMER_TIMEOUT_MODE2)
					block=true;
				return true;
			}
			else{
				if (mode==LOOPTIMER_TIMEOUT_MODE2){
					return true;
				}
				return false;
			}
		}
		else{
			return false;
		}

	}
	private:
	 u64 waittime=0;
	 u64 interval=0;
	 u8 mode=LOOPTIMER_TIMER_MODE;
	 bool block=false;
};

#endif /* SRC_ENGINE_LIB_11P_TIME_H_ */
