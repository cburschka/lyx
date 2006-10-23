# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

set (EXECUTABLE_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/bin)

set(PACKAGE lyx)
set(PACKAGE_VERSION 1.5.0svn)
set(LYX_DATE "not released yet")
#TODO
set(VERSION_INFO "CMake Build")

#TODO: add command line option
#      enable by default because the cmake 
#      build is mostly used for development
set(ENABLE_ASSERTIONS 1)

if(WIN32)
	set(USE_WINDOWS_PACKAGING 1)
else(WIN32)
	set(USE_POSIX_PACKAGING 1)
endif(WIN32)

if(NOT GROUP_CODE)
	#set(GROUP_CODE "The Golden Code")
	set(GROUP_CODE flat)
endif(NOT GROUP_CODE)

include(LyXPaths)
include(LyXMacros)
include(ProjectSourceGroup)

if(release)
	set(CMAKE_BUILD_TYPE Release)
	set(release)
endif(release)	


set(qt_postfix qt4)
project(lyx-${qt_postfix})
find_package(Qt4 REQUIRED)

find_package(ZLIB REQUIRED)
find_package(ICONV REQUIRED)
add_definitions(-DHAVE_ICONV=1)

if(all OR aspell)
	set(aspell TRUE CACHE TYPE STRING)
	find_package(ASPELL REQUIRED)
else(all OR aspell)
	find_package(ASPELL)
endif(all OR aspell)
set(aspell)

message("")
if(nls OR all)
	set(nls TRUE CACHE TYPE STRING)
	add_definitions(-DENABLE_NLS=1)
	message("----- Building with ENABLE_NLS")
else(nls OR all)	
	message("----- No nls, to enable use -Dnls=1")
endif(nls OR all)
set(nls)
if(ASPELL_FOUND)
	add_definitions(-DUSE_ASPELL=1)
	message("----- Building with USE_ASPELL")
else(ASPELL_FOUND)	
	message("----- No aspell, to get more information use -Daspell=1")
endif(ASPELL_FOUND)
message("")
set(all)

# create config.h
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h )


if(MSVC)
	if(MSVC_IDE)
		add_definitions(-DBOOST_USER_CONFIG=&lt\;config.h&gt\;)
	else(MSVC_IDE)
		add_definitions(-DBOOST_USER_CONFIG="<config.h>")
		SET(CMAKE_EXE_LINKER_FLAGS /MANIFEST)
	endif(MSVC_IDE)

 	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)	

else(MSVC)
	add_definitions(-DBOOST_USER_CONFIG="<config.h>")
endif(MSVC)

if(pch AND MSVC)
	set(pch TRUE CACHE TYPE STRING)

	macro(lyx_add_msvc_pch _sources)
		SET_SOURCE_FILES_PROPERTIES(${${_sources}} PROPERTIES COMPILE_FLAGS 
			"/Yuconfig.h /Fp${CMAKE_BINARY_DIR}/config.pch /Fd${CMAKE_BINARY_DIR}/pchlib.pdb")
	endmacro(lyx_add_msvc_pch)
	
	configure_file(config.C.cmake ${CMAKE_BINARY_DIR}/config.C)
	SET_SOURCE_FILES_PROPERTIES(${CMAKE_BINARY_DIR}/config.C PROPERTIES COMPILE_FLAGS  
		"/Ycconfig.h /Fp${CMAKE_BINARY_DIR}/config.pch /Fd${CMAKE_BINARY_DIR}/pchlib.pdb")
		
	include_directories(${CMAKE_CURRENT_BINARY_DIR} ${TOP_SRC_DIR}/src/support ${ICONV_INCLUDE_DIR})
	add_library(pchlib STATIC ${CMAKE_BINARY_DIR}/config.C)
	
	set(pchlibname pchlib )
	set(PRECOMPILED_HEADERS TRUE)
else(pch AND MSVC)
	macro(lyx_add_msvc_pch)
	endmacro(lyx_add_msvc_pch)
	set(pchlibname boost_signals) # dummy 
endif(pch AND MSVC)


#TODO: insource is not the best place
configure_file(${TOP_SRC_DIR}/lib/lyx2lyx/lyx2lyx_version.py.in 
               ${TOP_SRC_DIR}/lib/lyx2lyx/lyx2lyx_version.py)
     
include_directories( 
	${CMAKE_BINARY_DIR} 
	${TOP_SRC_DIR}/src 
	${TOP_SRC_DIR}/boost 
	${QT_INCLUDES} 
)

add_subdirectory(boost)
add_subdirectory(intl)
add_subdirectory(src)
