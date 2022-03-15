/*
 * input_event.h
 *
 *  Created on: 6 дек. 2019 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_PERIPHERY_INPUT_EVENT_H_
#define SRC_ENGINE_PERIPHERY_INPUT_EVENT_H_

#include "engine/print.h"

class KeyboardT {
	public:
   KeyboardT(string dev,GprintT * g):dev(dev),g(g){
	   if( (evdev_fd = open(dev.c_str(), O_RDWR|O_NONBLOCK)) < 0 )
	   {
	   		g->GPRINT(NORMAL_LEVEL,"error, not open %s \n",dev.c_str());
	   }
	   init_ok=true;
   }
   ~KeyboardT(){
	   if (evdev_fd!=0)
		   close(evdev_fd);
   }
   eErrorTp get(TIME_T & time,u16 & type,u16 & code,int & value){
	   if (read(evdev_fd, &event, sizeof(struct input_event))>0){
		   time=event.time.tv_sec;
		   type=event.type;
		   code=event.code;
		   value=event.value;
		  // printf("type %d code %d value %d\n",type,code,value);
		   return NO_ERROR;
	   }
	   return ERROR;
   }
   eErrorTp get_key(u16 & code,int & value)
   {
	   u16 type;
	   TIME_T time;
	   if (get(time,type,code,value)!=ERROR){
		   if (type==EV_KEY)
			   return NO_ERROR;
	   }
	   return ERROR;
   }
   eErrorTp clean(void){
   		while(read(evdev_fd, &event, sizeof(struct input_event))>0) {
   		}
   		return NO_ERROR;
   }

   private:
   string dev;
   GprintT * g;
   struct input_event event;
   bool init_ok=false;
   int evdev_fd=0;
};


#endif /* SRC_ENGINE_PERIPHERY_INPUT_EVENT_H_ */
