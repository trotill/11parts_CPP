#cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 17)

project(cnoda)
project(cnodakg)

project(svc)
project(necron)

if("${SSDP_SENDER}" STREQUAL "yes")
	project(ssdpd)
endif()

if("${WATCHDOG_STUB_ENABLE}" STREQUAL "yes")
	project(watchdog_stub)
endif()

find_package(PkgConfig REQUIRED)



pkg_check_modules(JSON_CPP REQUIRED jsoncpp)
include_directories(SYSTEM ${JSON_CPP_INCLUDE_DIRS})
link_directories(SYSTEM ${JSON_CPP_LIBRARY_DIRS})
add_definitions(${JSON_CPP_CFLAGS_OTHER})

pkg_check_modules(LIBMOUNT REQUIRED mount)
include_directories(SYSTEM ${LIBMOUNT_INCLUDE_DIRS})
link_directories(SYSTEM ${LIBMOUNT_LIBRARY_DIRS})
add_definitions(${LIBMOUNT_CFLAGS_OTHER})

# Location where cmake first looks for cmake modules.
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/build/")
set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables")

if("${GORCHAKOV_ENABLE}" STREQUAL "yes")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GORCHAKOV")
endif()

if("${SKEY}" STREQUAL "yes")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SKEY")
	set(SKEY_ADM src/engine/periphery/skey.cxx)
endif()

if("${SNMP_ENABLE}" STREQUAL "yes")
	set(SNMP src/engine/minisw/snmp.cxx)
	set(SNMP_ADM src/engine/proto/snmp_adm.cxx)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SNMP")
	set(SNMP_LIBS "-lnetsnmpmibs -lpci -ldl -lnetsnmpagent -Wl,-E -lnetsnmp -lcrypto -lm")
endif()

if("${MODBUSCPP_ENABLE}" STREQUAL "yes")
	set(MODBUSCPP src/engine/proto/modbuscpp.cxx)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_MODBUSCPP")
	set(LMODBUS "-lmodbus")
endif()

if("${MQTT_ENABLE}" STREQUAL "yes")
	set(MQTT src/engine/proto/mqtt.cxx)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_MQTT")
	set(LPAHO_MQTT3CS "-lpaho-mqtt3c")
endif()

if ("${HIREDIS_ENABLE}" STREQUAL "yes")
	pkg_check_modules(HIREDIS REQUIRED hiredis)
	include_directories(SYSTEM ${HIREDIS_INCLUDE_DIRS})
	link_directories(SYSTEM ${HIREDIS_LIBRARY_DIRS})
	add_definitions(${HIREDIS_CFLAGS_OTHER})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_HIREDIS")
endif()

if ("${SQLITE_ENABLE}" STREQUAL "yes")
	pkg_check_modules(SQLITE REQUIRED sqlite3)
	include_directories(SYSTEM ${SQLITE_INCLUDE_DIRS})
	link_directories(SYSTEM ${SQLITE_LIBRARY_DIRS})
	add_definitions(${SQLITE_CFLAGS_OTHER})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SQLITE")
	
	#set(SQLITE_LIBRARIES "${SQLITE_LIBRARIES} -lvsqlitepp")
	
endif()

if ("${ZEROMQ_ENABLE}" STREQUAL "yes")
	pkg_check_modules(ZEROMQ REQUIRED libzmq )
	include_directories(SYSTEM ${ZEROMQ_INCLUDE_DIRS})
	link_directories(SYSTEM ${ZEROMQ_LIBRARY_DIRS})
	add_definitions(${ZEROMQCFLAGS_OTHER})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_ZEROMQ")
	set(LZEROMQ "-lzmq")
endif()

if ("${SECURE_ENABLE}" STREQUAL "yes")
	add_compile_definitions(_SECURE_ENABLE)
	add_compile_definitions(_SECURE_UID="${SECURE_UID}")
	add_compile_definitions(_SECURE_SNP="${SECURE_SNP}")
	add_compile_definitions(_SECURE_COMBINATION="${SECURE_COMBINATION}")
	set(SKEY_ADM_V2 src/engine/periphery/skeyV2.cxx)
endif()

set(SAFE_LOGGER_ENABLE "yes")

