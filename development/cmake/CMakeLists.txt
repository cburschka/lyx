
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c) 2008-2011 Kornel Benko, <Kornel.Benko@berlin.de>


cmake_minimum_required(VERSION 2.6.4)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 OLD)
	cmake_policy(SET CMP0005 OLD)
endif()

#needed to distinguish fron trunk, so we can install both simultaneously
set(LYX_PROJECT lyx20)
project(${LYX_PROJECT})

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Supress regeneration
set(CMAKE_SUPPRESS_REGENERATION FALSE)

# where to look first for cmake modules,
# before ${CMAKE_ROOT}/Modules is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

if(UNIX)
	FIND_PROGRAM(DPKG_ARCHITECTURE_EXECUTABLE dpkg-architecture)
	if(DPKG_ARCHITECTURE_EXECUTABLE)
		EXECUTE_PROCESS(COMMAND dpkg-architecture -qDEB_HOST_MULTIARCH OUTPUT_VARIABLE ARCH_TRIPLET OUTPUT_STRIP_TRAILING_WHITESPACE)
	endif()
endif()
if(ARCH_TRIPLET)
	set(SYSTEM_LIB_DIRS /usr/lib /usr/lib/${ARCH_TRIPLET} /usr/local/lib)
else()
	set(SYSTEM_LIB_DIRS /usr/lib /usr/local/lib)
endif()

include(LyXMacros)


# Usage LYX_OPTION
# 1. parameter: option name without prefix 'LYX_'
# 2. parameter: description
# 3. parameter: default value, ON or OFF
# 4. parameter: system on which option is used: ALL, GCC, MSVC, ...


LYX_OPTION_INIT()

# Options for all compilers/systems
LYX_OPTION(CPACK "Use the CPack management (Implies LYX_INSTALL option)" OFF ALL)
LYX_OPTION(INSTALL "Build install projects/rules (implies a bunch of other options)" OFF ALL)
LYX_OPTION(NLS "Use nls" OFF ALL)
LYX_OPTION(ASPELL "Require aspell" OFF ALL)
LYX_OPTION(ENCHANT "Require Enchant" OFF ALL)
LYX_OPTION(HUNSPELL "Require Hunspell" OFF ALL)
LYX_OPTION(DEVEL_VERSION "Build developer version" OFF ALL) 
LYX_OPTION(RELEASE "Build release version, build debug when disabled" ON ALL)
LYX_OPTION(PACKAGE_SUFFIX "Use version suffix for packaging" ON ALL)
LYX_OPTION(PCH "Use precompiled headers" OFF ALL)
LYX_OPTION(MERGE_FILES "Merge source files into one compilation unit" OFF ALL)
LYX_OPTION(MERGE_REBUILD "Rebuild generated files from merged files build" OFF ALL)
LYX_OPTION(QUIET "Don't generate verbose makefiles" OFF ALL)
LYX_OPTION(INSTALL_PREFIX "Install path for LyX" OFF ALL)
LYX_OPTION(EXTERNAL_LIBINTL "Use external libintl" ON ALL)

# GCC specific
LYX_OPTION(PROFILE "Build profile version" OFF GCC)
LYX_OPTION(EXTERNAL_BOOST "Use external boost" OFF GCC)
LYX_OPTION(PROGRAM_SUFFIX "Append version suffix to binaries" ON GCC)
LYX_OPTION(DEBUG_GLIBC "Enable libstdc++ debug mode" OFF GCC)
LYX_OPTION(DEBUG_GLIBC_PEDANTIC "Enable libstdc++pedantic debug mode" OFF GCC)
LYX_OPTION(STDLIB_DEBUG "Use debug stdlib" OFF GCC)
LYX_OPTION(CONCEPT_CHECKS "Enable concept-checks" OFF GCC)

