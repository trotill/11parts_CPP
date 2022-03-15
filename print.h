/*
 * print.h
 *
 *  Created on: 17 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_PRINT_H_
#define SRC_ENGINE_PRINT_H_
#include "basic.h"




class GprintT{
	public:
		u32 GPRINT(eDebugTp level,char * format, ...);
		void WriteToFile(char * data){
			//zr();
			if (debug_level>=FILE_LEVEL){
				//zr();
				if (OutFile.size()==0){
					OutFile="/var/run/gptlog."+SourceStr+"."+=ObjPref;

					fout.open(OutFile);
				}
				//printf("OutFile %s\n",OutFile.c_str());
				fout<<data;
			}
		}
		~GprintT(){
			if (OutFile.size()!=0){
				//printf("CLOSE\n");
				fout.close();
			}
		}
		eDebugTp debug_level=NORMAL_LEVEL;
		string SourceStr="";
		string ObjPref="";

	private:
		string OutFile="";
		ofstream fout;
		//string multicast_addr="239.100.200.1";
		//u16 multicast_port=10000;
};



#endif /* SRC_ENGINE_PRINT_H_ */
