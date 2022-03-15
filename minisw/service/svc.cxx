/*
 * svc.cxx
 *
 *  Created on: 29 нояб. 2019 г.
 *      Author: root
 */
#include "svc.h"
#include <fcntl.h>


Mutex_t CmdMutex;
string command;
string prog_name;
bool do_exit=false;
//bool exit_svc=false;
TIME_T crit_diff_time=20;


bool run_killer=false;
bool exit_worker=false;
bool write_log=false;
bool worker_exited=false;
bool stop_worker=true;
bool worker_stoped=true;
pid_t child_pid=-1;
pid_t slogger_pid=-1;
Mutex_t WorkerMutex;
Mutex_t WrLogMutex;
int log_wr_fd=0;
string write_log_fn="";
bool debug=false;

eErrorTp open_log_pipe(char * pipe_name){

	if (strlen(pipe_name)==0)
		return ERROR;

	if ((existsSyncPipe(pipe_name)==ERROR)||(log_wr_fd!=0))
		return ERROR;

	mutex_lock(WrLogMutex);

	log_wr_fd=open(pipe_name,O_WRONLY|O_NONBLOCK);

	if (log_wr_fd<=0){
		log_wr_fd=0;

		mutex_unlock(WrLogMutex);
		return ERROR;
	}

	mutex_unlock(WrLogMutex);
	return NO_ERROR;
}
eErrorTp close_log_pipe(){

	if (log_wr_fd>0){
		mutex_lock(WrLogMutex);
		close(log_wr_fd);
		log_wr_fd=0;
		mutex_unlock(WrLogMutex);
		return NO_ERROR;
	}
	return ERROR;
}

eErrorTp write_log_pipe(char * stringdata){
	int stlen=strlen(stringdata);
	mutex_lock(WrLogMutex);
	//printf("fd %d wrsz %d\n",log_wr_fd,stlen+1);
	if (log_wr_fd>0){
		int ret=write(log_wr_fd, stringdata,stlen+1);
		//printf("wrret %d\n",ret);
		mutex_unlock(WrLogMutex);
		if (ret!=stlen)
			return ERROR;
		else
			return NO_ERROR;
	}
	mutex_unlock(WrLogMutex);
	return ERROR;
}

void ReadLog(bool write_log){
	char foo[4096];
	ssize_t read_sz=0;
	//memset(foo,0,sizeof(foo));
	while ((read_sz=read(proclink_stdout[0], foo, sizeof(foo)))>0){
			printf("read_sz %d\n",(int)read_sz);
			foo[read_sz]=0;
			if (debug)
				printf("%s",foo);
			if (write_log){
				write_log_pipe(foo);
			}
			//memset(foo,0,sizeof(foo));
	}

	//memset(foo,0,sizeof(foo));
	while ((read_sz=read(proclink_stderr[0], foo, sizeof(foo)))>0){
			foo[read_sz]=0;
			if (debug)
				printf("%s",foo);
			if (write_log){
				string stderr=sf("\n####STDERR####\n%s\n####STDERR####\n\n",foo);
				write_log_pipe((char*)stderr.c_str());
				printf("stderr !!! %s !!!\n",foo);
			}
			//memset(foo,0,sizeof(foo));
	}

}

