/*
 * abstraction_mc.cxx
 *
 *  Created on: 20 янв. 2020 г.
 *      Author: root
 */

#include "abstraction.h"


	eErrorTp DNKchainT::DNKlink_algo_IncCMD(Json::Value & inc_root,u32 inc){

		if (inc_root.isObject())
		{
			if ((inc_root.isMember("$"))&&(inc_root["$"].isMember("cmd"))){
						u32 cmd=stoul(inc_root["$"]["cmd"].asString(),0,16);
						cmd+=inc;
						inc_root["$"]["cmd"]=string_format("%X",cmd);
						//printf("cmd %d to %d\n",cmd,inc);
			}
			else{
				for (const auto& key : inc_root.getMemberNames()) {
					DNKlink_algo_IncCMD(inc_root[key],inc);
				}
			}


			/*if ((inc_root[key].isObject())&&(inc_root[key].isMember("$"))&&(inc_root[key]["$"].isMember("cmd"))){
				u32 cmd=stoul(inc_root[key]["$"]["cmd"].asString(),0,16);
				cmd+=inc;
				inc_root[key]["$"]["cmd"]=string_format("%x",cmd);
			}*/

		}
		return NO_ERROR;
	}



eErrorTp DNKchainT::MC_ParsePackData(char * cmd_code,Json::Value & mc_cmd,u8 * buf,Json::Value & json_result,eErrorTp (*User_UnPackerValue)(u8 * bin_value,u32 size,string algo,string & path,Json::Value & main_root,Json::Value & root_result,void * cb_context),void * cb_context){
		//printf("cmd_code %s\n",cmd_code);
		//Json::Value  json_result;
		//result="{}";
		if (mc_cmd.isMember(cmd_code)){
				Json::Value mc=mc_cmd[cmd_code];

				for (const auto& key : mc.getMemberNames()) {
						u32 offset=stoul(key.c_str());
						u32 size=mc[key][0].asInt();
						string path=mc[key][2].asString();

						bitCopy(MC_ParsePackData_value_tmp,0,size,buf,offset);
						//printf("ins %s\n",path.c_str());
						if (MC_ParsePackData_ConvValue((u8*)MC_ParsePackData_value_tmp,size,mc[key][1].asString(),path,json_result)==ERROR){
							if (User_UnPackerValue!=NULL){
								if (User_UnPackerValue((u8*)MC_ParsePackData_value_tmp,size,mc[key][1].asString(),path,root,json_result,cb_context)==ERROR)
									return ERROR;
							}
							else
								return ERROR;
						}

				}
		}
		else
		   return ERROR;

		//Json::FastWriter writer;
		//result=writer.write(json_result);
		return NO_ERROR;
	};