if("${SAFE_LOGGER_ENABLE}" STREQUAL "yes")
	pkg_check_modules(ZLIB REQUIRED zlib)
	include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
	link_directories(SYSTEM ${ZLIB_LIBRARY_DIRS})
	add_definitions(${ZLIB_CFLAGS_OTHER})
	
	set(SAFE_LOGGER_BASE src/engine/minisw/safe_logger.cxx)
	
	set(SAFE_LOGGER_ALGOBI src/engine/logger/algo_biphasic_offset.cxx)
	set(SAFE_LOGGER_ALGOgQLITE src/engine/logger/algo_sqlite.cxx src/engine/logger/algo_sqlite_cli.cxx)
	set(SAFE_LOGGER_ALGODNK src/engine/logger/dnk_biphasic_offset.cxx)
	set(SAFE_LOGGER_ALGODNK_CLI src/engine/logger/dnk_biphasic_offset_cli.cxx)
	set(SAFE_LOGGER_M src/engine/logger/safe_logger.cxx)
	set(SAFE_LOGGER_C src/engine/logger/safe_logger_client.cxx)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SAFE_LOGGER") 
	set(SAFE_LOGGER ${SAFE_LOGGER_BASE} ${SAFE_LOGGER_ALGOBI} ${SAFE_LOGGER_ALGODNK} ${SAFE_LOGGER_ALGODNK_CLI} ${SAFE_LOGGER_M} ${SAFE_LOGGER_C} ${SAFE_LOGGER_ALGOgQLITE})
endif()

if("${SSDP_SENDER}" STREQUAL "yes")
	set(SSDPD_CONTENT src/engine/proto/ssdpd/ssdpd.c
		src/engine/proto/ssdpd/web.c
		src/engine/proto/ssdpd/pidfile.c
		src/engine/proto/ssdpd/md5.c
	)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_POgIX_C_SOURCE=200112L -D_DEFAULT_SOURCE") 
endif()

if("${FTP_CLIENT_ENABLE}" STREQUAL "yes")
	set(CURL_LIBRARY "-lcurl")
	find_package(CURL REQUIRED)
	include_directories(${CURL_INCLUDE_DIRS})

	set(FTP_CLIENT src/engine/proto/ftp/CurlHandle.cpp src/engine/proto/ftp/FTPClient.cpp)
endif()

set(SVC_CONTENT src/engine/minisw/service/svc.cxx
	src/engine/settings_adm.cxx
	src/engine/thread_basic.cxx
	src/engine/thread.cxx
	src/engine/print.cxx
	src/engine/memadmin.cxx
	src/engine/logger/safe_logger_client.cxx
	src/engine/fifo.cxx
	src/engine/ipc_socket.cxx
	src/engine/lib/11p_string.cxx
  	src/engine/lib/11p_process.cxx
  	src/engine/lib/11p_text_convert.cxx
  	src/engine/lib/11p_files.cxx
  	src/engine/lib/11p_bin.cxx
  	src/engine/lib/11p_time.cxx
  	src/engine/proto/json_proto.cxx
  	src/engine/lib/11p_json.cxx
  	src/engine/algo/crc.cxx
  	src/engine/algo/MD5.cxx
  	src/engine/minisw/service/syslib.cxx
  	src/engine/algo/base64.cxx
)

set(CNODAKG_CONTENT src/engine/minisw/necron/cnodakg.cxx
	src/engine/algo/aes.cxx
	src/engine/algo/base64.cxx
	src/engine/algo/MD5.cxx
	src/engine/periphery/skeyV2.cxx
  	src/engine/lib/11p_string.cxx
  	src/engine/lib/11p_process.cxx
  	src/engine/lib/11p_text_convert.cxx
  	src/engine/lib/11p_files.cxx
  	src/engine/lib/11p_bin.cxx
  	src/engine/lib/11p_time.cxx
)
set(NECRON_CONTENT src/engine/minisw/necron/necron.cxx
	src/engine/periphery/watchdog.cxx
	src/engine/print.cxx
	src/engine/memadmin.cxx
	src/engine/thread_basic.cxx
	src/engine/lib/11p_process.cxx
	src/engine/lib/11p_string.cxx
	src/engine/lib/11p_json.cxx
	src/engine/lib/11p_files.cxx
	src/engine/minisw/service/syslib.cxx
)
	