static void WorkerThread(pid_t* data)
{

	printf("Run worker %s\n",prog_name.c_str());
	process_log->to_log(3,prog_name.c_str(),"-!!->",sf("Run worker\n").c_str());

	TIME_T stime=TIME((TIME_T*)NULL);
	TIME_T dtime;
	exit_worker=false;
	worker_exited=false;
	while((exit_worker==false)&&(do_exit==false)){
		if (stop_worker){

			worker_stoped=true;

			mdelay(100);
		}
		else{

			worker_stoped=false;

			ReadLog(write_log);

			if (run_killer==false)
				stime=TIME((TIME_T*)NULL);

			dtime=TIME((TIME_T*)NULL);
			mdelay(10);
			if ((dtime-stime)>=crit_diff_time){
				if (child_pid>0){
					stime=TIME((TIME_T*)NULL);
					printf("Child process is hung..., force kill pid %u %x\n",child_pid,child_pid);
					process_log->to_log(3,prog_name.c_str(),"-!!->",sf("Child process is hung..., force kill\n").c_str());
					printf("kill SIGQUIT\n");
					kill(child_pid, SIGQUIT);
					printf("kill SIGINT\n");
					kill(child_pid, SIGINT);
					printf("kill SIGHUP\n");
					kill(child_pid, SIGHUP);
					printf("kill SIGTERM\n");
					kill(child_pid, SIGTERM);
					printf("kill SIGABRT\n");
					kill(child_pid, SIGABRT);
					sleep(1);
					printf("kill SIGKILL\n");
					kill(child_pid, SIGKILL);
				}

			}
		}

	}
	printf("Worker exit\n");

	worker_exited=true;
	worker_stoped=true;

}

void RunKiller(){
	run_killer=true;
}

void StopKiller(){
	run_killer=false;
}

eErrorTp RunWorker(){

	Thread_t thrk=0;
	printf("start worker for %s\n",prog_name.c_str());
	process_log->to_log(3,prog_name.c_str(),"-!!->",sf("start worker\n").c_str());
	if (StartPthread(&thrk,&WorkerThread,(void*)NULL)==ERROR)
	{
		printf("Crit error: do not run worker thread!!!\n");
		return ERROR;
	}
	return NO_ERROR;
}

void StopWorker(){

	stop_worker=true;
	printf("wait stop worker PID %u\n",getpid());
	mutex_lock(WorkerMutex);

	while(worker_stoped==false){
		mdelay(100);
	}
	mutex_unlock(WorkerMutex);
	//write_log=false;
	//printf("proclink_stdout[0]=%d\n",proclink_stdout[0]);
	if (proclink_stdout[0]!=0){
		close(proclink_stdout[0]);
		close(proclink_stderr[0]);
	}
	//close_log_pipe();
	printf("ok stop worker\n");

}

void StartWorker(){

	stop_worker=false;
	printf("wait start worker PID %u\n",getpid());
	mutex_lock(WorkerMutex);

	while(worker_stoped){
		mdelay(100);
		if (worker_exited)
			break;
	}
	mutex_unlock(WorkerMutex);
	printf("ok start worker\n");
}

eErrorTp SheduleDBG(string prog,string arg) {
	char **argv;
	int argc=0;
	string prog_fn=basename(prog.c_str());
	arg=sf("%s %s",prog.c_str(),arg.c_str());
	printf("svc: run shedule %s %s\n",prog.c_str(),arg.c_str());
	argv = split_commandline((char*)arg.c_str(),&argc);

	process_log->to_log(3,prog.c_str(),"->",sf("run execvp(%s,%s)\n",prog.c_str(),(char*)arg.c_str()).c_str());
	execvp(prog.c_str(),argv);
	//sleep(10);
	process_log->to_log(3,prog.c_str(),"->",sf("exit execvp\n").c_str());
	return NO_ERROR;
}

void Test() {
	SheduleDBG("/usr/sbin/sshd","-D -e -f /www/pages/openssh.cfg");
	exit(1);
}


