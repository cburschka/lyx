# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(support)

configure_file(${TOP_SRC_DIR}/src/support/package.C.in 
               ${CMAKE_CURRENT_BINARY_DIR}/package.C)

file(GLOB support_sources ${TOP_SRC_DIR}/src/support/*.C)
file(GLOB support_headers ${TOP_SRC_DIR}/src/support/*.h)

list(REMOVE_ITEM support_sources 	
	${TOP_SRC_DIR}/src/support/os_win32.C
	${TOP_SRC_DIR}/src/support/os_unix.C
	${TOP_SRC_DIR}/src/support/os_cygwin.C
	${TOP_SRC_DIR}/src/support/os_os2.C
	${TOP_SRC_DIR}/src/support/atexit.c
	${TOP_SRC_DIR}/src/support/strerror.c)

set(support_sources ${support_sources} ${CMAKE_CURRENT_BINARY_DIR}/package.C)

lyx_add_msvc_pch(support)
	
include_directories(${TOP_SRC_DIR}/src/support ${ICONV_INCLUDE_DIR})

add_library(support STATIC ${support_sources} ${support_headers})

target_link_libraries(support boost_signals)

if(WIN32)
	target_link_libraries(support shlwapi)
endif(WIN32)

project_source_group("${GROUP_CODE}" support_sources support_headers)

