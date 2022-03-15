/*
 * snmp_adm.cxx
 *
 *  Created on: 14 янв. 2019 г.
 *      Author: root
 */

#include "snmp.h"
#include "custom_project/custom_main.h"

eErrorTp SNMP_adm::SNMP_DumpCfg(){
	printJSON("SNMP_DumpCfg",root_suba);
	return NO_ERROR;
}

eErrorTp SNMP_adm::SNMP_Init(string & subagent_cfg)
	{
		printf("read %s\n",subagent_cfg.c_str());
		std::ifstream config_doc(subagent_cfg.c_str(), std::ifstream::binary);
		config_doc >> root_suba;
		if (root_suba["fifo_name"].isNull()==false)
			{
				snmp_fifo_name=root_suba["fifo_name"].asString();
			}
		else
			snmp_fifo_name="/run/snmp.cnoda";

		return NO_ERROR;
	};


eErrorTp SNMP_adm::GetSNMP_Val(void)
{
				//echo "<name>@@<value>"
				//string send_str=string_format("echo '%s@@%s' > %s.set",name.c_str(),val.c_str(),snmp_fifo_name.c_str());
				eErrorTp err=NO_ERROR;

				string fname=snmp_fifo_name+".get";

				///string send_str=string_format("%s@@%s\n",name.c_str(),val.c_str());

				int fd=-1;
				int MaxBufLen=sizeof(SNMPGetBuf);
				int len=0;
				fd=open(fname.c_str(),O_RDONLY |O_NONBLOCK);
				if (fd!=-1)
				{
					len=read(fd, SNMPGetBuf, MaxBufLen);
					close(fd);
					if (!((len==0)||(len==-1)))
					{
						GPRINT(NORMAL_LEVEL,"SNMP agent read %d byte\n",len);
						GPRINT(NORMAL_LEVEL,"SNMP message [%s]\n",SNMPGetBuf);
						ParseSNMPAgentStr(SNMPGetBuf,len);
					}
				}
				else{
					GPRINT(MEDIUM_LEVEL,"Error: file %s not open\n",fname.c_str());
					err=ERROR;
				}

				return err;
}

eErrorTp SNMP_adm::SetSNMP_Val(string name,string val)
{
				//echo "<name>@@<value>"
				//string send_str=string_format("echo '%s@@%s' > %s.set",name.c_str(),val.c_str(),snmp_fifo_name.c_str());
	eErrorTp err=NO_ERROR;

				string fname=snmp_fifo_name+".set";
				string send_str=string_format("%s@@%s\n",name.c_str(),val.c_str());
				//printf("%s\n",send_str.c_str());
				GPRINT(HARD_LEVEL,"%s\n",send_str.c_str());
				int fd=-1;

				SNMP_mutex.lock();
				//mutex_lock(CnT->WrNamedFifo);
				fd=open(fname.c_str(),O_WRONLY |O_NONBLOCK);
				if (fd!=-1)
				{
					//printf("write %s\n",send_str.c_str());
					write(fd, send_str.c_str(), send_str.size());
					close(fd);
				}
				else{
					GPRINT(MEDIUM_LEVEL,"Error: file %s not open\n",fname.c_str());
					err=ERROR;
				}
				//mutex_unlock(CnT->WrNamedFifo);
				SNMP_mutex.unlock();
				return err;
}

eErrorTp SNMP_adm::SetSNMP_Val(string name,u32 val)
{
				//echo "<name>@@<value>"
				//string send_str=string_format("echo 'test@@%d' > %s.set",val,snmp_fifo_name.c_str());
	eErrorTp err=NO_ERROR;
				string fname=snmp_fifo_name+".set";
				string send_str=string_format("%s@@%d\n",name.c_str(),val);
				GPRINT(HARD_LEVEL,"%s\n",send_str.c_str());
				int fd=-1;
				SNMP_mutex.lock();
				//mutex_lock(CnT->WrNamedFifo);
				fd=open(fname.c_str(),O_WRONLY |O_NONBLOCK);
				if (fd!=-1)
				{
					write(fd, send_str.c_str(), send_str.size());
					close(fd);
				}
				else{
					GPRINT(MEDIUM_LEVEL,"Error: file %s not open\n",fname.c_str());
					err=ERROR;
				}
				//mutex_unlock(CnT->WrNamedFifo);
				SNMP_mutex.unlock();
				return err;
}

eErrorTp SNMP_adm::ParseSNMPAgentStr(char * bufr,u32 len)
{

				char * buf=bufr;
				u32 last_size=len;
				buffer name(len);
				buffer value(len);
				//char name[255];
				//char value[255];

				printhex((u8*)bufr,len,16);
				for(u8 z=0;z<255;z++)
					{
							//memset(name,0,sizeof(name));
							//memset(value,0,sizeof(name));

							printf("parse:[%s]\n",buf);
							sscanf(buf,"%[^@]@@%[^\n]",name.p(),value.p());
							if ((strlen((char*)name.p())!=0)&&(strlen((char*)value.p())!=0))
							{

								Customer.SetValue((char*)name.p(),(char*)value.p());
							}
							u32 cmd_len=0;
							for(u32 m=0;m<len;m++)
							{
								if (buf[m]=='\n')
								{
									cmd_len=m+1;
									break;
								}
							}

							if (last_size<cmd_len)
							{
								//printf("last_size %d<cmd_len %d\n",last_size,cmd_len);
								//printf("error data parse\n");

								break;
							}
							last_size-=cmd_len;
							//printf("last_size %d\n",last_size);
							if (last_size==0)
								break;

							buf+=cmd_len;
							if (buf[0]==0)
							{
								//printf("buf[0]==0\n");
								//printf("error data parse\n");
								break;
							}
					}

	return NO_ERROR;
}

OIDObj::OIDObj(string sname, string saccess, string soid, string stype, string sdefval)
{
			snmp_log( LOG_INFO,"Create oid name %s type %s oid %s access %s val %s\n",sname.c_str(),stype.c_str(),soid.c_str(),saccess.c_str(),sdefval.c_str());
			u32 type=GetNodeType(stype);
			u32 access=HANDLER_CAN_RONLY;
			if (type==ASN_INTEGER)
				intValue=stoul(sdefval);
			if (type==ASN_OCTET_STR)
				strValue=sdefval;

			if (saccess=="rw")
				access=HANDLER_CAN_RWRITE;

			if (saccess=="ro")
				access=HANDLER_CAN_RONLY;

			if (saccess=="wo")
				access=HANDLER_CAN_SET_ONLY;

			//printf("access %d\n",access);
			name=sname;
			mutex_init(Mutex);
			RegOIDValue(sname,soid,access);
			node_type=GetNodeType(stype);
}

eErrorTp OIDObj::RegOIDValue(string & name,string & oids,int modes)
{
	oid oid_values[20]={0};//{1,3,6,1,4,1,9876,11,5};
	stringstream res;
	u32 ovcount=0;
	for (u32 n=0;n<oids.size();n++)
	{
		if ((oids.c_str()[n]>='0')&&(oids.c_str()[n]<='9'))
		{
			res << oids.c_str()[n];
		}
		else
		{
			if (res.str().size()>0)
			{
				oid_values[ovcount]=stoul(res.str());
				ovcount++;
			}
			res.str("");
			res.clear();
		}
	}
	if (res.str().size()>0)
	{
		oid_values[ovcount]=stoul(res.str());
		ovcount++;
	}

	netsnmp_register_scalar(netsnmp_create_handler_registration(name.c_str(), OidHandler,oid_values, ovcount, modes));

	return NO_ERROR;
}

