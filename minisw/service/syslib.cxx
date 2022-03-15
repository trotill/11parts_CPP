/*
 * syslib.cxx
 *
 *  Created on: 3 сент. 2020 г.
 *      Author: root
 */


#include "syslib.h"

int proclink_stdout[2]={0,0};
int proclink_stderr[2]={0,0};

char **split_commandline(const char *cmdline, int *argc)
{
    char **argv = NULL;
    assert(argc);

    if (!cmdline)
    {
        return NULL;
    }

	wordexp_t p;

	if (wordexp(cmdline, &p, 0))
	{
		return NULL;
	}

	*argc = p.we_wordc;

    try{
        if (!(argv = (char**)calloc(*argc+1, sizeof(char *)))){
        	throw 0;
        }

        for (size_t i = 0; i < p.we_wordc; i++){
            if (!(argv[i] = strdup(p.we_wordv[i])))
            {
            	throw 1;
            }
        }

        wordfree(&p);

        return argv;
    }
    catch(int e){
		wordfree(&p);
		if (argv)
		{
			for (int i = 0; i < *argc; i++)
			{
				if (argv[i])
				{
					free(argv[i]);
				}
			}

			free(argv);
		}
    }
    return NULL;
}

eErrorTp Shedule(string prog,string arg) {
	char **argv;
	int argc=0;
	//string prog_fn=basename(prog.c_str());
	arg=sf("%s %s",prog.c_str(),arg.c_str());
	printf("svc: run shedule %s %s\n",prog.c_str(),arg.c_str());
	argv = split_commandline((char*)arg.c_str(),&argc);
	execvp(prog.c_str(),argv);
	return NO_ERROR;
}


eErrorTp Exe(pid_t & pID,string prg,string args){
	//pID=-1;
	//sl_pID=-1;

	pipe(proclink_stdout);
	pipe(proclink_stderr);

	pID=fork();

	if (pID==-1){
		printf("error fork process svc with arg %s\n",(char*)prg.c_str());
		exit(-1);
	}
	if (pID==0){

		dup2 (proclink_stdout[1], STDOUT_FILENO);
		dup2 (proclink_stderr[1], STDERR_FILENO);
		close(proclink_stdout[0]);
		close(proclink_stdout[1]);
		close(proclink_stderr[0]);
		close(proclink_stderr[1]);
		fcntl(STDOUT_FILENO, F_SETFL, O_NONBLOCK);
		fcntl(STDERR_FILENO, F_SETFL, O_NONBLOCK);


		Shedule(prg,args);
		printf("svc: terminate shedule\n");
		printf("terminate shedule pid %u\n",pID);
		exit(1);
	}
	else{

		close(proclink_stdout[1]);
		close(proclink_stderr[1]);
		fcntl(proclink_stdout[0], F_SETFL, O_NONBLOCK);
		fcntl(proclink_stderr[0], F_SETFL, O_NONBLOCK);
	}

	return NO_ERROR;
}
