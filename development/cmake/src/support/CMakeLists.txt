# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(support)

file(GLOB support_sources ${TOP_SRC_DIR}/src/support/${LYX_CPP_FILES})
file(GLOB support_headers ${TOP_SRC_DIR}/src/support/${LYX_HPP_FILES})

file(GLOB support_minzip_sources ${TOP_SRC_DIR}/src/support/minizip/*.c)
file(GLOB support_min_zip_headers ${TOP_SRC_DIR}/src/support/minizip/*.h)

list(REMOVE_ITEM support_sources 	
	${TOP_SRC_DIR}/src/support/os_win32.cpp
	${TOP_SRC_DIR}/src/support/os_unix.cpp
	${TOP_SRC_DIR}/src/support/os_cygwin.cpp
	${TOP_SRC_DIR}/src/support/os_os2.C
	${TOP_SRC_DIR}/src/support/atexit.c
	${TOP_SRC_DIR}/src/support/strerror.c
	${TOP_SRC_DIR}/src/support/minizip/iowin32.h
	${TOP_SRC_DIR}/src/support/minizip/iowin32.c)

lyx_add_msvc_pch(support)
	
include_directories(${TOP_SRC_DIR}/src/support
	${TOP_SRC_DIR}/src/support/minizip
	${QT_INCLUDE_DIR} ${QT_INCLUDE_DIR}/QtCore
	${ICONV_INCLUDE_DIR})


if(NOT MERGE_FILES)
	set(support_sources ${support_sources} ${support_minzip_sources})
	set(support_headers ${support_headers} ${support_minzip_headers})
	add_library(support ${library_type} ${support_sources} ${support_headers})
else()
	# move to front
	list(REMOVE_ITEM support_sources ${TOP_SRC_DIR}/src/support/convert.cpp)
	list(APPEND support_sources ${TOP_SRC_DIR}/src/support/convert.cpp)
	list(REMOVE_ITEM support_sources ${TOP_SRC_DIR}/src/support/lstrings.cpp)
	list(APPEND support_sources ${TOP_SRC_DIR}/src/support/lstrings.cpp)
	list(REVERSE support_sources)
	lyx_const_touched_files(_allinone  support_sources)
	add_library(support ${library_type} ${_allinone_files}
		${support_minzip_sources})
endif()


target_link_libraries(support boost_signals)

if(WIN32)
	target_link_libraries(support shlwapi)
endif()

project_source_group("${GROUP_CODE}" support_sources support_headers)

