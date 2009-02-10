# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 OLD)
	cmake_policy(SET CMP0005 OLD)
endif(COMMAND cmake_policy)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

project(lyx)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/
# is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")
set(EXECUTABLE_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/bin)

# Supress regeneration
set(CMAKE_SUPPRESS_REGENERATION TRUE)

set(VERSION_INFO "CMake Build")

# Try to get some informations from configure.ac

include(LyXPaths)		#called here to define the needed TOP_SRC_DIR-variable
file(STRINGS "${TOP_SRC_DIR}/configure.ac" _config_lines)

if(WIN32)
  set(USE_WINDOWS_PACKAGING 1)
elseif(APPLE)
  set(USE_MACOSX_PACKAGING 1)
else()
  set(USE_POSIX_PACKAGING 1)
endif()

macro(setstripped _varname _value)
  string(STRIP ${_value} _v)
  if(USE_POSIX_PACKAGING)
    string(TOLOWER ${_v} ${_varname})
  else()
    set(${_varname} ${_v})
  endif()
endmacro(setstripped)

foreach(_c_l ${_config_lines} )
  if(_c_l MATCHES "^AC_INIT\\(\([^,]+\),\([^,]+\),\([^,]+\)\(.*\)")
    setstripped(PACKAGE_BASE ${CMAKE_MATCH_1})
    setstripped(PACKAGE_VERSION ${CMAKE_MATCH_2})
    if(PACKAGE_VERSION MATCHES "^\([0-9]+\)\\.\([0-9]+\).*$")
      set(LYX_DIR_VER "LYX_DIR_${CMAKE_MATCH_1}${CMAKE_MATCH_2}x")
      set(LYX_USERDIR_VER "LYX_USERDIR_${CMAKE_MATCH_1}${CMAKE_MATCH_2}x")
      set(LYX_INSTALL_SUFFIX "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}")
    endif()
  endif()
  if(_c_l MATCHES "^AC_SUBST\\( *LYX_DATE *, *\\[\\\"(.*)\\\"\\].*")
    set(LYX_DATE "${CMAKE_MATCH_1}")
  endif()
endforeach(_c_l)

if(UseVersionSuffix)
  message("-- Using versioned PACKAGE. Disable with -DUseVersionSuffix=0")
  set(PACKAGE ${PACKAGE_BASE}${LYX_INSTALL_SUFFIX})
  set(PROGRAM_SUFFIX "${LYX_INSTALL_SUFFIX}")
else()
  message("-- PACKAGE not versioned, to enable use -DUseVersionSuffix=1")
  set(PACKAGE ${PACKAGE_BASE})
  set(PROGRAM_SUFFIX "")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  # see http://www.cmake.org/pipermail/cmake/2006-October/011559.html
  if (UNIX)
    # don't use the default "/usr/local"
    # but respect the user-choice on the command-line
    SET(CMAKE_INSTALL_PREFIX
      "/usr/local/share/lyx${LYX_INSTALL_SUFFIX}" CACHE PATH "LyX install prefix" FORCE)
  endif()
  if(WIN32)
    SET(CMAKE_INSTALL_PREFIX
      ${CMAKE_INSTALL_PREFIX}/${LYX_INSTALL_SUFFIX} CACHE PATH "LyX install prefix" FORCE)
  endif()
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(LYX_ABS_INSTALLED_DATADIR "${CMAKE_INSTALL_PREFIX}")
set(LYX_LOCALEDIR "locale")
set(LYX_ABS_INSTALLED_LOCALEDIR "${LYX_ABS_INSTALLED_DATADIR}/${LYX_LOCALEDIR}")
set(LYX_ABS_TOP_SRCDIR "${TOP_SRC_DIR}")


if(NOT GROUP_CODE)
	#set(GROUP_CODE "The Golden Code")
	set(GROUP_CODE flat)
endif()

# lyx's source files
set(LYX_CPP_FILES *.cpp)
set(LYX_HPP_FILES *.h)

include(LyXMacros)
include(ProjectSourceGroup)

