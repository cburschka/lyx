# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(frontends)

include_directories(${TOP_SRC_DIR}/src/frontends)

add_subdirectory(qt4) 

file(GLOB frontends_sources ${TOP_SRC_DIR}/src/frontends/${LYX_CPP_FILES})
file(GLOB frontends_headers ${TOP_SRC_DIR}/src/frontends/${LYX_HPP_FILES})

lyx_add_msvc_pch(frontends)


if(NOT MERGE_FILES)
	add_library(frontends ${library_type} ${frontends_sources} ${frontends_headers})
else()
	lyx_const_touched_files(_allinone  frontends_sources)
	add_library(frontends ${library_type} ${_allinone_files})
endif()

project_source_group("${GROUP_CODE}" frontends_sources frontends_headers)