set(LIB_CONTENT src/engine/node.cxx
	src/engine/engine.cxx
	src/engine/periphery/adapter.cxx
	src/engine/periphery/abstraction.cxx
	src/engine/periphery/abstraction_mc.cxx
	src/engine/periphery/abstraction_sast.cxx
	src/engine/proto/ethernet_multicast.cxx
	src/engine/settings_adm.cxx
	src/engine/thread_basic.cxx
	src/engine/thread.cxx
	src/engine/loop/mq_collector.cxx
	src/engine/loop/reporter_admin.cxx
	src/engine/loop/jnoda_adapter.cxx
	src/engine/loop/websrv_adapter.cxx
	src/engine/loop/safe_storage.cxx
	src/engine/loop/watchdog_admin.cxx
	src/engine/loop/factory_reset.cxx
	src/engine/loop/guard_loop.cxx
	src/engine/loop/update_admin.cxx
	src/engine/loop/mbus_loop.cxx
	src/engine/loop/mbusLoopChannel.cxx	
	src/engine/ipc_socket.cxx
	src/engine/global.cxx
	src/engine/print.cxx
	src/engine/algo/base64.cxx
	src/engine/algo/aes.cxx
  	src/engine/algo/crc.cxx
  	src/engine/algo/MD5.cxx
  	src/engine/minisw/minisw.cxx
  	${SNMP}
  	${SNMP_ADM}
  	${MQTT}
  	${SKEY_ADM}
  	${FTP_CLIENT}
  	 # src/engine/minisw/snmp.cxx
  	src/engine/minisw/usb_reset.cxx
  	src/engine/minisw/imcheck.cxx
  	src/engine/minisw/evedt_evnode.cxx
  	src/engine/minisw/factory.cxx
  	src/engine/minisw/gprivate.cxx
  	src/engine/minisw/gset.cxx
  	src/engine/minisw/setclean.cxx
  	src/engine/minisw/sset.cxx
  	${SAFE_LOGGER}
  	#src/engine/minisw/safe_logger.cxx
  	src/engine/periphery/tty.cxx
  	src/engine/periphery/gpio.cxx
  	src/engine/periphery/watchdog.cxx
  	src/engine/memadmin.cxx 
  	${MODBUSCPP}
  	#src/engine/proto/modbuscpp.cxx
  	src/engine/proto/json_proto.cxx
  	src/engine/auth.cxx
  	src/engine/secure_access.cxx
  	src/engine/update/update.cxx
  	src/engine/fifo.cxx
  	src/engine/node_req.cxx
  	src/engine/lib/11p_string.cxx
  	src/engine/lib/11p_process.cxx
  	src/engine/lib/11p_text_convert.cxx
  	src/engine/lib/11p_files.cxx
  	src/engine/lib/11p_bin.cxx
  	src/engine/lib/11p_time.cxx
  	src/engine/lib/11p_json.cxx
    src/custom_project/protected/config_node.cxx
    ${SKEY_ADM_V2}
    )

set(WADCHDOG_STUB_CONTENT
	src/engine/minisw/watchdog_stub.cxx
	src/engine/periphery/watchdog.cxx
	src/engine/lib/11p_process.cxx
	src/engine/lib/11p_string.cxx
	src/engine/print.cxx
	src/engine/memadmin.cxx
)
add_library(cnoda SHARED ${LIB_CONTENT})

set_target_properties(cnoda PROPERTIES SOVERSION 1)
target_include_directories(cnoda PRIVATE src/engine src/custom_project)

if("${SSDP_SENDER}" STREQUAL "yes")
	add_executable(ssdpd WIN32 ${SSDPD_CONTENT})
	set_target_properties(ssdpd PROPERTIES SOVERSION 1)
	target_include_directories(ssdpd PRIVATE src/engine/proto/ssdpd)
endif()

	add_executable(cnodakg WIN32 ${CNODAKG_CONTENT})
	set_target_properties(cnodakg PROPERTIES SOVERSION 1)
	target_include_directories(cnodakg PRIVATE src/engine/minisw/necron)
	target_compile_definitions(cnodakg PUBLIC -D CNODAKG)
	
	add_executable(svc WIN32 ${SVC_CONTENT})
	set_target_properties(svc PROPERTIES SOVERSION 1)
	target_include_directories(svc PRIVATE src/engine/minisw/service)
	
	add_executable(necron WIN32 ${NECRON_CONTENT})
	set_target_properties(necron PROPERTIES SOVERSION 1)
	target_include_directories(necron PRIVATE src/engine/minisw/necron)
	