if(merge OR merge_rebuild)
	set(merge 1 CACHE TYPE STRING FORCE)
	set(MERGE_FILES 1 CACHE TYPE STRING FORCE)
	message(STATUS "")
	message(STATUS "All *.cpp files of a project are merged into two files, disable with -Dmerge=0")
	message(STATUS "")
else()
	set(merge 0 CACHE TYPE STRING FORCE)
	set(MERGE_FILES 0 CACHE TYPE STRING FORCE)
	message(STATUS "Enable merging files with -Dmerge=1")
endif()


set(CMAKE_BUILD_TYPE Release)

if(profile)
	set(CMAKE_BUILD_TYPE Profile CACHE TYPE STRING FORCE)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg"
	    CACHE TYPE STRING FORCE)
	set(profile)
endif()

if(release)
	set(CMAKE_BUILD_TYPE Release CACHE TYPE STRING FORCE)
	set(release TRUE CACHE TYPE STRING FORCE)
	set(release)
endif()

if(debug)
	set(CMAKE_BUILD_TYPE Debug CACHE TYPE STRING FORCE)
	set(debug TRUE CACHE TYPE STRING FORCE)
	set(debug)
endif()

if(shared)
	set(library_type SHARED)
 	message(STATUS "building shared libraries")
else()
 	set(library_type STATIC)
endif()

if(NOT MSVC)
	if(NOT quiet)
		set(CMAKE_VERBOSE_MAKEFILE ON CACHE TYPE STRING FORCE)
		message(STATUS "verbose Makefile, disable with -Dquiet=1")
		message(STATUS "")
	else()
		set(CMAKE_VERBOSE_MAKEFILE OFF CACHE TYPE STRING FORCE)
	endif()
	set(LYX_CXX_FLAGS -Wall)
	if(stdlib-debug)
		set(LYX_CXX_FLAGS "${LYX_CXX_FLAGS} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
	endif()
	if(concept-checks)
		set(LYX_CXX_FLAGS "${LYX_CXX_FLAGS} -D_GLIBCPP_CONCEPT_CHECKS")
	endif()
	set(CMAKE_CXX_FLAGS          "${LYX_CXX_FLAGS}" CACHE TYPE STRING FORCE)
	set(CMAKE_CXX_FLAGS_DEBUG    "${LYX_CXX_FLAGS} -O -g -D_DEBUG" CACHE TYPE STRING FORCE)
	if(MINGW)
		set(CMAKE_CXX_FLAGS_RELEASE  "${LYX_CXX_FLAGS} -O2 -DNDEBUG" CACHE TYPE STRING FORCE)
	else()
		set(CMAKE_CXX_FLAGS_RELEASE  "${LYX_CXX_FLAGS} -O3 -DNDEBUG" CACHE TYPE STRING FORCE)
	endif()
	set(CMAKE_CXX_FLAGS_PROFILE  "${CMAKE_CXX_FLAGS_RELEASE} -pg" CACHE TYPE STRING FORCE)
endif()


find_package(Qt4 REQUIRED)
add_definitions(-DQT_NO_STL -DQT_NO_KEYWORDS)

find_package(ZLIB REQUIRED)

if(all OR aspell)
	set(aspell TRUE CACHE TYPE STRING)
	find_package(ASPELL REQUIRED)
else()
	find_package(ASPELL)
endif()
set(aspell)

set(use_external_libintl TRUE)

if(use_external_libintl)
	find_package(Libintl REQUIRED)
	add_definitions(-DHAVE_GETTEXT)
endif()

message("")
if(nls OR all)
	set(nls TRUE CACHE TYPE STRING)
	add_definitions(-DENABLE_NLS=1)
	message("----- Building with ENABLE_NLS")
else()	
	message("----- No nls, to enable use -Dnls=1")
endif()
set(nls)
if(ASPELL_FOUND)
	add_definitions(-DUSE_ASPELL=1)
	message("----- Building with USE_ASPELL")
else()	
	message("----- No aspell, to get more information use -Daspell=1")
endif()
set(all)

if(WIN32)
	if(noconsole)
		set(noconsole TRUE CACHE TYPE STRING)
		set(WIN32_CONSOLE WIN32)
		set(LYX_QTMAIN_LIBRARY ${QT_QTMAIN_LIBRARY})
		message("----- Console disabled")
	else()
		message("----- Console enabled, disable it with -Dnoconsole=1")
	endif()
	if(MSVC)
		add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -DNOMINMAX)	
		# disable checked iterators for msvc release builds to get maximum speed
		set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} /D_SECURE_SCL=0")
	else()
		add_definitions(-DWINVER=0x0500)
	endif()
