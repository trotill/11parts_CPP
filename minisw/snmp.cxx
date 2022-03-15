/*
 * snmp.cxx
 *
 *  Created on: 22 марта 2018 г.
 *      Author: root
 */
#include "../proto/snmp.h"

#include "engine/thread.h"
#include "custom_project/custom_project.h"
sGlobalSNMP sGSN;

string SUBAGENT_CONFIG_FILE="/etc/Cnoda/subagent.json";
SNMP_WrNamedFifoMutex SNMP_mutex;

eErrorTp SAParseOpts(int argc, char *argv[])
{
	int c;


	static struct option long_options[] =
            {
              {"help",     no_argument,       0, 'h'},
              {"conf",  required_argument,       0, 'c'},
         //     {"debug",  required_argument,       0, 'd'},
              {0, 0, 0, 0}
            };
	 struct in_addr inp;
	 while (1)
	 {
		 int option_index = 0;

          c = getopt_long (argc, argv, "h:c",
                           long_options, &option_index);

          /* Detect the end of the options. */
          if (c == -1)
            break;

          switch (c)
            {
            case 0:

              /* If this option set a flag, do nothing else now. */
              if (long_options[option_index].flag != 0)
                break;
              cout << "option " << long_options[option_index].name;
              if (optarg)
                cout << "with arg " << optarg;
              cout << endl;
              break;

            case 'c':
            	if (optarg!=0)
            	{
            		if (SearchFile(optarg)!=ERROR)
            		{
            			SUBAGENT_CONFIG_FILE=optarg;

            			 cout << "Found  conf " << SUBAGENT_CONFIG_FILE << endl;
            		}
            		else
            			 cout << "Not found "<<optarg<<", use default conf " << SUBAGENT_CONFIG_FILE << endl;
            	}
            	break;

            case 'h':

            case '?':
            	exit(0);
            	break;
              /* getopt_long already printed an error message. */
            default:
              printf("Incorrect options, exit\n");
              exit(0);
            }
	 }


   return NO_ERROR;
 }

eErrorTp AgentConfig(string & conf)
{

	std::ifstream config_doc(conf.c_str(), std::ifstream::binary);
	config_doc >> sGSN.root;

	sGSN.agentx_enable=false;
	if (sGSN.root["agentx_enable"].isNull()==false)
	{
		if (sGSN.root["agentx_enable"].asBool())
			sGSN.agentx_enable=true;
	}

	sGSN.agent_name="agent";
	if (sGSN.root["agent_name"].isNull()==false)
	{
		sGSN.agent_name=sGSN.root["agent_name"].asString();
	}

	sGSN.debug_level=0;
	if (sGSN.root["debug_level"].isNull()==false)
	{
		sGSN.debug_level=sGSN.root["debug_level"].asInt();
	}

	string fifo_name_out="/run/snmp.cnoda.get";
	string fifo_name_in="/run/snmp.cnoda.set";
	if (sGSN.root["fifo_name"].isNull()==false)
	{
		fifo_name_out=sGSN.root["fifo_name"].asString()+".get";
		fifo_name_in=sGSN.root["fifo_name"].asString()+".set";
	}

	remove(fifo_name_out.c_str());
	remove(fifo_name_in.c_str());
	mkfifo(fifo_name_out.c_str(), 0666);
	mkfifo(fifo_name_in.c_str(), 0666);
	sGSN.fdfifo_out = open(fifo_name_out.c_str(), O_RDWR|O_NONBLOCK);
	sGSN.fdfifo_in = open(fifo_name_in.c_str(), O_RDWR);

	return NO_ERROR;
}

eErrorTp AgentRegs(void)
{
	u32 ngp=0;
	if (sGSN.root["node"].isArray())
		{
			while(sGSN.root["node"][ngp].isNull()==false)
			{
				//string sname, string saccess, string soid, string stype, string sdefval

				sGSN.OIDs[ngp]=new OIDObj(sGSN.root["node"][ngp][0].asString(),
						sGSN.root["node"][ngp][2].asString(),
						sGSN.root["node"][ngp][3].asString(),
						sGSN.root["node"][ngp][1].asString(),
						sGSN.root["node"][ngp][4].asString());
				ngp++;
			}
		}
	//agent_name

	return NO_ERROR;
}

#if 1



#endif

u32 GetNodeIndex(string name)
{
	u32 ngp=0;
	while(sGSN.root["node"][ngp].isNull()==false)
	{
		if (sGSN.root["node"][ngp][0].asString()==name)
		{
			return ngp;
		}
		ngp++;
	}
	return -1;
}

u32 GetNodeType(string stype)
{
	u32 type;
	type=ASN_OCTET_STR;
	if (stype=="INTEGER")
		type=ASN_INTEGER;

	if (stype=="BOOLEAN")
		type=ASN_BOOLEAN;

	if (stype=="OCTET_STR")
		type=ASN_OCTET_STR;

	return type;
}

