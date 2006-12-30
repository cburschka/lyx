# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.4 FATAL_ERROR)


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
set(all)

if(WIN32)
	if(noconsole)
		set(noconsole TRUE CACHE TYPE STRING)
		set(WIN32_CONSOLE WIN32)
		set(LYX_QTMAIN_LIBRARY ${QT_QTMAIN_LIBRARY})
		message("----- Console disabled")
	else(noconsole)
		message("----- Console enabled, disable it with -Dnoconsole=1")
	endif(noconsole)
endif(WIN32)

set(LYX_DIR         "")
set(TOP_SRCDIR      "${TOP_SRC_DIR}")
set(PROGRAM_SUFFIX  "")
set(LOCALEDIR       "")

message("")
message("----- LYX_DIR       : ${LYX_DIR}")
message("----- TOP_SRCDIR    : ${TOP_SRCDIR}")
message("----- PROGRAM_SUFFIX: ${PROGRAM_SUFFIX}")
message("----- LOCALEDIR     : ${LOCALEDIR}")
message("")


# create config.h
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h )


if(MSVC)
 	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)	
 	# disable checked iterators for msvc release builds to get maximum speed
	set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} /D_SECURE_SCL=0")
endif(MSVC)

add_definitions(-DBOOST_USER_CONFIG="<config.h>")
	
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

if(MSVC)
	if(WALL)
		set(DISABLEWALL 0 CACHE TYPE STRING FORCE)
	else(WALL)
		set(DISABLEWALL 1 CACHE TYPE STRING FORCE)
	endif(WALL)
	
	if(NOT DISABLEWALL)
	
		# Use the highest warning level
		set(CMAKE_CXX_WARNING_LEVEL 4 CACHE TYPE STRING FORCE)
		
		if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
			STRING(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		else(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
			SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
		endif(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		
		if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
			STRING(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
		else(CMAKE_C_FLAGS MATCHES "/W[0-4]")
			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
		endif(CMAKE_C_FLAGS MATCHES "/W[0-4]")
		
		# add here warnings which should produce an error /weXXXX
		SET(MSVC_W_ERROR   "/we4101 /we4189")
		# add here warnings which should be disabled /wdXXXX
		SET(MSVC_W_DISABLE "/wd4800 /wd4996 /wd4311 /wd4312 /wd4505")
		
		set(CMAKE_CXX_FLAGS_DEBUG    "${CMAKE_CXX_FLAGS_DEBUG}         /Wp64 ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE}       /Wp64 ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_MINSIZEREL  "${CMAKE_CXX_FLAGS_MINSIZEREL} /Wp64 ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		
	else(NOT DISABLEWALL)	
	
		set(DISABLEWALL 1 CACHE TYPE STRING FORCE)
		set(CMAKE_CXX_WARNING_LEVEL 3 CACHE TYPE STRING FORCE)
		set(WARNING_LEVEL_MESSAGE "(switch to warning level 4 with -DWALL=1)")
	
		# add here warnings which should produce an error /weXXXX
		SET(MSVC_W_ERROR   "/we4101 /we4189")
		# add here warnings which should be disabled /wdXXXX
		SET(MSVC_W_DISABLE "/wd4800 /wd4996")
		
		set(CMAKE_CXX_FLAGS_DEBUG    "${CMAKE_CXX_FLAGS_DEBUG}         ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE}       ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_MINSIZEREL  "${CMAKE_CXX_FLAGS_MINSIZEREL} ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		
	endif(NOT DISABLEWALL)
	
	message("----- Warning level      : ${CMAKE_CXX_WARNING_LEVEL}  ${WARNING_LEVEL_MESSAGE}")
	message("----- Warnings as errors : ${MSVC_W_ERROR}")
	message("----- Warnings disabled  : ${MSVC_W_DISABLE}")
	message("")
	
endif(MSVC)


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