eErrorTp DNKchainT::MC_CreatePackData_ConvValue(u32 bitsize,string algo,Json::Value js_value, u8 * result){
	if (js_value.isArray()){
		if ((algo=="a1")){
			u32 i=0;
			u32 delta=0;
			u32 rezd=0;
			if (js_value.size()<=bitsize){
				memset(result,0,(bitsize>>3)+1);

				while(js_value[i].isNull()==false)
				{
					rezd=i>>3;
					delta=i-(rezd<<3);
					result[rezd]|=(js_value[i].asInt()&1)<<delta;
					i++;
				}
			}
		}
		else
		if ((algo=="a2")){
			u32 i=0;
			u32 bytesize=bitsize/8;
			if (js_value.size()<=bytesize){
				memset(result,0,bytesize);
				while(js_value[i].isNull()==false){
					result[i]=js_value[i].asInt();
					i++;
				}
			}
		}
	}
	else
	{
		//zr();
		string value=js_value.asString();
		if (value.size()==0){
			printf("ERROR: MC_CreatePackData_ConvValue value is empty\n");
			//value="0";
			return ERROR;
		}
		//printf("value %s\n",value.c_str());
		//zr();
		//printf("Conv %s %s bsize %d\n",algo.c_str(),value.c_str(),bitsize);
		if (algo=="d"){
			if (bitsize<=32){
				//zr();
				s32 v=(s32)stol(value);//int - 32b, long - 64b
			//	zr();
				memcpy(result,&v,sizeof(s32));
				//zr();
			}
			else{
				u64 v=stoll(value);
				memcpy(result,&v,sizeof(u64));
			}
		}
		else
		if (algo=="u"){
				if (bitsize<=32){
					u32 v=(u32)stoul(value);//int - 32b, long - 64b
					memcpy(result,&v,sizeof(u32));
				}
				else{
					u64 v=stoull(value);
					memcpy(result,&v,sizeof(u64));
				}
			}
		else
		if ((algo=="d1f")||(algo=="d2f")||(algo=="d3f")){
			//float число которое следует * на 10 и представить как int
			u32 rate=10;
			if 	(algo=="d2f")
				rate=100;
			else {
			    if 	(algo=="d3f")
			        rate=1000;
			}

			if (bitsize<=32){
				f32 v=stof(value);
				s32 vd=(s32)(v*rate);
				memcpy(result,&vd,sizeof(s32));
			}
			else{
				d64 v=stold(value);
				s64 vd=(s64)(v*rate);
				memcpy(result,&vd,sizeof(s64));
			}
		}
        else
		if ((algo=="u1f")||(algo=="u2f")||(algo=="u3f")){
		    //float число которое следует * на 10 и представить как u32
		    u32 rate=10;
		    if 	(algo=="u2f")
		        rate=100;
		    else {
		        if 	(algo=="u3f")
		            rate=1000;
		    }

            if (bitsize<=32){
                f32 v=stof(value);
                u32 vd=(u32)(v*rate);
                memcpy(result,&vd,sizeof(u32));
            }
            else{
                d64 v=stold(value);
                u64 vd=(u64)(v*rate);
                memcpy(result,&vd,sizeof(u64));
            }
		}
		else
		if (algo[0]=='t'){
			u32 rate=60;
			if 	(algo[1]=='2')
				rate=3600;
			else
			if 	(algo[1]=='3')
				rate=86400;

			if (bitsize<=32){
					u32 v=stoul(value)/rate;
					memcpy(result,&v,sizeof(u32));
			}
			else{
					u64 v=stoull(value)/rate;
					memcpy(result,&v,sizeof(u64));
			}
		}
		else
		if ((algo=="ms")||(algo=="us")){
			u32 rate=1000;
			if (algo=="us")
				rate=1000000;
			d64 v=stold(value);
			u64 vd=(u64)(v*rate);
			memcpy(result,&vd,sizeof(u64));

		}
		else
		if (algo=="f"){
			f32 v=stold(value);
			memcpy(result,&v,sizeof(f32));
		}
		else
		if (algo=="s"){
			strcpy((char*)result,(char*)value.c_str());
		}
		else
		if (algo=="s64"){
			string s=base64_decode(value);
			memcpy((char*)result,(char*)s.c_str(),s.size());
			//printf("Decode \n");
			//printhex(result,s.size(),16);
		}
		else
			return ERROR;//special translate
	}
	return NO_ERROR;
}