if("${WATCHDOG_STUB_ENABLE}" STREQUAL "yes")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WO_CNT_DEP") 
	add_executable(watchdog_stub WIN32 ${WADCHDOG_STUB_CONTENT})
	set_target_properties(watchdog_stub PROPERTIES SOVERSION 1)
	target_include_directories(watchdog_stub PRIVATE src/engine)
endif()

include_directories(src)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe -Wall -Wno-unused-variable -Wno-write-strings -Wextra -Wno-switch -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -Wall -Wno-unused-variable -Wno-write-strings -fstack-protector-strong -Wno-switch -Wno-comment -std=c++17")

set(CMAKE_C_FLAGS_DEBUG "-Og -g -ggdb -pg")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Og -g -ggdb -pg")
set(CMAKE_C_FLAGS_RELEASE "-Og")
set(CMAKE_C_FLAGS_MINSIZEREL "-Og")

set(CMAKE_CXX_FLAGS_DEBUG "-Og -g -ggdb -pg")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Og -g -ggdb -pg")
set(CMAKE_CXX_FLAGS_RELEASE "-Og")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Og")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message(STATUS "GNU v${CMAKE_CXX_COMPILER_VERSION}")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
        message(FATAL_ERROR "GCC version must be at least 4.8!")
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wconditionally-supported ")
    endif()
    if ((CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 5.0) OR (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.0))
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wswitch-bool ")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wswitch-bool ")
    endif()
    if ((CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 6.0) OR (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0))
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-show-caret -ftrack-macro-expansion ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-show-caret -ftrack-macro-expansion -Wnoexcept -Wstrict-null-sentinel")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message(STATUS "Clang v${CMAKE_CXX_COMPILER_VERSION}")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.6)
        message(FATAL_ERROR "Clang version must be at least 3.6!")
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wswitch-bool -Wnull-dereference -Wunused-const-variable ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wswitch-bool -Wnull-dereference -Wunused-const-variable -std=c++14 ")
else()
    message(WARNING "You are using an unsupported compiler! Compilation has only been tested with Clang and GCC.")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

set(LIBRARY ${JSON_CPP_LIBRARIES} "-lpthread" ${LPAHO_MQTT3CS} ${LIBMOUNT_LIBRARIES} ${ZLIB_LIBRARIES} ${CURL_LIBRARIES} ${HIREDIS_LIBRARIES} ${SQLITE_LIBRARIES} ${SNMP_LIBS} ${LMODBUS} ${LZEROMQ})

if("${WATCHDOG_STUB_ENABLE}" STREQUAL "yes")
	target_link_libraries(watchdog_stub ${JSON_CPP_LIBRARIES})
	install(TARGETS watchdog_stub RUNTIME DESTINATION "${INSTALL_BIN_DIR}")
endif()

if("${SSDP_SENDER}" STREQUAL "yes")
    target_link_libraries(ssdpd "-lm")
	install(TARGETS ssdpd RUNTIME DESTINATION "${INSTALL_BIN_DIR}")	
endif()

target_link_libraries(svc ${JSON_CPP_LIBRARIES} ${LIBMOUNT_LIBRARIES} "-lpthread" ${HIREDIS_LIBRARIES} ${ZLIB_LIBRARIES})
install(TARGETS svc RUNTIME DESTINATION "${INSTALL_BIN_DIR}")	

target_link_libraries(necron ${JSON_CPP_LIBRARIES} ${LIBMOUNT_LIBRARIES} "-lpthread" ${ZLIB_LIBRARIES})
install(TARGETS necron RUNTIME DESTINATION "${INSTALL_BIN_DIR}")	

target_link_libraries(cnodakg ${JSON_CPP_LIBRARIES} ${LIBMOUNT_LIBRARIES} "-lpthread" ${ZLIB_LIBRARIES})
install(TARGETS cnodakg RUNTIME DESTINATION "${INSTALL_BIN_DIR}")	

target_link_libraries(cnoda ${LIBRARY})

#INSTALL(TARGETS cnoda 
#        LIBRARY DESTINATION usr/lib
#        PUBLIC_HEADER DESTINATION usr/include
#)