/*
 * cnodakg.cxx
 *
 *  Created on: 17 февр. 2021 г.
 *      Author: root
 */

#include "engine/minisw/necron/cnodakg.h"
#include "engine/periphery/skeyV2.h"
#if 0
void printhex(u8 * buf,u32 len,u16 loop)
{
 u32 i;
 u16 k=0;
 u8 buf2[10];
 if (len<loop) loop=len;
 cout << "-S--------------------------------------------------" << endl;
 for (i=0;i<len;i++){
	 snprintf((char*)buf2,10,"[0x%02x]",buf[i]);
	cout << buf2;
	k++;
	if (k==loop) {
	  cout << endl;
	  k=0;
	}
 }
 if (k!=0) cout << endl;
 cout << "-F--------------------------------------------------" << endl;

}


std::string sf(const std::string fmt, ...) {
    int size = 100;
    std::string str;
    va_list ap;
    while (1) {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {
            str.resize(n);
            return str;
        }
        if (n > -1)
            size = n + 1;
        else
            size *= 2;
    }
    return str;
}
#endif
void SetStdinEcho(bool enable = true)
{
#ifdef WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);

    if( !enable )
        mode &= ~ENABLE_ECHO_INPUT;
    else
        mode |= ENABLE_ECHO_INPUT;

    SetConsoleMode(hStdin, mode );

#else
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if( !enable )
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    (void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

int main(int argc, char *argv[])
{
	//if (argc!=2){
	//	printf("error input\n");
	//	exit(1);
	//}
	//string uid=argv[1];
	//string pass=argv[2];
#ifdef _SECURE_ENABLE
	string uid;
	string pass;
	epsec::secure sec;
	string key;

	//testSkey();
	if (argc==3){
		string uidA=argv[2];
		string passA=argv[1];
		sleep(1);
		if (uidA.size()<32)
			exit(1);

		if (passA!=_SECURE_SNP){
			exit(1);
		}

		mdelay(500);
		sec.logPull();
		key=sec.genKey(uidA,passA);
		cout << key << endl;
		sec.logPush();
		exit(0);
	}

	printf("Input password\n");
	 SetStdinEcho(false);
	cin >> pass;
	if (pass!=_SECURE_SNP){
		sleep(1);
		SetStdinEcho(true);
		exit(1);
	}
	SetStdinEcho(true);
	while(1){
		//printf("Input uid\n");
		mdelay(700);
		cin >> uid;
		sleep(1);
		if (uid.size()<32)
			continue;
		//printf("uid %s pass %s\n",uid.c_str(),pass.c_str());
		sec.logPull();
		key=sec.genKey(uid,pass);
		sec.logPush();
		cout << key << endl;
		sleep(1);
		//printf("%s",key.c_str());
	}
#endif
}

