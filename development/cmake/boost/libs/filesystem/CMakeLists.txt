# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(boost_filesystem)

set(boost_filesystem_sources
	exception.cpp
	operations.cpp
	path.cpp
	portability.cpp
)

lyx_add_path(boost_filesystem_sources ${TOP_SRC_DIR}/boost/libs/filesystem/src)

if(NOT MERGE_FILES)
	add_library(boost_filesystem STATIC ${boost_filesystem_sources})
else(NOT MERGE_FILES)
	add_definitions(-D_WIN32_WINNT=0x500)
	lyx_merge_files(${CMAKE_CURRENT_BINARY_DIR}/boost_filesystem_allinone.C boost_filesystem_sources)
	add_library(boost_filesystem STATIC ${CMAKE_CURRENT_BINARY_DIR}/boost_filesystem_allinone.C)
endif(NOT MERGE_FILES)