eErrorTp Exe(pid_t & pID,pid_t & sl_pID,string prg,string args,bool log_enable,string log_cfg_file,string slogger){
	//pID=-1;
	//sl_pID=-1;
	StopWorker();
	pipe(proclink_stdout);
	pipe(proclink_stderr);


	pID=fork();

	if (pID==-1){
		printf("error fork process svc with arg %s\n",(char*)prg.c_str());
		exit(-1);
	}
	if (pID==0){
		exit_worker=true;
		stop_worker=true;

		dup2 (proclink_stdout[1], STDOUT_FILENO);
		dup2 (proclink_stderr[1], STDERR_FILENO);
		close(proclink_stdout[0]);
		close(proclink_stdout[1]);
		close(proclink_stderr[0]);
		close(proclink_stderr[1]);
		fcntl(STDOUT_FILENO, F_SETFL, O_NONBLOCK);
		fcntl(STDERR_FILENO, F_SETFL, O_NONBLOCK);

		process_log->to_log(3,prog_name.c_str(),"->",sf("fork shedule pid %u\n",pID).c_str());

		if (log_enable){
			args="-o0 -i0 -e0 "+prg+" "+args;
			prg="stdbuf";
		}

		Shedule(prg,args);
		printf("svc: terminate shedule\n");
		process_log->to_log(3,prog_name.c_str(),"->",sf("terminate shedule pid %u\n",pID).c_str());
		exit(1);
	}
	else{

		close(proclink_stdout[1]);
		close(proclink_stderr[1]);
		fcntl(proclink_stdout[0], F_SETFL, O_NONBLOCK);
		fcntl(proclink_stderr[0], F_SETFL, O_NONBLOCK);

		StartWorker();

		if (log_enable){
			if (sl_pID==-1){
				printf("log enabled, fork safe_logger for %s\n",prg.c_str());
				sl_pID=fork();
				slogger_pid=sl_pID;
				if (sl_pID==-1){
					printf("error fork safe_logger svc with arg %s\n",prg.c_str());
					exit(-1);
				}
				if (sl_pID==0){
					exit_worker=true;
					stop_worker=true;

					process_log->to_log(3,prog_name.c_str(),"->",sf("fork slogger pid %u\n",sl_pID).c_str());

					Shedule(slogger,log_cfg_file);
					printf("svc: terminate slogger\n");
					process_log->to_log(3,prog_name.c_str(),"->",sf("terminate slogger pid %u\n",sl_pID).c_str());
					exit(1);
				}
				else{

					for (u8 n=0;n<20;n++){
						if (open_log_pipe((char*)write_log_fn.c_str())==NO_ERROR)
							break;
						else{
							printf("error open %s\n",(char*)write_log_fn.c_str());
						}
						mdelay(100);
					}
					printf("opened pipe %s\n",(char*)write_log_fn.c_str());
					write_log=true;
				}
			}
			else
				write_log=true;
		}
	}

	child_pid=pID;
	slogger_pid=sl_pID;

	return NO_ERROR;
}

static void IsThread(void* data)
{
	ssize_t ret;
	string cmd;
	time_t ot,nt;
	u32 cnt=0;
	ot=time(NULL);
	while(1)
	{
		nt=time(NULL);
		if (cnt>=100){

			if ((nt>ot)&&((nt-ot)<2))
			{
				//broken stdin detect
				break;
				mdelay(100);
			}
			ot=time(NULL);
			cnt=0;
			//mdelay(100);
		}
		cnt++;


		cin >> cmd;
		mutex_lock(CmdMutex);
		command=cmd;
		mutex_unlock(CmdMutex);

	}
	do_exit=true;
}

string GetCommand(){
	mutex_lock(CmdMutex);
	string cmd=command;
	command="";
	mutex_unlock(CmdMutex);
	return cmd;
}
static pid_t   internal_child_pid = 0;
static inline pid_t get_child_pid(void)    { return __atomic_load_n(&internal_child_pid, __ATOMIC_SEQ_CST); }

static void forward_handler(int signum, siginfo_t *info, void *context)
{
    const pid_t target = get_child_pid();

    if (target != 0 && info->si_pid != target){
    	printf("child catch signal -%d PID %d\n",signum,target);
    	kill(target, signum);
    }
    else
    {
    	printf("parent catch signal -%d PID source %d recive from PID sender %d\n",signum,getpid(),info->si_pid);
    }
    if (info->si_pid!=getpid()){
		do_exit=true;
		//if (signum==SIGTERM)
			//exit_svc=true;
    }
}

static int forward_signal(const int signum)
{
    struct sigaction act;

    memset(&act, 0, sizeof act);
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = forward_handler;
    act.sa_flags = SA_SIGINFO | SA_RESTART;

    //if (signum==SIGPIPE)
    printf("forward_signal signum %d\n",signum);
    if (sigaction(signum, &act, NULL))
        return errno;

    return 0;
}