eErrorTp DNKchainT::MC_ParsePackData_ConvValue(u8 * bin_value,u32 size,string algo,string & path,Json::Value & root_result){
	u32 bsize=size>>3;


	if (((bsize<<3)-size)!=0)
		bsize++;

	if (algo=="d"){
		if (size<=8){
			s8 var=0;
			memcpy(&var,bin_value,bsize);
			JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
		}
		if (size<=16){
			s16 var=0;
			memcpy(&var,bin_value,bsize);
			JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
		}
		if (size<=32){
			s32 var=0;
			memcpy(&var,bin_value,bsize);
			JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
		}else{
			s64 var=0;
			memcpy(&var,bin_value,bsize);
			JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
		}
	}
	else
	if (algo=="u"){
		if (size<32){
			u32 var=0;
			if ((size%8)==0)
				memcpy(&var,bin_value,bsize);
			else
				bitCopy((u8*)&var,0,size,bin_value,0);
			JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
		}else{
			u64 var=0;
			if ((size%8)==0)
				memcpy(&var,bin_value,bsize);
			else
				bitCopy((u8*)&var,0,size,bin_value,0);
			JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
		}
	}
	else
	if ((algo=="d1f")||(algo=="d2f")||(algo=="d3f")){
				//float число которое следует * на 10 и представить как int
		u32 rate=10;
		if 	(algo=="d2f")
			rate=100;
		else
		if 	(algo=="d3f")
			rate=1000;

		if (size<=8){
				s8 iv=0;
				memcpy(&iv,bin_value,bsize);
				f32 var=static_cast<f32>(iv)/rate;

				JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
				JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
		}
		else
		if (size<=16){
				s16 iv=0;
				memcpy(&iv,bin_value,bsize);
				f32 var=static_cast<f32>(iv)/rate;

				JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
				JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
		}
		else
		if (size<=32){
			s32 iv=0;
			memcpy(&iv,bin_value,bsize);
			f32 var=static_cast<f32>(iv)/rate;

			JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
		}
		else{
			s64 iv=0;
			memcpy(&iv,bin_value,bsize);
			d64 var=iv/rate;
			JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
		}
	}
	else
    if ((algo=="u1f")||(algo=="u2f")||(algo=="u3f")){
        //float число которое следует * на 10 и представить как int
        u32 rate=10;
        if 	(algo=="u2f")
            rate=100;
        else {
            if 	(algo=="u3f")
                rate=1000;
        }

        if (size<=8){
            u8 iv=0;
            memcpy(&iv,bin_value,bsize);
            f32 var=static_cast<f32>(iv)/rate;

            JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
            JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
        }
        else
            if (size<=16){
                u16 iv=0;
                memcpy(&iv,bin_value,bsize);
                f32 var=static_cast<f32>(iv)/rate;

                JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
                JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
            }
            else
                if (size<=32){
                    u32 iv=0;
                    memcpy(&iv,bin_value,bsize);
                    f32 var=static_cast<f32>(iv)/rate;

                    JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
                    JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
                }
                else{
                    u64 iv=0;
                    memcpy(&iv,bin_value,bsize);
                    d64 var=iv/rate;
                    JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
                    JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
                }
    }
	else
	if (algo[0]=='t'){
		u32 rate=60;
		if 	(algo[1]=='2')
			rate=3600;
		else
		if 	(algo[1]=='3')
			rate=86400;

		if (size<=32){
			u32 v=0;
			memcpy(&v,bin_value,bsize);
			u32 var=v*rate;
			JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
		}
		else{
			u64 v=0;
			memcpy(&v,bin_value,bsize);
			u64 var=v*rate;
			JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
		}
	}
	else
	if (algo=="a1"){
		Json::Value arr;
		u32 delta=0;
		u32 rezd=0;
		for (u32 n=0;n<size;n++){
			rezd=n>>3;
			delta=n-(rezd<<3);
			arr[n]=(bin_value[rezd]>>delta)&1;
		}
		JSON_SetValueOnStringPath(root,(char*)path.c_str(),arr);
		JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),arr);
	}
	else
	if (algo=="a2"){
		u32 i=0;
		u32 bytesize=size/8;
		Json::Value arr;
		for (u32 n=0;n<bsize;n++){
			arr[n]=bin_value[n];
		}
		JSON_SetValueOnStringPath(root,(char*)path.c_str(),arr);
		JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),arr);
	}
	else
	if ((algo=="ms")||(algo=="us")){
		u32 rate=1000;
		if (algo=="us")
			rate=1000000;
		if (size<=32){
			u32 v=0;
			memcpy(&v,bin_value,bsize);
			f32 var=static_cast<f32>(v)/rate;
			JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
		}
		else{
			u64 v=0;
			memcpy(&v,bin_value,bsize);
			d64 var=static_cast<d64>(v)/rate;
			JSON_SetValueOnStringPath(root,(char*)path.c_str(),var);
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),var);
		}
	}
	else
	if (algo=="f"){
		float v;
		memcpy(&v,bin_value,bsize);
		JSON_SetValueOnStringPath(root,(char*)path.c_str(),v);
		JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),v);
	}
	else
	if (algo=="s"){
		if (bin_value[bsize-1]==0){
			JSON_SetValueOnStringPath(root,(char*)path.c_str(),bin_value);
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),bin_value);
		}
		else{
			//char * s=new char[bsize+1];
			buffer s(bsize+1);
			memcpy(s.p(),bin_value,bsize);
			s.p()[bsize]=0;
			JSON_SetValueOnStringPath(root,(char*)path.c_str(),s.p());
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),s.p());
			//delete s;
		}
	}
	else
		if (algo=="s64"){
			//printf("Encode \n");
			//printhex(bin_value,bsize,16);
			string s=base64_encode((unsigned char const*)bin_value,bsize);
			JSON_SetValueOnStringPath(root,(char*)path.c_str(),s);
			JSON_SetValueOnStringPath(root_result,(char*)path.c_str(),s);
			//printf("Encode result %s\n",s.c_str());
		}
	else
		return ERROR;//special translate


	return NO_ERROR;
}