# MSVC specific
LYX_OPTION(CONSOLE "Show console on Windows" ON MSVC)
LYX_OPTION(VLD "Use VLD with MSVC" OFF MSVC)
LYX_OPTION(WALL "Enable all warnings" OFF MSVC)
LYX_OPTION(CONFIGURE_CHECKS "Also run configure checks for MSVC" OFF MSVC)
LYX_OPTION(DEPENDENCIES_DOWNLOAD "Download dependencies for MSVC 10" OFF MSVC)

# Check option dependencies
if(LYX_CPACK)
	set(LYX_INSTALL ON)
endif()

if(LYX_INSTALL)
	set(LYX_NLS ON)
	if (WIN32)
		set(LYX_HUNSPELL ON)
	endif()
	set(LYX_CONSOLE OFF)
	set(LYX_PACKAGE_SUFFIX ON)
	set(LYX_RELEASE ON)
	set(LYX_PROFILE OFF)
endif()

if(LYX_MERGE_FILES)
	set(LYX_PCH OFF)
else()
	set(LYX_MERGE_REBUILD OFF)
endif()

if(LYX_DEPENDENCIES_DOWNLOAD)
	message(STATUS)
	set(LYX_DEPENDENCIES_DIR ${CMAKE_BINARY_DIR}/msvc2010-deps)
	message(STATUS "Using downloaded dependencies in ${LYX_DEPENDENCIES_DIR}")
	set(deps_files lyx20-deps-msvc2010.zip)
	set(deps_server http://downloads.sourceforge.net/project/lyx/Win_installers/Dependencies)
	foreach(it ${deps_files})
		set(already_downloaded already_downloaded-NOTFOUND CACHE PATH "downloaded" FORCE)
		find_file(already_downloaded ${it} "${LYX_DEPENDENCIES_DIR}/download")
		if(NOT already_downloaded)
			message(STATUS "Downloading ${it} ...")
			file(DOWNLOAD ${deps_server}/${it} ${LYX_DEPENDENCIES_DIR}/download/${it} SHOW_PROGRESS)
			execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${LYX_DEPENDENCIES_DIR}/download/${it}
		                      WORKING_DIRECTORY ${LYX_DEPENDENCIES_DIR})
		endif()
	endforeach()
	set(GNUWIN32_DIR ${LYX_DEPENDENCIES_DIR}/deps20)
endif()


message(STATUS)

set(EXECUTABLE_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)



if(UNIX OR MINGW)
	execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
	message(STATUS "Using GCC version ${GCC_VERSION}")
	if(NOT GCC_VERSION VERSION_LESS 4.4)
		set(LYX_USE_TR1 1)
		# GCC <= 4.5 does not support regex: there are linker errors
		# http://gcc.gnu.org/onlinedocs/libstdc++/manual/status.html#status.iso.tr1
		set(LYX_USE_TR1_REGEX 0)
	endif()
else()
	if(MSVC10)
		set(LYX_USE_TR1 1)
		#set(LYX_USE_TR1_REGEX 1) #TODO should we use it in ECMAScript mode?
	endif()
endif()


set(VERSION_INFO "CMake Build")

# Try to get some informations from configure.ac

include(LyXPaths)    #called here to define the needed TOP_SRC_DIR-variable
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
	if(_c_l MATCHES "^AC_INIT\\(\([^,]+\),\([^,]+\), *\\[\([^,]+\)\\] *,\(.*\)")
		setstripped(PACKAGE_BASE ${CMAKE_MATCH_1})
		setstripped(PACKAGE_VERSION ${CMAKE_MATCH_2})
		setstripped(PACKAGE_BUGREPORT ${CMAKE_MATCH_3})
		if(PACKAGE_VERSION MATCHES "^\([0-9]+\)\\.\([0-9]+\).*$")
			set(LYX_MAJOR_VERSION ${CMAKE_MATCH_1})
			set(LYX_MINOR_VERSION ${CMAKE_MATCH_2})
			set(LYX_DIR_VER "LYX_DIR_${CMAKE_MATCH_1}${CMAKE_MATCH_2}x")
			set(LYX_USERDIR_VER "LYX_USERDIR_${CMAKE_MATCH_1}${CMAKE_MATCH_2}x")
			set(LYX_INSTALL_SUFFIX "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}")
		endif()
	endif()
	if(_c_l MATCHES "^AC_SUBST\\( *LYX_DATE *, *\\[\\\"(.*)\\\"\\].*")
		set(LYX_DATE "${CMAKE_MATCH_1}")
	endif()