int OidHandler(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    int ret;
    int intValue;
    u32 node_idx=GetNodeIndex(handler->handler_name);

    mutex_lock(sGSN.OIDs[node_idx]->Mutex);

    u32 node_type=GetNodeType(sGSN.root["node"][node_idx][1].asString());


    //if (handler->handler_name!=NULL)
    //snmp_log(LOG_INFO,"handler_name %s node_idx %d\n",handler->handler_name,node_idx);

    if (reginfo->contextName!=NULL)
    snmp_log(LOG_INFO,"contextName %s\n",reginfo->contextName);

    //snmp_log(LOG_INFO,"MODE %d\n",reqinfo->mode);
    switch(reqinfo->mode) {

        case MODE_GET:
            if( sGSN.debug_level ) snmp_log( LOG_INFO, "MODE_GET\n" );
//            iNextStep++;
            switch (node_type){
            case ASN_INTEGER: snmp_set_var_typed_value(requests->requestvb, node_type,
            			&sGSN.OIDs[node_idx]->intValue               /* XXX: a pointer to the scalar's data */,
            			sizeof( sGSN.OIDs[node_idx]->intValue  )      /* XXX: the length of the data in bytes */);
            	break;
            case ASN_OCTET_STR:
                snmp_set_var_typed_value(requests->requestvb, node_type,
                        sGSN.OIDs[node_idx]->strValue.c_str()               /* XXX: a pointer to the scalar's data */,
                         sGSN.OIDs[node_idx]->strValue.size()        /* XXX: the length of the data in bytes */);
                break;
            }
            break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
            if( sGSN.debug_level ) snmp_log( LOG_INFO, "MODE_SET_RESERVE1\n" );
                /* or you could use netsnmp_check_vb_type_and_size instead */
            ret = netsnmp_check_vb_type(requests->requestvb, node_type);
            if ( ret != SNMP_ERR_NOERROR ) {
                snmp_log( LOG_ERR, "MODE_SET_RESERVE1 rc=%d\n", ret );
                netsnmp_set_request_error(reqinfo, requests, ret );
            }
            netsnmp_set_request_error( reqinfo, requests, SNMP_ERR_NOERROR );
            break;

        case MODE_SET_RESERVE2:
            if( sGSN.debug_level ) snmp_log( LOG_INFO, "MODE_SET_RESERVE2\n" );
            /* XXX malloc "undo" storage buffer */
//            if (/* XXX if malloc, or whatever, failed: */) {
//                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
//            }
            netsnmp_set_request_error( reqinfo, requests, SNMP_ERR_NOERROR );
            break;

        case MODE_SET_FREE:
            if( sGSN.debug_level ) snmp_log( LOG_INFO, "MODE_SET_FREE\n" );
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            if( sGSN.debug_level ) snmp_log( LOG_INFO, "MODE_SET_ACTION\n" );
            /* XXX: perform the value change here */
            { netsnmp_variable_list *var = requests->requestvb;
              netsnmp_vardata val = var->val;
              string rstr="";
              switch (node_type)
              {
              	  case ASN_INTEGER:
              		  	 sGSN.OIDs[node_idx]->intValue = *val.integer;
              	  	  	 rstr=string_format("%s@@%d\n",sGSN.OIDs[node_idx]->name.c_str(),sGSN.OIDs[node_idx]->intValue);
              	  	  break;
              	  case ASN_OCTET_STR:
              		    sGSN.OIDs[node_idx]->strValue = (char*)var->val.string;
              		    rstr=string_format("%s@@%s\n",sGSN.OIDs[node_idx]->name.c_str(),sGSN.OIDs[node_idx]->strValue.c_str());
              	  	  break;
              }
              ret=write(sGSN.fdfifo_out, rstr.c_str(), rstr.size());
              //iCurrentValue += intValue;
            }
//            if (/* XXX: error? */) {
//                netsnmp_set_request_error(reqinfo, requests, /* some error */);
//            }
            break;

        case MODE_SET_COMMIT:
            if( sGSN.debug_level ) snmp_log( LOG_INFO, "MODE_SET_COMMIT\n" );
            /* XXX: delete temporary storage */
//            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
//                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
//            }
            break;

        case MODE_SET_UNDO:
            if( sGSN.debug_level ) snmp_log( LOG_INFO, "MODE_SET_UNDO\n" );
            /* XXX: UNDO and return to previous value for the object */
//            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
//                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
//            }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_nextStep\n", reqinfo->mode );
            mutex_unlock(sGSN.OIDs[node_idx]->Mutex);
            return SNMP_ERR_GENERR;
    }

    mutex_unlock(sGSN.OIDs[node_idx]->Mutex);
    return SNMP_ERR_NOERROR;
}

volatile sig_atomic_t keep_running = 0;
RETSIGTYPE stop_server( int a ) {
   if( sGSN.debug_level != 0 )
      snmp_log( LOG_INFO, "\n--------- got signal: %d -------------\n", a );
   keep_running = 0;
}


