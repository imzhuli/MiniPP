cmake_minimum_required(VERSION 3.14)
set(libname ppcomponent)
project (lib_${libname})

include_directories(
	../src_lib_common/
	../src_lib_protocol/
	${LOCAL_3RD_INCLUDE_DIR}
	${X_INCLUDE_DIRS}
)

# local libs
aux_source_directory("./pp_component"                SRC)
aux_source_directory("./pp_component/dns"            SRC)
aux_source_directory("./pp_component/client_entry"   SRC)


aux_source_directory("./ppc_common/"   SRC)
aux_source_directory("./ppc_geo/"      SRC)

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

