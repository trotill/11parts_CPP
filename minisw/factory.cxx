/*
 * factory.cxx
 *
 *  Created on: 18 дек. 2018 г.
 *      Author: root
 */

#include "custom_project/custom_project.h"

void run_cmd(string cmd){
	printf("Input CMD [%s]\n",cmd.c_str());
	if (cmd=="factory")
		ResetDeviceToFactory();
	if (cmd=="iptables")
		system("/usr/sbin/iptables -v -n -L");
	if (cmd=="top")
		system("/usr/bin/top");
	if (cmd=="hwclock")
		system("/sbin/hwclock");
	if (cmd=="date")
		system("/bin/date");
	if (cmd=="ifconfig")
		system("/sbin/ifconfig -a");
	if (cmd=="route")
		system("/sbin/route -n");
	if (cmd=="meminfo")
		system("cat /proc/meminfo");
	if (cmd=="cpuinfo")
		system("cat /proc/cpuinfo");
	if (cmd=="cpuinfo_cur_freq")
		system("cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq");
	if (cmd=="cpuinfo_max_freq")
		system("cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
	if (cmd=="reboot")
		system("reboot");
	if (cmd=="help")
		printf("CMD list:\n"
				"	reboot\n"
				"	factory\n"
				"	iptables\n"
				"	top\n"
				"	hwclock\n"
				"	date\n"
				"	ifconfig\n"
				"	route\n"
				"	meminfo\n"
				"	cpuinfo\n"
				"	cpuinfo_cur_freq\n"
				"	cpuinfo_max_freq\n");
}

int to_factory(int argc, char **argv)
{
	//printf("For roll back device to factory write - 'factory', and press enter\n");
	string cmd;
	if (argc>1){
		run_cmd(argv[1]);
		//if (strcmp(argv[1],"factory")
	}
	while(1){
		printf("For roll back device to factory write - 'factory', and press enter\n");
		cin >> cmd;
		run_cmd(cmd);

#ifdef _GORCHAKOV
#warning("Gorchakov support")
		if (cmd=="gorchakov")
			system("/bin/sh");
#endif
	}
	return 0;
}