endforeach(_c_l)


if (LYX_INSTALL_PREFIX)
	set(CMAKE_INSTALL_PREFIX ${LYX_INSTALL_PREFIX} CACHE PATH "LyX user's choice install prefix" FORCE)
	set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
endif()
set(LYX_INSTALL_PREFIX ${LYX_INSTALL_PREFIX} CACHE PATH "LyX user's choice install prefix" FORCE)

if(LYX_PACKAGE_SUFFIX)
	set(PACKAGE ${PACKAGE_BASE}${LYX_INSTALL_SUFFIX})
else()  
	set(PACKAGE ${PACKAGE_BASE})
endif()

if(LYX_PROGRAM_SUFFIX)
	set(PROGRAM_SUFFIX "${LYX_INSTALL_SUFFIX}")
else()
	set(PROGRAM_SUFFIX "")
endif()


if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	# see http://www.cmake.org/pipermail/cmake/2006-October/011559.html
	if (UNIX)
		# don't use the default "/usr/local"
		# but respect the user-choice on the command-line
		if(LYX_INSTALL_SUFFIX)
			set(CMAKE_INSTALL_PREFIX "/usr/local/lyx${LYX_INSTALL_SUFFIX}" CACHE PATH "LyX default install prefix" FORCE)
		else()
			set(CMAKE_INSTALL_PREFIX "/usr/local/${LYX_PROJECT}" CACHE PATH "LyX default install prefix" FORCE)
		endif()
	endif()
	if(WIN32)
		set(CMAKE_INSTALL_PREFIX LYX_INSTALLED CACHE PATH "LyX default install prefix" FORCE)
	endif()
endif()

if (WIN32)
	set(LYX_DATA_SUBDIR "Resources/" CACHE STRING "Subdirectory for all lyx-system-data" FORCE)
else()
	set(LYX_DATA_SUBDIR "" CACHE STRING "Subdirectory for all lyx-system-data" FORCE)
endif()

set(LYX_ABS_INSTALLED_DATADIR "${CMAKE_INSTALL_PREFIX}")
set(LYX_LOCALEDIR "locale")
set(LYX_ABS_INSTALLED_LOCALEDIR "${LYX_ABS_INSTALLED_DATADIR}/${LYX_DATA_SUBDIR}${LYX_LOCALEDIR}")
set(LYX_ABS_TOP_SRCDIR "${TOP_SRC_DIR}")


if(NOT GROUP_CODE)
	#set(GROUP_CODE "The Golden Code")
	set(GROUP_CODE flat)
endif()

# lyx's source files
set(LYX_CPP_FILES *.cpp)
set(LYX_HPP_FILES *.h)
set(LYX_MOC_FILES moc_*.cpp)


include(ProjectSourceGroup)


if(LYX_PROFILE)
	set(CMAKE_BUILD_TYPE Profile CACHE TYPE STRING FORCE)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
endif()

