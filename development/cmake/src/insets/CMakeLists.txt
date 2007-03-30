# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(insets)

file(GLOB insets_sources ${TOP_SRC_DIR}/src/insets/*.C)
file(GLOB insets_headers ${TOP_SRC_DIR}/src/insets/*.h)

list(REMOVE_ITEM insets_sources ${TOP_SRC_DIR}/src/insets/insettheorem.C)

lyx_add_msvc_pch(insets)

include_directories(${TOP_SRC_DIR}/src/insets)


if(NOT MERGE_FILES)
	add_library(insets ${library_type} ${insets_sources} ${insets_headers})
else(NOT MERGE_FILES)
	lyx_const_touched_files(_allinone const_touched_files  insets_sources)
	add_library(insets ${library_type} ${const_touched_files})
endif(NOT MERGE_FILES)


project_source_group("${GROUP_CODE}" insets_sources insets_headers)

