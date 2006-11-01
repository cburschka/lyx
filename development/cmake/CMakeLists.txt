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
 	
 	# disable checked iterators for msvc release builds to get maximum speed
	set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} /D_SECURE_SCL=0")
else(MSVC)
	add_definitions(-DBOOST_USER_CONFIG="<config.h>")
endif(MSVC)

if(MSVC AND NOT disable-pch)
	configure_file(${CMAKE_SOURCE_DIR}/pcheaders.h    ${CMAKE_BINARY_DIR}/pcheaders.h)
	configure_file(${CMAKE_SOURCE_DIR}/config.C.cmake ${CMAKE_CURRENT_BINARY_DIR}/config_pch.C)
	macro(lyx_add_msvc_pch name_)
		SET_SOURCE_FILES_PROPERTIES(${${name_}_sources} PROPERTIES COMPILE_FLAGS "/Yuconfig.h /Fp\$(IntDir)/config.pch")
		SET_SOURCE_FILES_PROPERTIES(${CMAKE_BINARY_DIR}/config_pch.C PROPERTIES COMPILE_FLAGS "/Ycconfig.h /Fp\$(IntDir)/config.pch")
		set(${name_}_sources ${${name_}_sources} ${CMAKE_BINARY_DIR}/config_pch.C ${CMAKE_BINARY_DIR}/pcheaders.h)
		add_definitions(/DLYX_ENABLE_PCH)
	endmacro(lyx_add_msvc_pch)
	set(CMAKE_CXX_FLAGS_DEBUG    "${CMAKE_CXX_FLAGS_DEBUG}         /DLYX_PCH_STL /DLYX_PCH_BOOST /DLYX_PCH_QT4")
	set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE}       /DLYX_PCH_STL /DLYX_PCH_BOOST /DLYX_PCH_QT4")
	set(CMAKE_CXX_FLAGS_MINSIZEREL  "${CMAKE_CXX_FLAGS_MINSIZEREL} /DLYX_PCH_STL /DLYX_PCH_BOOST")
	
else(MSVC AND NOT disable-pch)
	set(disable-pch TRUE CACHE TYPE STRING)
	macro(lyx_add_msvc_pch)
	endmacro(lyx_add_msvc_pch)
endif(MSVC AND NOT disable-pch)


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