if(LYX_RELEASE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build release version" FORCE)
else()
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build debug version" FORCE)
endif()

# When shared libs are supported enable this option
#LYX_OPTION(SHARED_LIBRARIES "Build shared libraries" OFF ALL)
if(LYX_SHARED_LIBRARIES)
	set(library_type SHARED)
else()
 	set(library_type STATIC)
endif()

if(NOT MSVC)
	if(NOT LYX_QUIET)
		set(CMAKE_VERBOSE_MAKEFILE ON)
	endif()
	set(LYX_CXX_FLAGS "-Wall -Wunused-parameter")
	if(LYX_STDLIB_DEBUG)
		set(LYX_CXX_FLAGS "${LYX_CXX_FLAGS} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
	endif()
	if(LYX_CONCEPT_CHECKS)
		set(LYX_CXX_FLAGS "${LYX_CXX_FLAGS} -D_GLIBCPP_CONCEPT_CHECKS")
	endif()
	set(CMAKE_CXX_FLAGS       "${LYX_CXX_FLAGS} -fno-strict-aliasing " CACHE TYPE STRING FORCE)
	set(CMAKE_CXX_FLAGS_DEBUG "${LYX_CXX_FLAGS} -fno-strict-aliasing -O0 -g -D_DEBUG" CACHE TYPE STRING FORCE)
	if(MINGW)
		set(CMAKE_CXX_FLAGS_RELEASE "${LYX_CXX_FLAGS} -O2 -DNDEBUG" CACHE TYPE STRING FORCE)
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "${LYX_CXX_FLAGS} -O3 -DNDEBUG" CACHE TYPE STRING FORCE)
	endif()
	set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_RELEASE} -pg" CACHE TYPE STRING FORCE)
endif()


find_package(Qt4 REQUIRED)
find_package(ICONV REQUIRED)
find_package(ZLIB REQUIRED)

include_directories(${CMAKE_BINARY_DIR} ${TOP_SRC_DIR}/src)
  
if(LYX_ASPELL)
	find_package(ASPELL)
	include_directories(${ASPELL_INCLUDE_DIR})
endif()

if(LYX_ENCHANT)
	find_package(Enchant REQUIRED)
	include_directories(${ENCHANT_INCLUDE_DIR})
endif()

if(LYX_HUNSPELL)
	find_package(Hunspell REQUIRED)
	include_directories(${HUNSPELL_INCLUDE_DIR})
endif()

if(LYX_NLS)
	FIND_PROGRAM(LYX_PYTHON_EXECUTABLE python HINTS ${GNUWIN32_DIR}/python)
	if(${LYX_PYTHON_EXECUTABLE} MATCHES "-NOTFOUND")
		message(FATAL_ERROR "Python 2.6 needed!")
	endif()
	find_package(LyXGettext REQUIRED)
	include_directories(${TOP_SRC_DIR}/po)
	add_subdirectory(po)
endif()

if(LYX_EXTERNAL_LIBINTL)
	find_package(Libintl REQUIRED)
	add_definitions(-DHAVE_GETTEXT) #TODO move to config.h
else()
	add_subdirectory(intl)
endif()

if(LYX_EXTERNAL_BOOST)
	message(STATUS "Searching for boost")
	find_package(Boost COMPONENTS signals regex) #TODO could REQUIRED be used
	if(Boost_FOUND)
		message(STATUS "Boost found")
		message(STATUS "Boost-libs = ${Boost_LIBRARIES}")
		set(Lyx_Boost_Libraries ${Boost_LIBRARIES})
	else()
		message(FATAL_ERROR "Boost not found" ${Boost_ERROR_REASON})
	endif()
else()
	if(LYX_USE_TR1_REGEX)
		set(Lyx_Boost_Libraries boost_signals)
	else()
		set(Lyx_Boost_Libraries boost_signals boost_regex)
	endif()
	add_definitions(-DBOOST_USER_CONFIG="<config.h>")
	include_directories(${TOP_SRC_DIR}/boost)
	add_subdirectory(boost)
endif()




if(WIN32)
	if(NOT LYX_CONSOLE)
		set(WIN32_CONSOLE WIN32)
		set(LYX_QTMAIN_LIBRARY ${QT_QTMAIN_LIBRARY})
	endif()
	if(MSVC)
		add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -DNOMINMAX)
		# disable checked iterators for msvc release builds to get maximum speed
		set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} /D_SECURE_SCL=0")
	else()
		add_definitions(-DWINVER=0x0500)
	endif()