endif()

message("")
foreach(_v PACKAGE PACKAGE_VERSION
    PROGRAM_SUFFIX LYX_DATE LYX_DIR_VER LYX_USERDIR_VER
    LYX_ABS_TOP_SRCDIR
    LYX_ABS_INSTALLED_DATADIR LYX_ABS_INSTALLED_LOCALEDIR LYX_INSTALL_SUFFIX)
  if(NOT DEFINED ${_v})
    message(FATAL_ERROR "${_v} not defined")
  endif()
  string(SUBSTRING "${_v}                            " 0 28 _v1)
  message("----- ${_v1}: ${${_v}}")
endforeach(_v)
message("")


add_definitions(-DBOOST_USER_CONFIG="<config.h>")
	
if(MSVC AND NOT disable-pch AND NOT MERGE_FILES)
	message("-----")
	message("----- using precompiled headers, disable with -Ddisable-pch=1")
	message("-----")
	configure_file(${CMAKE_SOURCE_DIR}/pcheaders.h
		${CMAKE_BINARY_DIR}/pcheaders.h)
	configure_file(${CMAKE_SOURCE_DIR}/config.cpp.cmake
		${CMAKE_BINARY_DIR}/config_pch.cpp)
	macro(lyx_add_msvc_pch name_)
		set_source_files_properties(${${name_}_sources} PROPERTIES
			COMPILE_FLAGS "/Yuconfig.h /Fp\$(IntDir)/config.pch")
		set_source_files_properties(${CMAKE_BINARY_DIR}/config_pch.cpp PROPERTIES
			COMPILE_FLAGS "/Ycconfig.h /Fp\$(IntDir)/config.pch")
		set(${name_}_sources ${${name_}_sources} ${CMAKE_BINARY_DIR}/config_pch.cpp
			${CMAKE_BINARY_DIR}/pcheaders.h)
		add_definitions(/DLYX_ENABLE_PCH)
	endmacro(lyx_add_msvc_pch)
	set(CMAKE_CXX_FLAGS_DEBUG
		"${CMAKE_CXX_FLAGS_DEBUG} /DLYX_PCH_STL /DLYX_PCH_BOOST /DLYX_PCH_QT4")
	set(CMAKE_CXX_FLAGS_RELEASE
		"${CMAKE_CXX_FLAGS_RELEASE} /DLYX_PCH_STL /DLYX_PCH_BOOST /DLYX_PCH_QT4")
	set(CMAKE_CXX_FLAGS_MINSIZEREL
		"${CMAKE_CXX_FLAGS_MINSIZEREL} /DLYX_PCH_STL /DLYX_PCH_BOOST")
else()
	if(MSVC)
		message("-----")
		message("----- precompiled headers disabled, enable with -Ddisable-pch=0")
		message("-----")
	endif()
	set(disable-pch TRUE CACHE TYPE STRING)
	macro(lyx_add_msvc_pch)
	endmacro(lyx_add_msvc_pch)
endif()

