/*
 * 11p_time.cxx
 *
 *  Created on: 27 дек. 2018 г.
 *      Author: root
 */

#include "11p_time.h"
#include "11p_string.h"

eErrorTp SetTime(struct tm * timeinfo)
{
	TIME_T tt=(TIME_T)mktime(timeinfo);
	struct timeval tv;
	tv.tv_sec=tt;
	tv.tv_usec=0;
	if (settimeofday(&tv,NULL)!=0) return ERROR;
	else
		return NO_ERROR;
}

eErrorTp GetDiffHiRes(sdTime & Time, int & diffuSec)
{
	int dSec=Time.NextTime.tv_sec-Time.LastTime.tv_sec;
	int duSec=Time.NextTime.tv_usec+(dSec*1000000)-Time.LastTime.tv_usec;
	diffuSec=0;
	if ((dSec>60)|(dSec<0)) return ERROR;
	diffuSec=duSec;
	return NO_ERROR;
}

u64 getSourceTimeHiRes_uS()
{
	struct timeval stime;
	gettimeofday(&stime,NULL);

	return stime.tv_usec+(stime.tv_sec*1000000);
}

u64 getSourceTimeHiRes_mS()
{
	struct timeval stime;
	gettimeofday(&stime,NULL);

	//u64 sec=(u64)stime.tv_sec*1000;
	//cout << "sec " << sec << "sizeof(u64)" << sizeof(u64) << endl;
	//printf("stime.tv_sec %llu stime.tv_sec %u stime.tv_usec %lu\n",sec,stime.tv_sec,stime.tv_usec/1000);
	return (stime.tv_usec/1000)+((u64)stime.tv_sec*1000);
}


void GetDataTime(struct tm * times,struct timeval * tv)
{
	TIME_T rawtime;
	struct tm * timeinfo;

	if (tv!=NULL)
		gettimeofday(tv,NULL);
	TIME (&rawtime);

	LOCALTIME (&rawtime,times);

}

string TimeFormat(TIME_T  t,u8 var)
{
	//struct tm * ti=LOCALTIME(&t);
	struct tm ti;
	LOCALTIME(&t,&ti);

	string res;
	if (ti.tm_year==70)
		return "---";

	if (var==0)
		return sf("%04d-%02d-%02d %02d:%02d:%02d",ti.tm_year+1900,ti.tm_mon+1,ti.tm_mday,ti.tm_hour,ti.tm_min,ti.tm_sec);
	if (var==1)
		return sf("%04d-%02d-%02d",ti.tm_year+1900,ti.tm_mon+1,ti.tm_mday);

	return sf("%04d-%02d-%02d %02d:%02d",ti.tm_year+1900,ti.tm_mon+1,ti.tm_mday,ti.tm_hour,ti.tm_min);

}


eErrorTp TimeToString(struct tm & timeinfo, stringstream & strdate)
{
	strdate << setw(4) << (timeinfo.tm_year+1900) << '-'
		<< setfill ('0') << setw(2) << (timeinfo.tm_mon+1) << '-'
		<< setfill ('0') << setw(2) << timeinfo.tm_mday << 'T'
		<< setfill ('0') << setw(2) << timeinfo.tm_hour << ':'
		<< setfill ('0') << setw(2) << timeinfo.tm_min << ':'
		<< setfill ('0') << setw(2) << timeinfo.tm_sec << 'Z';

	return NO_ERROR;
}

TIME_T TIME(){
	TIME_T t=NULL;
	return TIME(&t);
}

time_t getLocalTimeStamp(void){
    // Возвращает таймстам с учетом часового пояса
    time_t timeV=time(nullptr);
    std::tm* _tm=localtime((time_t*)&timeV);
    return timeV+_tm->tm_gmtoff;
}