endif()

message(STATUS "")
foreach(_v PACKAGE PACKAGE_VERSION
	PROGRAM_SUFFIX LYX_DATE LYX_DIR_VER LYX_USERDIR_VER
	LYX_ABS_TOP_SRCDIR
	LYX_ABS_INSTALLED_DATADIR LYX_ABS_INSTALLED_LOCALEDIR LYX_INSTALL_SUFFIX)
	if(NOT DEFINED ${_v})
		message(FATAL_ERROR "${_v} not defined")
	endif()
	string(SUBSTRING "${_v}                            " 0 28 _v1)
	message(STATUS "----- ${_v1}: ${${_v}}")
endforeach(_v)
message(STATUS "")


if(LYX_PCH)
	# PCHs not supported by cmake: http://www.cmake.org/Bug/view.php?id=1260
	# Not sure if it works for all non-msvc compilers
	include(PCHSupport_26)
	configure_file(${CMAKE_SOURCE_DIR}/pcheaders.h ${CMAKE_BINARY_DIR}/pcheaders.h)
	configure_file(${CMAKE_SOURCE_DIR}/config.cpp.cmake ${CMAKE_BINARY_DIR}/config_pch.cpp)
	add_definitions(-DLYX_ENABLE_PCH)

	if(MSVC)
		macro(lyx_add_msvc_pch name_)
			set_source_files_properties(${${name_}_sources} PROPERTIES
				COMPILE_FLAGS "/Yuconfig.h /Fp\$(IntDir)/config.pch")
			set_source_files_properties(${CMAKE_BINARY_DIR}/config_pch.cpp PROPERTIES
				COMPILE_FLAGS "/Ycconfig.h /Fp\$(IntDir)/config.pch")
			set(${name_}_sources ${${name_}_sources} ${CMAKE_BINARY_DIR}/config_pch.cpp
				${CMAKE_BINARY_DIR}/pcheaders.h)
			set(CMAKE_CXX_FLAGS_DEBUG	"${CMAKE_CXX_FLAGS_DEBUG} /DLYX_PCH_STL /DLYX_PCH_BOOST /DLYX_PCH_QT4")
			set(CMAKE_CXX_FLAGS_RELEASE	"${CMAKE_CXX_FLAGS_RELEASE} /DLYX_PCH_STL /DLYX_PCH_BOOST /DLYX_PCH_QT4")
			set(CMAKE_CXX_FLAGS_MINSIZEREL	"${CMAKE_CXX_FLAGS_MINSIZEREL} /DLYX_PCH_STL /DLYX_PCH_BOOST")			
		endmacro(lyx_add_msvc_pch)
		macro(lyx_add_gcc_pch name_)
		endmacro()
	else()
		macro(lyx_add_msvc_pch name_)
		endmacro()
		macro(lyx_add_gcc_pch name_)
			add_definitions(-DLYX_PCH_STL -DLYX_PCH_BOOST -DLYX_PCH_QT4)
			ADD_PRECOMPILED_HEADER(${name_} ${CMAKE_BINARY_DIR}/config_pch.cpp ${CMAKE_BINARY_DIR}/config.h.gch)
		endmacro()
	endif()
else()
	macro(lyx_add_msvc_pch)
	endmacro(lyx_add_msvc_pch)
	macro(lyx_add_gcc_pch name_)
	endmacro()
endif()

