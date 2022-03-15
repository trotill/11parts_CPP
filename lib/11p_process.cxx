/*
 * 11p_process.cxx
 *
 *  Created on: 26 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_LIB_11P_PROCESS_CXX_
#define SRC_ENGINE_LIB_11P_PROCESS_CXX_
#include "engine/basic.h"
#include "engine/lib/11p_bin.h"
#include "engine/global.h"

int SearchProcess(const char* nameWPath)
{
    DIR* dir;
    FILE* fp;
    string name=basename(nameWPath);
    struct dirent* ent;
    char* endptr;
    char buf[512];
    int proc_found = 0;

    if (!(dir = opendir("/proc"))) {
        perror("can't open /proc");
        return (pid_t) -2;
    }


    while((ent = readdir(dir)) != NULL) {
        /* if endptr is not a null character, the directory is not
         * entirely numeric, so ignore it */
        long lpid = strtol(ent->d_name, &endptr, 10);
        if (*endptr != '\0') {
            continue;
        }

        /* try to open the cmdline file */
        snprintf(buf, sizeof(buf), "/proc/%ld/cmdline", lpid);

        fp = fopen(buf, "r");


        if (fp) {
            if (fgets(buf, sizeof(buf), fp) != NULL) {
                /* check the first token in the file, the program name */
                char* first = strtok(buf, " ");
                char * cmdLinefName=basename(first);
                //printf("[%s]/[%s]\n",cmdLinefName,name);
                if (name==cmdLinefName) {
                	//printf("[%s]/[%s]\n",cmdLinefName,name);
                    ++proc_found;
                }
            }

            fclose(fp);
        }

    }

    closedir(dir);
    return proc_found;
}

eErrorTp KillProcess(char * process,char * killstr,u32 time_ms)
{
	u32 n=0;
	while (SearchProcess(process)!=0)
	{
		//printf("!!!!!!while (SearchProcess('node')!=0)\n");
		system(killstr);
		mdelay(time_ms);
		n++;
		if (n==20)
		{
			printf("Error:Process %s not killed\n",process);
			return ERROR;
		}
		//printf("while (SearchProcess('node')!=0)\n");
	}
	return NO_ERROR;
}

string ShellResult(char* cmd) {

	    std::string result = "";

	   // printf("Run: %s\n",buffer);
	   buffer buf(4096);
	   FILE* pipe = popen(cmd, "r");
	    //printf("pipe: 0x%08x %s\n",pipe,buffer);
	    if (!pipe)
	        printf("popen(%s) failed!\n",cmd);
	    else{
			try {
				//while (!feof(pipe)) {
				if ((!feof(pipe))&&(fgets((char*)buf.p(), buf.size(), pipe) != NULL))
				    result = (char*)buf.p();
			   // }
			}
			catch (...) {
			    printf("catch popen error %s\n",cmd);
			}
			pclose(pipe);
	    }

	    return result;
}
string BashResult(string cmd) {
	buffer buf(1000);
	//std::unique_ptr<char[]>buffer(new tmp_str_size());
	if (cmd.size()>=1000)
		return "";

	//strcpy((char*)buf.p(),(char*)cmd.c_str());
    //snprintf(buffer,sizeof(buffer),"%s %s",cmd,args);

	//printf("BashResult %s\n",(char*)buf.p());
	return ShellResult((char*)cmd.c_str());
}
string ExecResult(char* cmd,char* args) {
    //buffer cmd(1000);
	//std::unique_ptr<char[]>buffer(new tmp_str_size());

	//snprintf((char*)cmd->p(),cmd->size(),"%s %s",cmd,args);

	string cmdArg=string(cmd)+" "+string(args);
	return ShellResult((char*)cmdArg.c_str());
}

u16 GetProgVersion(void) {
    char s_month[5];
    int month, day, year;
    u16 version;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

    sscanf(__DATE__, "%s %d %d", s_month, &day, &year);

    month = (strstr(month_names, s_month)-month_names)/3;
    version=(year-2000)<<9|(month<<5)|(day);

    return version;
}

#define READ   0
#define WRITE  1
FILE * popen2(string command, string type, int & pid)
{
    pid_t child_pid;
    int fd[2];
    pipe(fd);

    if((child_pid = fork()) == -1)
    {
        perror("fork");
        exit(1);
    }

    /* child process */
    if (child_pid == 0)
    {
        if (type == "r")
        {
            close(fd[READ]);    //Close the READ end of the pipe since the child's fd is write-only
            dup2(fd[WRITE], 1); //Redirect stdout to pipe
        }
        else
        {
            close(fd[WRITE]);    //Close the WRITE end of the pipe since the child's fd is read-only
            dup2(fd[READ], 0);   //Redirect stdin to pipe
        }

        setpgid(child_pid, child_pid); //Needed so negative PIDs can kill children of /bin/sh
        //execl(command.c_str(),command.c_str(),)
       // system(command.c_str());
        execl("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL);
        exit(0);
    }
    else
    {
        if (type == "r")
        {
            close(fd[WRITE]); //Close the WRITE end of the pipe since parent's fd is read-only
        }
        else
        {
            close(fd[READ]); //Close the READ end of the pipe since parent's fd is write-only
        }
    }

    pid = child_pid;

    if (type == "r")
    {
        return fdopen(fd[READ], "r");
    }

    return fdopen(fd[WRITE], "w");
}

int pclose2(FILE * fp, pid_t pid)
{
    int stat;

    fclose(fp);
    while (waitpid(pid, &stat, 0) == -1)
    {
        if (errno != EINTR)
        {
            stat = -1;
            break;
        }
    }

    return stat;
}

void RebootSystem(void){
	system("reboot");
#ifndef _WO_CNT_DEP
	CnT->reboot_state=true;
#endif
}


#endif /* SRC_ENGINE_LIB_11P_PROCESS_CXX_ */
