/*
 * jnoda_adapter.cxx
 *
 *  Created on: 15 дек. 2018 г.
 *      Author: root
 */


#include "loop/jnoda_adapter.h"

Jnoda_adapter::Jnoda_adapter(eDebugTp debug_level):ThreadT("Jnoda_ad","jnad",debug_level){
		GPRINT(HARD_LEVEL,"Create Jnoda_adapter\n");
		 JSON_ReadConfigField(CnT->json_cfg,"jnad_loopdelay",loopdelay);
		 NRT=make_shared<NodeReqT>(debug_level);
}

eErrorTp Jnoda_adapter::Loop(void* thisPtr){
		sInterThrMsgHeader Mheader;

		if (skey)
			auth_SyncAuth(this,NRT->Group);
		while(1){

			while (NRT->ReadNetUDP()!=0)
			{
				NRT->RND_Parse();
			}

			while (GetUcastFifoMessage(FifoPreBuf,Mheader)!=ERROR)
					{

					 GPRINT(PARANOIDAL_LEVEL,"Get MsgType %d\n",Mheader.MsgType);
						switch(Mheader.MsgType)
						{
							case srvmAPPLY_ONE:
								NRT->ApplySetting((char*)FifoPreBuf.p());
								break;
							case srvmJNODA_READY:
								jnoda_ready=true;
											//printf("SRV_HELPER JNODA_READY\n");
								break;
						}
					}


			if (TERMReq)
			{
				break;
			}
			mdelay(loopdelay);
		}
		return NO_ERROR;
	}
