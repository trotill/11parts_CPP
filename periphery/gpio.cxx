/*
 * gpio.cxx
 *
 *  Created on: 30 июля 2014 г.
 *      Author: root
 */

#include "gpio.h"

eErrorTp GpioInInit(u8 gpio)
{
	string str;
	str= "echo \"" + intToString(gpio) + "\"> /sys/class/gpio/export";
	printf("%s\n",str.c_str());
	system(str.c_str());

	str= "echo \"in\" > /sys/class/gpio/gpio"+intToString(gpio)+"/direction";
	printf("%s\n",str.c_str());
	system(str.c_str());

	return NO_ERROR;
}

eErrorTp GpioOutInit(u8 gpio, u8 defval)
{
	string str;
	str= "echo \"" + intToString(gpio) + "\"> /sys/class/gpio/export";
	system(str.c_str());

	str= "echo \""+ intToString(defval) +"\" > /sys/class/gpio/gpio"+intToString(gpio)+"/value";
	system(str.c_str());

	str= "echo \"out\" > /sys/class/gpio/gpio"+intToString(gpio)+"/direction";
	system(str.c_str());

	return NO_ERROR;
}

u8 GpioInRead(u8 gpio)
{
	string res=ExecResult("cat",(char*)string("/sys/class/gpio/gpio"+intToString(gpio)+"/value").c_str());
	//printf("res %s\n",res.c_str());
	if (res=="1\n")
		return 1;
	else
		return 0;
}

eErrorTp GpioOnOff(u8 gpio,u8 val)
{
	string str;

	//if (val)
		//val=(~val)&0x1;

	str= "echo "+ intToString(val) +" > /sys/class/gpio/gpio"+intToString(gpio)+"/value";
	system(str.c_str());
	printf("%s\n",str.c_str());
	return NO_ERROR;
}

gpio::gpio(string num,string direct,string value){
		ngpio=num;
		int  fd = open("/sys/class/gpio/export", O_WRONLY);
		write(fd, num.c_str(), num.size());
		close(fd);

		access=O_RDONLY;
		if (direct=="out"){
			access=O_WRONLY;
		}

			fd_val = open(string_format("/sys/class/gpio/gpio%s/value",num.c_str()).c_str(), access);

			//write(fd_val, value.c_str(), value.size());
			printf("open %s\n",string_format("/sys/class/gpio/gpio%s/value",num.c_str()).c_str());
			//printf("Write %s size %d\n",value.c_str(),value.size());

			fd_dir = open(string_format("/sys/class/gpio/gpio%s/direction",num.c_str()).c_str(), O_WRONLY);

			write(fd_dir, direct.c_str(), direct.size());
			if (direct=="out")
				set((char*)value.c_str());

			//set("1");
			//printf("Write %s size %d\n",value.c_str(),value.size());


			//sleep(5);


		//int  fd = open("/sys/class/gpio/gpio23/value", O_WRONLY);
	}

gpio::~gpio(void){
		close(fd_val);
		close(fd_dir);
}

eErrorTp gpio::set(char * val){


		write(fd_val, val, 1);
		return NO_ERROR;
}

u32 gpio::get(void){

		char buf[2]={0};

		u32 len=read(fd_val, buf, 1);
		//printf("get %d %d len %d\n",buf[0],buf[1],len);
		close(fd_val);
		fd_val = open(string_format("/sys/class/gpio/gpio%s/value",ngpio.c_str()).c_str(), access);

		return atoi(buf);
}