u32 DNKchainT::MC_CreatePackData(char * cmd_code,Json::Value & mc_cmd,u8 * buf, eErrorTp (*User_PackerValue)(u32,string,Json::Value,u8*,void * cb_context)){
	u32 bufsize=0;
	//Json::StyledWriter writer;
	u32 bsize=0;

	if (mc_cmd.isMember(cmd_code)){
		Json::Value mc=mc_cmd[cmd_code];

		for (const auto& key : mc.getMemberNames()) {

			u32 offset=stoul(key.c_str());
			u32 size=mc[key][0].asInt();
			Json::Value roott;

			JSON_GetObjectOnStringPath(root,roott,(char*)mc[key][2].asString().c_str());

			//u32 data=roott.asString();
			if (MC_CreatePackData_ConvValue(size,mc[key][1].asString(),roott,MC_CreatePackData_value_tmp)==ERROR){
				if (User_PackerValue!=NULL)
					User_PackerValue(size,mc[key][1].asString(),roott,MC_CreatePackData_value_tmp,this);
			}

			//cout << writer.write( roott ) << endl;
			//printf("Pack %s offset %d size %d data %d %s\n",key.c_str(), offset, size,data,(char*)mc[key][2].asString().c_str());

			bsize=bitCopy(buf,offset,size,MC_CreatePackData_value_tmp,0);

			if (bsize>bufsize)
				bufsize=bsize;
			//printf("bufsize %d bsize %d\n",bufsize,bsize);
		}
	}
	else
		return 0;

	return bufsize;
}

eErrorTp DNKchainT::MC_Create_ProtoDB_Standart(char * target,Json::Value & mc_name,Json::Value & mc_cmd){

	//Json::StyledWriter writer;

	string varname="";
	GMC_Json_Standart(target,root,mc_name,mc_cmd,varname);

	//cout << writer.write( mc_name ) << endl;

	//varname="";
	//Json::Value  mc_cmd;
	//GMC_Json_CMD_key(target,root,mc_cmd,varname);
	//cout << writer.write( mc_cmd ) << endl;


	return NO_ERROR;
}
void DNKchainT::GMC_Json_Standart(char * target,Json::Value root_src,Json::Value & mc_name,Json::Value & mc_cmd,string varname){
				for (const auto& key : root_src.getMemberNames()) {
					if (root_src[key].isObject()) {
						///printf("key %s\n",key.c_str());
						if ((key=="$")&&(root_src["$"].isMember("type"))){
							//printf("isMember(target) %s %s\n",key.c_str(),root_src["$"]["target"].asString().c_str());
							if (root_src["$"]["type"].asString()==target){

								//printf("path %s\n",varname.c_str());
								string vn;
								string cmd=root_src["$"]["cmd"].asString();
								for (const auto& keyf : root_src["$"]["frm"].getMemberNames()) {
									 vn=varname+'.'+keyf;
									 mc_name[vn][0]=cmd;//cmd
									 mc_name[vn][1]=root_src["$"]["frm"][keyf][0];//offset
									 mc_name[vn][2]=root_src["$"]["frm"][keyf][1];//size
									 mc_name[vn][3]=root_src["$"]["frm"][keyf][2];//type
									 mc_cmd[mc_name[vn][0].asString()][mc_name[vn][1].asString()][0]=mc_name[vn][2];

									 mc_cmd[mc_name[vn][0].asString()][mc_name[vn][1].asString()][0]=mc_name[vn][2];
									 mc_cmd[mc_name[vn][0].asString()][mc_name[vn][1].asString()][1]=mc_name[vn][3];
									 mc_cmd[mc_name[vn][0].asString()][mc_name[vn][1].asString()][2]=vn;
								}

							}
							//if ()
						}
						else{
							string nname;
							if (varname.size()>0)
								nname=varname+"."+key;
							else
								nname=key;
							GMC_Json_Standart(target,root_src[key],mc_name,mc_cmd,nname);
						}
					}
				}
			}