eErrorTp GenUID(string name, string arg, string & rUID){
	//std::replace(name.begin(),name.end(),' ','');
	string UID;
	//string nUID;


	UID=name+"_"+arg;
	buffer nUID(UID.length());

	std::copy_if(UID.begin(), UID.end(), nUID.p(), [](char c)
			{return
					((c!=' ')&&
					 (c!='/')&&
					 (c!='\\')&&
					 (c!='[')&&
					 (c!=']')&&
					 (c!=',')&&
					 (c!='&')&&
					 (c!='.')&&
					 (c!='^')&&
					 (c!='%')&&
					 (c!='\''));
			}
	);

	rUID=(char*)nUID.p();
	return NO_ERROR;;

}

eErrorTp PrepareLog(Json::Value logcfg,string & UID, string & cfg_filename,string & slogger){
	Json::Value cfg;
	cfg["slogger_loglevel"]=1;
	cfg["algo_type"]=0;
	cfg["aal_max_blocks"]=0;
	cfg["aal_lines_in_block"]=10000;
	cfg["aal_sync_time"]=0;
	cfg["aal_offset_time"]=1;
	cfg["aal_save_style"]=1;
	cfg["slogger_atime_format"]=0;
	cfg["slogger_atime"]=1;


	if (existsDir(SLOGGER_PIPE_PATH)==ERROR){
		MkPath(SLOGGER_PIPE_PATH, 0xffffffff);
	}
	string rlog_fn=sf("%s/log.%s",SLOGGER_PIPE_PATH,UID.c_str());
	write_log_fn=rlog_fn+"_wo";
	cfg["log_out_fifo"]=rlog_fn;
	cfg["fifo_file"]=write_log_fn;

	cfg["aal_path[0]"]="/var/run/log1/shared";
	cfg["aal_path[1]"]="/var/run/log2/shared";

	if (logcfg.isMember("aal_save_style"))
		cfg["aal_save_style"]=logcfg["aal_save_style"];

	if (logcfg.isMember("aal_lines_in_block"))
		cfg["aal_lines_in_block"]=logcfg["aal_lines_in_block"];

	if (logcfg.isMember("aal_sync_time")){
		cfg["aal_sync_time"]=logcfg["aal_sync_time"];
	}

	if (logcfg.isMember("aal_offset_time")){
			cfg["aal_offset_time"]=logcfg["aal_offset_time"];
	}

	if (logcfg.isMember("aal_path")){
		string base=logcfg["aal_path"].asString();
		string lp1=base+"/log1/"+UID;
		string lp2=base+"/log2/"+UID;
		cfg["aal_path[0]"]=lp1;
		cfg["aal_path[1]"]=lp2;

	}
	if (logcfg.isMember("slogger")){
		slogger=logcfg["slogger"].asString();
	}
	else
		slogger="/www/pages/necron/Cnoda/safe_logger";

	if (logcfg.isMember("aal_max_blocks"))
			cfg["aal_max_blocks"]=logcfg["aal_max_blocks"];

	string cfg_path=logcfg["aal_path"].asString();
	if (existsSync(cfg_path)==ERROR)
		MkPath(cfg_path.c_str(),0xffffffff);

	cfg_filename=cfg_path+"/"+UID;
	string data=StyledWriteJSON(cfg);
	return WriteStringFile((char*)cfg_filename.c_str(),data);
}
//Examples
///svc '{"prg":"sleep","args":"10","stop_time":10,"log":{"aal_lines_in_block":10000,"aal_path":"/var/run/svclog/"}}'

/*
 * {
 prg:""
 args:"",
 stop_time:20,
 uid:"",
 log:{
   aal_lines_in_block:"10000",
   aal_path:"/var/run/"
   aal_max_blocks:"0"
   slogger:"/www/pages/necron/Cnoda/safe_logger
  // slogger_loglevel:0
 }
}
 */



