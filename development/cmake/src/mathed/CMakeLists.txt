# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(mathed)

file(GLOB mathed_sources ${TOP_SRC_DIR}/src/mathed/*.C)
file(GLOB mathed_headers ${TOP_SRC_DIR}/src/mathed/*.h)

list(REMOVE_ITEM mathed_sources
	${TOP_SRC_DIR}/src/mathed/InsetMathXYArrow.C
	${TOP_SRC_DIR}/src/mathed/InsetFormulaMacro.C)

lyx_add_msvc_pch(mathed)

include_directories(${TOP_SRC_DIR}/src/mathed)

if(NOT MERGE_FILES)
	add_library(mathed STATIC ${mathed_sources} ${mathed_headers})
else(NOT MERGE_FILES)
	lyx_merge_files(${CMAKE_CURRENT_BINARY_DIR}/mathed_allinone.C mathed_sources)
	add_library(mathed STATIC ${CMAKE_CURRENT_BINARY_DIR}/mathed_allinone.C)
endif(NOT MERGE_FILES)


project_source_group("${GROUP_CODE}" mathed_sources mathed_headers)

