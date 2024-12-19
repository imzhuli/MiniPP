cmake_minimum_required(VERSION 3.14)
set(libname ppprotocol)
project (lib_${libname})

include_directories(
	../src_lib_common/
	${LOCAL_3RD_INCLUDE_DIR}
	${X_INCLUDE_DIRS}
)

# local libs
aux_source_directory("./pp_protocol"                        SRC)
aux_source_directory("./pp_protocol/app_status"             SRC)
aux_source_directory("./pp_protocol/audit_terminal"         SRC)
aux_source_directory("./pp_protocol/backend"                SRC)
aux_source_directory("./pp_protocol/config_center"          SRC)
aux_source_directory("./pp_protocol/common"                 SRC)
aux_source_directory("./pp_protocol/dns_server"             SRC)
aux_source_directory("./pp_protocol/geo_server"             SRC)
aux_source_directory("./pp_protocol/proxyaccess_relay"      SRC)
aux_source_directory("./pp_protocol/relay_terminal"         SRC)

add_library(${libname} ${SRC})
target_compile_definitions(${libname} PRIVATE X_OPTION_STATIC)

install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/
	DESTINATION include
	FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*.hpp"
	)

install(
	TARGETS
	${libname}
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
)

