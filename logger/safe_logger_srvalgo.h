/*
 * safe_logger_srvalgo.h
 *
 *  Created on: 5 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_LOGGER_SAFE_LOGGER_SRVALGO_H_
#define SRC_ENGINE_LOGGER_SAFE_LOGGER_SRVALGO_H_

//#include "cnoda.h"
#include <cstdarg>
#include <engine/gzip/compress.hpp>
#include <engine/gzip/config.hpp>
#include <engine/gzip/decompress.hpp>
#include <engine/gzip/utils.hpp>
#include <engine/gzip/version.hpp>
#include "engine/lib/11p_files.h"
#include "engine/print.h"
#include "engine/lib/11p_string.h"

#define DNK_BIPHASIC_PREFIX "JDB"
#define FORMAT_TABLE_FILENAME "json_format"

enum {AAL_SAVE_RAW,AAL_SAVE_RAW_WDATE,AAL_SAVE_QUOTES_COMMA};

class safelogger_server;

class algo_abstract {
	public:
	virtual eErrorTp tick(){
		printf("Warning: not define tick method\n");
		return NO_ERROR;
	}
	virtual eErrorTp save(u8 * buf){
		printf("Warning: not define save method\n");
		return NO_ERROR;
	}
	virtual eErrorTp buildlog(){
		printf("Warning: not define buildlog method\n");
		return NO_ERROR;
	}
	virtual ~algo_abstract(){

	}
	string tmpdir;
	string tmpfile;
	string logprefix="aal";

};

eErrorTp MakeObjectFromDataAndHeader(Json::Value & data_root,Json::Value & header_root,Json::Value & result_root,bool ts_only);
eErrorTp MakeObjectFromDataAndHeader_rpd(rapidjson::Document & data_root,rapidjson::Document & header_root,rapidjson::Document & result_root,bool ts_only);
eErrorTp ReadJsonFormatFile(Json::Value & header_root,string * dal_format_table_file);
eErrorTp ReadJsonFormatFile_rpd(rapidjson::Document & header_root,string * dal_format_table_file);

#endif /* SRC_ENGINE_LOGGER_SAFE_LOGGER_SRVALGO_H_ */
