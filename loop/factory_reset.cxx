/*
 * factory_reset.cxx
 *
 *  Created on: 23 дек. 2018 г.
 *      Author: root
 */

#include "factory_reset.h"


Factory_reset::Factory_reset(eDebugTp debug_level):ThreadT("Fact_rst","fars",debug_level){
		GPRINT(HARD_LEVEL,"Create Factory_reset\n");
		JSON_ReadConfigField(CnT->json_cfg,"fars_loopdelay",loopdelay);
		JSON_ReadConfigField(CnT->json_cfg,"fars_detectinterval",detectinterval);
		JSON_ReadConfigField(CnT->json_cfg,"fars_reset_to_factory_on_gpio",reset_to_factory_on_gpio);
		JSON_ReadConfigField(CnT->json_cfg,"fars_reset_gpio",reset_gpio);
		JSON_ReadConfigField(CnT->json_cfg,"fars_reset_to_factory_on_key",reset_to_factory_on_key);
		JSON_ReadConfigField(CnT->json_cfg,"fars_reset_key",reset_key);
		JSON_ReadConfigField(CnT->json_cfg,"fars_reset_input_dev",reset_input_dev);
		JSON_ReadConfigField(CnT->json_cfg,"fars_reset_value",reset_value);
		if (reset_to_factory_on_key){
			if ((fd_input_dev = open(reset_input_dev, O_RDONLY|O_NONBLOCK)) == -1) {
					GPRINT(NORMAL_LEVEL,"Error open input event %s for factory reset key\n",reset_input_dev);
			}
		}
	}

Factory_reset::~Factory_reset (void){
		GPRINT(HARD_LEVEL,"Destroy Factory_reset\n");
	}

eErrorTp Factory_reset::Loop(void* thisPtr){
		//sSaved OldSaved;
	shared_ptr<gpio> rf_gpio;

	if (reset_to_factory_on_gpio)
		rf_gpio=make_shared<gpio>(IntToStr(reset_gpio),"in","0");

	u32 factory_rst_counter=detectinterval;
	bool press=false;
	while(1){
			//GPRINT(NORMAL_LEVEL,"LOOP\n");
			while (GetUcastFifoMessage(FifoPreBuf,Mheader)!=ERROR)
			{
				switch(Mheader.MsgType)
				{
					case srvmJNODA_READY:
							jnoda_ready=true;
					break;
					default:
					GPRINT(MEDIUM_LEVEL,"undef msg type [%d]\n",Mheader.MsgType);
				}
			}

			if (fd_input_dev!=-1){
				while (read(fd_input_dev, &ie, sizeof(struct input_event))>0){
					if ((ie.code==reset_key)){
						if (ie.value==reset_value){
							press=true;
						}
						else{
							press=false;
						}
					}
				}
			}

			//printf("%d\n",factory_rst_counter);
			if (reset_to_factory_on_gpio){
				if (rf_gpio->get()==0)
					press=true;
				else
					press=false;
			}

			if (press){
				factory_rst_counter--;
				//printf("%d\n",factory_rst_counter);
				if (factory_rst_counter==0){
					//led_show();
					//printf("Customer.ResetDeviceToFactory();\n");
					ResetDeviceToFactory();
				}
			}
			else
				factory_rst_counter=detectinterval;


			if (TERMReq){
				break;
			}


			mdelay(loopdelay);
		}

		return NO_ERROR;
	}

void ResetDeviceToFactory(void){
	string data="";
	if (CnT->trigResetDeviceToFactory==false){
		WriteStringFile((char*)string(CnT->CACHE_NECRON_PATH+"/"+ROLL_OUT_FACTORY_STUMP).c_str(),data);
		Customer.ResetDeviceToFactory();
		CnT->trigResetDeviceToFactory=true;
	}
}