if(MSVC)
	if(LYX_VLD)
		set(LYX_LEAK_DETECTION 1 CACHE TYPE STRING FORCE)
		set(LIBRARY_OUTPUT_PATH  ${EXECUTABLE_OUTPUT_PATH})
		set(LYX_VLD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../Win32/vld)
		include(${LYX_VLD_PATH}/tools/cmake/vld.cmake)
	endif()

	if(LYX_WALL)
		# Use the highest warning level
		set(CMAKE_CXX_WARNING_LEVEL 4 CACHE TYPE STRING FORCE)

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
		set(MSVC_W_DISABLE "/wd4288 /wd4355 /wd4800 /wd4996 /wd4311 /wd4312 /wd4505 /wd4267 /wd4512 /wd4245 /wd4127 /wd4180 /wd4231")

		set(CMAKE_CXX_FLAGS_DEBUG      "${CMAKE_CXX_FLAGS_DEBUG} /Wp64 ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_RELEASE    "${CMAKE_CXX_FLAGS_RELEASE} /Wp64 ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /Wp64 ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")

	else()
		set(CMAKE_CXX_WARNING_LEVEL 3 CACHE TYPE STRING FORCE)

		# add here warnings which should produce an error /weXXXX
		set(MSVC_W_ERROR   "/we4101 /we4189")
		# add here warnings which should be disabled /wdXXXX
		set(MSVC_W_DISABLE "/wd4288 /wd4355 /wd4800 /wd4996 /wd4267 /wd4180 /wd4231")

		set(CMAKE_CXX_FLAGS_DEBUG      "${CMAKE_CXX_FLAGS_DEBUG} ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_RELEASE    "${CMAKE_CXX_FLAGS_RELEASE} ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")

	endif()
	set(CMAKE_CXX_FLAGS_DEBUG      "${CMAKE_CXX_FLAGS_DEBUG} /Zc:wchar_t-")
	set(CMAKE_CXX_FLAGS_RELEASE    "${CMAKE_CXX_FLAGS_RELEASE} /Zc:wchar_t-")
	set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /Zc:wchar_t-")

	message(STATUS "----- Warning level      : ${CMAKE_CXX_WARNING_LEVEL}  ${WARNING_LEVEL_MESSAGE}")
	message(STATUS "----- Warnings as errors : ${MSVC_W_ERROR}")
	message(STATUS "----- Warnings disabled  : ${MSVC_W_DISABLE}")
	message(STATUS "")
endif()


if(LYX_DEBUG_GLIBC)
	set(_GLIBCXX_DEBUG 1)
endif()

if(LYX_DEBUG_GLIBC_PEDANTIC)
	set(_GLIBCXX_DEBUG_PEDANTIC 1)
endif()

# compiler tests, config.h generation
if(MSVC AND NOT LYX_CONFIGURE_CHECKS)
	configure_file(configCompiler.h.msvc ${CMAKE_BINARY_DIR}/configCompiler.h)
else()
	include(ConfigureChecks.cmake)
	configure_file(configCompiler.h.cmake ${CMAKE_BINARY_DIR}/configCompiler.h)
endif()

configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h)


add_subdirectory(src)
add_subdirectory(lyx2lyx)
add_subdirectory(scripts)


if(LYX_INSTALL)
	if(${LYX_PYTHON_EXECUTABLE} MATCHES "-NOTFOUND")
		message(STATUS "Python required to create doc!")
	else()
		add_subdirectory(man)
		add_subdirectory(doc)
	endif()
	include(../Install)
endif()


message(STATUS)
message(STATUS "Build options, switch LYX_* variables by -DLYX_*=1 or 0:")
message(STATUS)
LYX_OPTION_LIST_ALL()


set(CPACK_PACKAGE_VERSION_MAJOR "${LYX_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${LYX_MINOR_VERSION}")
FILE(STRINGS "${TOP_SRC_DIR}/development/cmake/LyX_summary.txt" CPACK_PACKAGE_DESCRIPTION_SUMMARY)

set(CPACK_SOURCE_GENERATOR TGZ ZIP TBZ2)
set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${TOP_SRC_DIR};/") # http://www.mail-archive.com/cmake@cmake.org/msg33720.html

set(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${LYX_INSTALL_SUFFIX}")

set(CPACK_SOURCE_GENERATOR TGZ ZIP TBZ2)
set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${TOP_SRC_DIR};/") # http://www.mail-archive.com/cmake@cmake.org/msg33720.html
#
# needed by rpm
set(CPACK_SET_DESTDIR "ON")
FILE(READ "${TOP_SRC_DIR}/development/cmake/LyX_license.txt" CPACK_RPM_PACKAGE_LICENSE)
set(CPACK_RPM_PACKAGE_GROUP "Applications/Publishing")
set(CPACK_RPM_PACKAGE_VENDOR "The LyX team")
#
# the next ones are needed by deb
set(CPACK_PACKAGE_CONTACT "${PACKAGE_BUGREPORT}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${LYX_PROJECT}-${LYX_INSTALL_SUFFIX}")

# This is experimental, valid on _my_ system (Kornel)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqtgui4 (>= 4.4.3-0)")
if(ASPELL_FOUND)
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libaspell15 (>= 0.60.6-1),${CPACK_DEBIAN_PACKAGE_DEPENDS}")
endif()
if(ENCHANT_FOUND)
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libenchant1c2a (>= 1.6.0-0),${CPACK_DEBIAN_PACKAGE_DEPENDS}")
endif()
if(LYX_EXTERNAL_BOOST)
	if(Boost_FOUND)
		set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-regex1.40.0 (>= 1.40.0-4),libboost-signals1.40.0 (>= 1.40.0-4),${CPACK_DEBIAN_PACKAGE_DEPENDS}")
	endif()
endif()
#
# for the next ones, cpack insists on data with values in some file
set(CPACK_PACKAGE_DESCRIPTION_FILE "${TOP_SRC_DIR}/development/cmake/LyX_description.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${TOP_SRC_DIR}/development/cmake/LyX_license.txt")

# Find the revision number and use it as the release in rpm-package-build.
# This way we may omit the otherwise needed "--force" parameter when
# installing from that rpm package.
FIND_PROGRAM(LYX_SVNVERSION svnversion)
#message(STATUS "svnversion = ${LYX_SVNVERSION}")
if(NOT ${LYX_SVNVERSION} MATCHES "-NOTFOUND")
	EXECUTE_PROCESS(COMMAND ${LYX_SVNVERSION} WORKING_DIRECTORY "${TOP_SRC_DIR}" OUTPUT_VARIABLE CPACK_RPM_PACKAGE_RELEASE OUTPUT_STRIP_TRAILING_WHITESPACE)
	# We use this value also to set the package-patch-value
	if(CPACK_RPM_PACKAGE_RELEASE MATCHES "^\([0-9]+\)")
		set(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_MATCH_1})
	endif()
endif()

# we do not have a readme or welcome data,
# so we do not provide infofiles for
# CPACK_RESOURCE_FILE_README and CPACK_RESOURCE_FILE_WELCOME

if(LYX_CPACK)
	include(CPack)
endif()

#Now it is possible to create some packages
# cd <BuildDir>
# make package
#############################################################################################
# So, e.g. for variables
#    CMAKE_PROJECT_NAME                     : lyx
#    CPACK_PACKAGE_VERSION_MAJOR            : 2
#    CPACK_PACKAGE_VERSION_MINOR            : 0
#    CPACK_PACKAGE_VERSION_PATCH            : 1
#    CMAKE_SYSTEM_NAME                      : Linux
#    CPACK_BINARY_DEB:BOOL                  : ON
#
# the package name builds as "lyx-2.0.1-Linux.deb"
#
############################## rpm ################################### deb ##################
# create    # cpack -G RPM --config CPackConfig.cmake   # cpack -G DEB --config CPackConfig.cmake
# creates =># lyx-2.0.1-Linux.rpm                       # lyx-2.0.1-Linux.deb
# list      # rpm -qlp lyx-2.0.1-Linux.rpm              # dpkg-deb -c lyx-2.0.1-Linux.deb
# install   # rpm -U lyx-2.0.1-Linux.rpm                # dpkg -i lyx-2.0.1-Linux.deb
#
message(STATUS)
