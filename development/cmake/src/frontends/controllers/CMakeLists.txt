# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(controllers)

file(GLOB controllers_sources
	${TOP_SRC_DIR}/src/frontends/controllers/${LYX_CPP_FILES})
file(GLOB controllers_headers
	${TOP_SRC_DIR}/src/frontends/controllers/${LYX_HPP_FILES})

lyx_add_msvc_pch(controllers)

include_directories(${TOP_SRC_DIR}/src/frontends/controllers)

if(NOT MERGE_FILES)
	add_library(controllers ${library_type}
		${controllers_sources} ${controllers_headers})
else()
	lyx_const_touched_files(_allinone  controllers_sources)
	add_library(controllers ${library_type} ${_allinone_files})
endif()

target_link_libraries(controllers boost_regex boost_filesystem)

project_source_group("${GROUP_CODE}" controllers_sources controllers_headers)

