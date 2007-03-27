# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(controllers)

file(GLOB controllers_sources ${TOP_SRC_DIR}/src/frontends/controllers/*.C)
file(GLOB controllers_headers ${TOP_SRC_DIR}/src/frontends/controllers/*.h)

lyx_add_msvc_pch(controllers)

include_directories(${TOP_SRC_DIR}/src/frontends/controllers)

if(NOT MERGE_FILES)
	add_library(controllers ${library_type} ${controllers_sources} ${controllers_headers})
else(NOT MERGE_FILES)
	lyx_merge_files(allinone controllers_sources)
	add_library(controllers ${library_type} 
					${CMAKE_CURRENT_BINARY_DIR}/allinone_const.C
					${CMAKE_CURRENT_BINARY_DIR}/allinone_touched.C)
endif(NOT MERGE_FILES)

target_link_libraries(controllers boost_regex boost_filesystem)

project_source_group("${GROUP_CODE}" controllers_sources controllers_headers)