static void* ReadFifoThread(void* data)
{

	ssize_t ret;
	char name[MAX_SYMBOLS];
	char value[MAX_SYMBOLS];
	ssize_t rbsize=sizeof(sGSN.ReadBuf);
	char * buf=sGSN.ReadBuf;
	u32 last_size=0;
	while(1)
	{
		//printf("thread\n");
		//echo "<name>@@<value>"
		ret=read(sGSN.fdfifo_in, sGSN.ReadBuf, rbsize);
		//printf("read:%d byte\n",ret);

		if ((ret!=0)&&(ret!=-1)&&(ret!=rbsize))
		{
			//printf("read fifo:[%s]\n",sGSN.ReadBuf);
			buf=sGSN.ReadBuf;
			last_size=ret;
			for(u8 z=0;z<255;z++)
			{
			memset(name,0,sizeof(name));
			memset(value,0,sizeof(name));

			//printf("parse:[%s]\n",buf);
			sscanf(buf,"%[^@]@@%[^\n]",name,value);
			//printf("read %s %s\n",name,value);
			if ((strlen(name)!=0)&&(strlen(value)!=0))
			{
				printf("name %s value %s\n",name,value);
				u32 x=0;

				while((sGSN.OIDs[x]!=NULL))
				{
					if (sGSN.OIDs[x]->name.compare(name)==0)
					{
						mutex_lock(sGSN.OIDs[x]->Mutex);
						if (sGSN.OIDs[x]->node_type==ASN_INTEGER)
						{
							sGSN.OIDs[x]->intValue=atoi(value);
						}
						else
						{
							sGSN.OIDs[x]->strValue=value;
						}
						mutex_unlock(sGSN.OIDs[x]->Mutex);
						break;
					}
					x++;
					//printf("x %d\n",x);
				}
			}
			//printf("m1\n");
			u32 cmd_len=0;
			for(u32 m=0;m<1024;m++)
			{
				//printf("buf[%d]=%d\n",m,buf[m]);
				if (buf[m]=='\n')
				{

					cmd_len=m+1;
					//printf("cmd_len %d\n",cmd_len);
					break;
				}
			}
			if (last_size<cmd_len)
			{
				printf("error data parse\n");
				break;
			}
			last_size-=cmd_len;

			//printf("last_size %d\n",last_size);
			if (last_size==0)
				break;

			//printf("last_size %d cmd_len %d\n",last_size,cmd_len);
			buf+=cmd_len;
			if (buf[0]==0)
			{
				printf("error data parse\n");
				break;
			}
			}
			memset(sGSN.ReadBuf,0,ret);
		}
		mdelay(100);
	}
}

int snmp_main(int argc, char **argv)
{

	//argv[1] - json conf string




	Thread_t thr=0;
	SAParseOpts(argc,argv);
	//printf("conf %s argc %d \n",SUBAGENT_CONFIG_FILE.c_str(),argc);
	if ((argc!=2)||(SearchFile(SUBAGENT_CONFIG_FILE.c_str())==ERROR))
		return -1;

	//string conf=argv[1];
	//printf("conf file %s\n",SUBAGENT_CONFIG_FILE.c_str());

	AgentConfig(SUBAGENT_CONFIG_FILE);
	init_snmp_logging();
	snmp_enable_syslog();

	snmp_enable_stderrlog();      // print log errors to stderr


	//RegOIDValue();
	//u32 agentx_subagent = 0;
	//if (agentx_enable
	//u32 debug_level=0;
	 //printf("me0\n");
	// if( background && netsnmp_daemonize( 1, !syslog ) ) exit( 1 );
	if( sGSN.debug_level > 1 ) snmp_set_do_debugging( 1 );
	//printf("me0.1\n");
	if( sGSN.agentx_enable )         // we're an agentx subagent?
	      netsnmp_ds_set_boolean( NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1 );
	//printf("me0.2\n");
	   SOCK_STARTUP;                 // initialize tcpip, if necessary
	  // printf("agentname %s\n",sGSN.agent_name.c_str());
	   init_agent( sGSN.agent_name.c_str() );   // initialize the agent library
	  // printf("me1\n");
	   AgentRegs();
	  // printf("me2\n");
	   if (StartPthread(&thr,&ReadFifoThread,&sGSN)==ERROR)
	   {
		   printf("Crit error: do not run thread!!!\n");
		   exit(-1);
	   }

	   /*
	   init_currentValue();          // initialize mib code here...
	   init_nextStep();
	   */

	   init_snmp( sGSN.agent_name.c_str() );    // имя конфиг-файла
	   if( !sGSN.agentx_enable )        // If we're going to be a snmp master agent, initial the ports
	      init_master_agent();       // open the port to listen on (defaults to udp:161)
	   signal( SIGTERM, stop_server );
	   signal( SIGINT, stop_server );
	   if( sGSN.debug_level != 0 ) snmp_log( LOG_INFO, "%s is up and running.\n", sGSN.agent_name.c_str() );
	   keep_running = 1;
	   while( keep_running ) {       // you're main loop here...
	      // if you use select(), see snmp_select_info() in snmp_api(3)
	      //     --- OR ---
	      agent_check_and_process( 1 ); // 0 == don't block
	     // printf("test\n");
	   }
	   snmp_shutdown( sGSN.agent_name.c_str() );   // завершение работы
	   SOCK_CLEANUP;
	   if( sGSN.debug_level != 0 ) snmp_log( LOG_INFO, "%s was finished.\n", sGSN.agent_name.c_str() );


	exit(0);
}