int main(int argc, char *argv[])
{
	Json::Value jarg;
	Json::Reader rd;
	string ip="239.100.100.1";
	//u32 port=11220;
	string args;
	string prg;
	string UID;

	IPC_SocketT ips(NORMAL_LEVEL);
	pid_t pID=-1;
	pid_t sl_pID=-1;
	bool log_enable=false;
	string log_cfg_filename;
	string slogger_path;
	mutex_init(CmdMutex);
	mutex_init(WorkerMutex);
	mutex_init(WrLogMutex);



	Thread_t thr=0;
	if (argc<2)
		exit(1);

	//printf("run %s %s\n",argv[0],argv[1]);
	if (forward_signal(SIGINT) ||
	        forward_signal(SIGHUP) ||
	        forward_signal(SIGTERM) ||
	        forward_signal(SIGQUIT) ||
	        forward_signal(SIGUSR1) ||
			forward_signal(SIGPIPE) ||
	        forward_signal(SIGUSR2)) {
	        fprintf(stderr, "Cannot install signal handlers: %s.\n", strerror(errno));
	        return EXIT_FAILURE;
	    }
	if (rd.parse(argv[1],jarg)){
		if (jarg.isMember("prg")&&
				jarg.isMember("args")
		)
		{
			if (jarg.isMember("ip")){
				ip=jarg["ip"].asString();
			}
			if (jarg.isMember("stop_time")){
				crit_diff_time=jarg["stop_time"].asUInt();
			}
			//if (jarg.isMember("port")){
				//port=jarg["port"].asUInt();
			//}
			args=jarg["args"].asString();
			prog_name=prg=jarg["prg"].asString();

			if (jarg.isMember("uid")&&(jarg["uid"].asString().length()!=0)){
				//string ui=jarg["uid"].asString();//.length();
				//if (ui.length()!=0)
				UID=jarg["uid"].asString();
			}
			else
				GenUID(prg, args,UID);

			printf("UID %s\n",UID.c_str());
			cout << StyledWriteJSON(jarg) << endl;

			if (jarg.isMember("log")){
				log_enable=true;
				PrepareLog(jarg["log"],UID, log_cfg_filename,slogger_path);

			}
			if (jarg.isMember("debug"))
				debug=true;

			process_log=make_shared<safelogger_client>("$ $ $","/www/pages/necron/Cnoda/svc.slogger.json",NORMAL_LEVEL);

			//zr();

			sSock Socket;
			u16 sport;


			RunWorker();
			process_log->to_log(3,prog_name.c_str(),"->","first start\n");
			Exe(pID,sl_pID,prg,args,log_enable,log_cfg_filename,slogger_path);
			printf("PID %d\n",pID);

			int status;
			int logger_status;


			TIME_T t;
			//zr();
			if (StartPthread(&thr,&IsThread,NULL)==ERROR)
				  {
					   printf("Crit error: do not run thread!!!\n");
					   exit(-1);
				   }

			 string cmd;
			 string old_cmd="start";
			 enum {NONE,STOP,FORCE_STOP,START,RESTART,EXIT};
			 u32 stat=START;

			 TIME_T init_time;
			 TIME(&init_time);
			 pid_t lpID;
			 u8 cntr10s=0;

			 auto WaitPid=[&](void){
			 	  if (log_enable)
			 		  waitpid(sl_pID, &status, WNOHANG);
			 	  waitpid(pID, &logger_status, WNOHANG);
			 	  printf("waidpid\n");
			 	};
			while(1){
				//Test();
				TIME(&t);
				if (init_time>t)
					init_time=t;

				cmd=GetCommand();

				if ((old_cmd==cmd)&&(cmd!="exit"))
					continue;

				if (cmd.length()!=0){
					printf("cmd %s\n",cmd.c_str());
					old_cmd=cmd;
				}

				if (do_exit)
					cmd="exit";

				if (cmd.length()!=0){
					if (cmd=="stop"){
						printf("\n----STOP---[%s]\n\n",prog_name.c_str());
						process_log->to_log(3,prog_name.c_str(),"->","stop\n");
						stat=STOP;
						if ((lpID!=-1)&&(pID>0)){
							printf("kill pID %u, 0x%x\n",pID,pID);
							RunKiller();
							kill(pID, SIGINT);
							//if (log_enable){
							//	printf("kill sl_pID %u, 0x%x\n",sl_pID,sl_pID);
							//	kill(sl_pID, SIGINT);
							//}
						}
						else
							stat=NONE;

					}
					if(cmd=="exit"){
						stat=EXIT;
						printf("\n----EXIT---[%s]\n\n",prog_name.c_str());
						process_log->to_log(3,prog_name.c_str(),"->","exit\n");
						if ((lpID!=-1)&&(pID>0)){
							printf("kill pID %u, 0x%x\n",pID,pID);
							RunKiller();
							kill(pID, SIGINT);
							if (log_enable){
								printf("kill sl_pID %u, 0x%x\n",sl_pID,sl_pID);
								StopWorker();
								close_log_pipe();
								kill(sl_pID, SIGINT);
							}
						}
						else{
						 WaitPid();
						 break;// exit(0);
						}
					}
					if (cmd=="start"){
						printf("\n----START---[%s]\n\n",prog_name.c_str());
						process_log->to_log(3,prog_name.c_str(),"->","start\n");
						stat=START;
						init_time=t;
						Exe(pID,sl_pID,prg,args,log_enable,log_cfg_filename,slogger_path);
						//stat=NONE;
					}
					if(cmd=="restart"){
						printf("\n----RESTART---[%s]\n\n",prog_name.c_str());
						process_log->to_log(3,prog_name.c_str(),"->","restart\n");
						stat=RESTART;
						if ((lpID!=-1)&&(pID>0)){
							kill(pID, SIGINT);
							RunKiller();
							//if (log_enable)
								//kill(sl_pID, SIGINT);
						}
						else{
							init_time=t;
							Exe(pID,sl_pID,prg,args,log_enable,log_cfg_filename,slogger_path);
							stat=NONE;
							//stat=NONE;
						}
					}
				}

				if (log_enable){
					if (waitpid(sl_pID, &status, WNOHANG)==-1)
						mdelay(10);
				}
				if ((lpID=waitpid(pID, &logger_status, WNOHANG)) == -1){
					mdelay(10);
				}

#if 1
				else if (lpID == 0) {

				  printf("run %s %s run time %u stat %d\n",(char*)prg.c_str(),(char*)args.c_str(), t-init_time,stat);
				  //sleep(1);
				  cntr10s++;
				  if (cntr10s>=10){
					  process_log->to_log(3,prog_name.c_str(),"->",sf("args %s run time %u stat %d\n",(char*)args.c_str(), (u32)(t-init_time),stat).c_str());
					  cntr10s=0;
				  }
				}
				else {
				  if (WIFEXITED(status)){
					printf("exit with result %d %s %s %u stat %d\n", WEXITSTATUS(status),(char*)prg.c_str(),(char*)args.c_str(), (u32)(t-init_time),stat);
					process_log->to_log(3,prog_name.c_str(),"->",sf("exit with result %d args %s %u stat %d\n", WEXITSTATUS(status),(char*)args.c_str(), t-init_time,stat).c_str());
				  }
				  else{
					  printf("exit %s %s %u stat %d\n",(char*)prg.c_str(),(char*)args.c_str(), t-init_time,stat);
					  process_log->to_log(3,prog_name.c_str(),"->",sf("exit args %s %u stat %d\n",(char*)args.c_str(), (u32)(t-init_time),stat).c_str());
				  }
				  StopKiller();
				  StopWorker();

				  if (stat==EXIT){
					  WaitPid();
					  break;//exit(0);
				  }

				  if (stat==RESTART)
					  old_cmd="start";

				  if ((stat==START)||(stat==RESTART)){
					  init_time=t;
					  Exe(pID,sl_pID,prg,args,log_enable,log_cfg_filename,slogger_path);

				  }
				}
				sleep(1);
#endif
			}

			exit_worker=true;
			while(worker_exited==false){
				sleep(1);
				printf("Wait worker exit\n");
			}
		}

		exit(0);
	}
	exit(-1);
}