if(MSVC)	
       if(vld)
               set(vld 1 CACHE TYPE STRING FORCE)
               set(LYX_LEAK_DETECTION 1 CACHE TYPE STRING FORCE)
               message(STATUS "")
               message(STATUS "Leak detection enabled, disable with -Dvld=0")
               message(STATUS "")
               set(LIBRARY_OUTPUT_PATH  ${EXECUTABLE_OUTPUT_PATH})
               set(vld_path ${CMAKE_CURRENT_SOURCE_DIR}/../Win32/vld)
               include(${vld_path}/tools/cmake/vld.cmake)
       else()
               set(vld 0 CACHE TYPE STRING FORCE)
               set(LYX_LEAK_DETECTION 0 CACHE TYPE STRING FORCE)
               message(STATUS "")
               message(STATUS "Enable leak detection with -Dvld=1")
               message(STATUS "")
       endif()

	if(WALL)
		set(WALL 1 CACHE TYPE STRING FORCE)
		
		# Use the highest warning level
		set(CMAKE_CXX_WARNING_LEVEL 4 CACHE TYPE STRING FORCE)
		set(WARNING_LEVEL_MESSAGE "(switch to warning level 3 with -DWALL=0)")
		
		if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
			string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
		endif()
		
		if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
			string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
		else()
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
		endif()
		
		# add here warnings which should produce an error /weXXXX
		set(MSVC_W_ERROR   "/we4101 /we4189")
		# add here warnings which should be disabled /wdXXXX
		set(MSVC_W_DISABLE "/wd4288 /wd4355 /wd4800 /wd4996 /wd4311 /wd4312 /wd4505 /wd4267 /wd4512 /wd4245 /wd4127 /wd4180")
		
		set(CMAKE_CXX_FLAGS_DEBUG
			"${CMAKE_CXX_FLAGS_DEBUG} /Wp64 ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_RELEASEs
			"${CMAKE_CXX_FLAGS_RELEASE} /Wp64 ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_MINSIZEREL
			"${CMAKE_CXX_FLAGS_MINSIZEREL} /Wp64 ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		
	else()	
		set(WALL 0 CACHE TYPE STRING FORCE)
		
		set(CMAKE_CXX_WARNING_LEVEL 3 CACHE TYPE STRING FORCE)
		set(WARNING_LEVEL_MESSAGE "(switch to warning level 4 with -DWALL=1)")
	
		# add here warnings which should produce an error /weXXXX
		set(MSVC_W_ERROR   "/we4101 /we4189")
		# add here warnings which should be disabled /wdXXXX
		set(MSVC_W_DISABLE "/wd4288 /wd4355 /wd4800 /wd4996 /wd4267 /wd4180")
		
		set(CMAKE_CXX_FLAGS_DEBUG
			"${CMAKE_CXX_FLAGS_DEBUG} ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_RELEASE
			"${CMAKE_CXX_FLAGS_RELEASE} ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_MINSIZEREL
			"${CMAKE_CXX_FLAGS_MINSIZEREL} ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		
	endif()
	
	message("----- Warning level      : ${CMAKE_CXX_WARNING_LEVEL}  ${WARNING_LEVEL_MESSAGE}")
	message("----- Warnings as errors : ${MSVC_W_ERROR}")
	message("----- Warnings disabled  : ${MSVC_W_DISABLE}")
	message("")
	
endif()


if(nls OR all)
  find_package(LyXGettext REQUIRED)
  include_directories(${TOP_SRC_DIR}/po)
  add_subdirectory(po)
endif()

# compiler tests, config.h generation
if(MSVC AND NOT CONFIGURECHECKS)
	configure_file(configCompiler.h.msvc ${CMAKE_BINARY_DIR}/configCompiler.h)
else()
	include(ConfigureChecks.cmake)
	configure_file(configCompiler.h.cmake ${CMAKE_BINARY_DIR}/configCompiler.h)
endif()
configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h)


find_package(ICONV REQUIRED)
add_definitions(-DHAVE_ICONV=1)

include_directories(
	${CMAKE_BINARY_DIR}
	${TOP_SRC_DIR}/src
	${TOP_SRC_DIR}/boost)

add_subdirectory(boost)
if(NOT use_external_libintl)
	add_subdirectory(intl)
endif()
add_subdirectory(src)

add_subdirectory(man)
add_subdirectory(doc)
add_subdirectory(lyx2lyx)
add_subdirectory(scripts)

include(../Install)

