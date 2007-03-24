# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(graphics)

file(GLOB graphics_sources ${TOP_SRC_DIR}/src/graphics/*.C)
file(GLOB graphics_headers ${TOP_SRC_DIR}/src/graphics/*.h)

lyx_add_msvc_pch(graphics)


include_directories(${TOP_SRC_DIR}/src/graphics)

if(NOT MERGE_FILES)
	add_library(graphics STATIC ${graphics_sources} ${graphics_headers})
else(NOT MERGE_FILES)
	lyx_merge_files(allinone graphics_sources)
	add_library(graphics STATIC 
					${CMAKE_CURRENT_BINARY_DIR}/allinone.h
					${CMAKE_CURRENT_BINARY_DIR}/allinone_const.C
					${CMAKE_CURRENT_BINARY_DIR}/allinone_touched.C)
endif(NOT MERGE_FILES)

project_source_group("${GROUP_CODE}" graphics_sources graphics_headers)

