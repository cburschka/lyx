# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(frontends)

include_directories(${TOP_SRC_DIR}/src/frontends)

add_subdirectory(controllers) 
add_subdirectory(${qt_postfix}) 


file(GLOB frontends_sources ${TOP_SRC_DIR}/src/frontends/*.C)
file(GLOB frontends_headers ${TOP_SRC_DIR}/src/frontends/*.h)

lyx_add_msvc_pch(frontends)


if(NOT MERGE_FILES)
	add_library(frontends STATIC ${frontends_sources} ${frontends_headers})
else(NOT MERGE_FILES)
	lyx_merge_files(allinone frontends_sources)
	add_library(frontends STATIC 					
					${CMAKE_CURRENT_BINARY_DIR}/allinone_const.C
					${CMAKE_CURRENT_BINARY_DIR}/allinone_touched.C)
endif(NOT MERGE_FILES)

project_source_group("${GROUP_CODE}" frontends_sources frontends_headers)

