# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c) 2008-2011 Kornel Benko, <Kornel.Benko@berlin.de>

cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_CXX_STANDARD 20)
set(GNUWIN32_DIR D:/LyX/lyx-unstable/lyx-windows-deps-msvc2017)
set(LYX_USE_QT "QT5")
set(LYX_REQUIRE_SPELLCHECK true)

set(LYX_PROJECT LyX)

# Instruct cmake to not use gnu extensions,
# this prevents the mix of '-std=c++*' and '-std=gnu++*' flags
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

get_filename_component(lyx_dir_readme ${CMAKE_SOURCE_DIR}/README REALPATH) # Resolve symlinks
get_filename_component(TOP_SRC_DIR ${lyx_dir_readme} PATH)
message(STATUS "TOP_SRC_DIR = ${TOP_SRC_DIR}")

set(LYX_CMAKE_DIR "development/cmake")
set(TOP_CMAKE_PATH "${TOP_SRC_DIR}/${LYX_CMAKE_DIR}")
set(TOP_MODULE_PATH "${TOP_CMAKE_PATH}/modules")
set(TOP_SCRIPT_PATH "${TOP_CMAKE_PATH}/scripts")

set(CMAKE_MODULE_PATH "${TOP_MODULE_PATH}")

set(CMAKE_PROJECT_NAME ${LYX_PROJECT})

message(STATUS)
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(STATUS "Building in-source")
	set(TOP_BINARY_DIR "${CMAKE_BINARY_DIR}/build")
else()
	message(STATUS "Building out-of-source")
	set(TOP_BINARY_DIR "${CMAKE_BINARY_DIR}")
endif()

# This directory should be used if some test uses lyx-executable
set(LYX_TESTS_USERDIR "${TOP_BINARY_DIR}/Testing/.lyx")
file(MAKE_DIRECTORY "${LYX_TESTS_USERDIR}")

if(COMMAND cmake_policy)
	# Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION
	cmake_policy(SET CMP0006 NEW)
	if(POLICY CMP0043)
		# COMPILE_DEFINITIONS are not used yet. Enable new behavior.
		cmake_policy(SET CMP0043 NEW)
	endif()
	cmake_policy(SET CMP0020 NEW)
	if(POLICY CMP0075)
		cmake_policy(SET CMP0075 NEW)
	endif()
endif()

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Supress regeneration
set(CMAKE_SUPPRESS_REGENERATION FALSE)

if(LYX_XMINGW)
    set(CMAKE_SYSTEM_NAME Windows)
    set(TOOLNAME ${LYX_XMINGW})
    set(TOOLCHAIN "${TOOLNAME}-")
    set(CMAKE_C_COMPILER   "${TOOLCHAIN}gcc"     CACHE PATH "Mingw C compiler" FORCE)
    set(CMAKE_CXX_COMPILER "${TOOLCHAIN}g++"     CACHE PATH "Mingw C++ compiler" FORCE)
    set(CMAKE_RC_COMPILER  "${TOOLCHAIN}windres" CACHE PATH "Mingw rc compiler" FORCE)

    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()

if(NOT help AND NOT HELP)
  # 'project' triggers the searching for a compiler
  project(${LYX_PROJECT})
  if (CMAKE_COMPILER_IS_GNUCXX)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.0")
      message(STATUS "Gnu CXX compiler version = ${CMAKE_CXX_COMPILER_VERSION}")
      message(STATUS "is too old, should be >= 4.0")
      message(FATAL_ERROR "Exiting")
    endif()
  endif()
endif()

if(UNIX)
	set(ARCH_TRIPLET )
	FIND_PROGRAM(DPKG_ARCHITECTURE_EXECUTABLE dpkg-architecture)
	if(DPKG_ARCHITECTURE_EXECUTABLE)
		EXECUTE_PROCESS(COMMAND ${DPKG_ARCHITECTURE_EXECUTABLE} -qDEB_HOST_MULTIARCH
				OUTPUT_VARIABLE ARCH_TRIPLET
				ERROR_VARIABLE ERROR_ARCH_TRIPLET
				OUTPUT_STRIP_TRAILING_WHITESPACE)
	endif()
endif()
if(ARCH_TRIPLET)
	set(SYSTEM_LIB_DIRS /usr/lib /usr/lib/${ARCH_TRIPLET} /usr/local/lib)
else()
	set(SYSTEM_LIB_DIRS /usr/lib /usr/local/lib)
endif()

include(LyXMacros)
include(LyXDestinations)

# Try to get some informations from configure.ac
include(LyXPaths)

# Value of USE_POSIX_PACKAGING is needed in determineversionandbuildtype()
if(WIN32)
	set(CMAKE_PREFIX_PATH Specify-path-to-Qt CACHE PATH "Used Qt version")
	if(MSVC)
		set(LYX_3RDPARTY_BUILD ON CACHE BOOL "Build 3rdparty libraries" FORCE)
	endif()
	set(USE_WINDOWS_PACKAGING ON)
elseif(APPLE)
	set(USE_MACOSX_PACKAGING ON)
else()
	set(USE_POSIX_PACKAGING ON)
endif()

determineversionandbuildtype("${TOP_SRC_DIR}/configure.ac" _package_list _version_list _envlist LYX_DATE LYX_BUILD_TYPE)
list(GET _package_list 0 PACKAGE_BASE)
list(GET _package_list 1 PACKAGE_VERSION)
list(GET _package_list 2 PACKAGE_BUGREPORT)
list(GET _envlist 0 LYX_DIR_VER)
list(GET _envlist 1 LYX_USERDIR_VER)
list(GET _version_list 0 LYX_VERSION)
list(GET _version_list 1 LYX_MAJOR_VERSION)
list(GET _version_list 2 LYX_MINOR_VERSION)
list(GET _version_list 3 LYX_RELEASE_LEVEL)
list(GET _version_list 4 LYX_RELEASE_PATCH)

# Usage LYX_OPTION
# 1. parameter: option name without prefix 'LYX_'
# 2. parameter: description
# 3. parameter: default value, ON or OFF
# 4. parameter: system on which option is used: ALL, GCC, MSVC, ...

# Usage LYX_COMBO
# 1. parameter: name without prefix 'LYX_'
# 2. parameter: description
# 3. parameter: default value
# 4-n parameter: possible other string values

LYX_OPTION_INIT()
LYX_COMBO(ENABLE_BUILD_TYPE  "Allows to tweak the compiled code" AUTO release prerelease development gprof)

if(LYX_ENABLE_BUILD_TYPE MATCHES "AUTO")
  message(STATUS "Selecting build type defaults from configure.ac")
else()
  set(LYX_BUILD_TYPE "${LYX_ENABLE_BUILD_TYPE}")
  message(STATUS "Selecting build type defaults from LYX_ENABLE_BUILD_TYPE")
endif()

# Select some defaults depending on LYX_BUILD_TYPE
# they can always be overwritten by the respective command line settings
# These settings are only effective on fresh(==empty) CMakeCache.txt
if(LYX_BUILD_TYPE STREQUAL "development")
  set(DefaultLyxDebug ON)
  set(DefaultLyxRelease OFF)
  set(DefaultLyxStdlibDebug ON)
  set(DefaultLyxEnableAssertions ON)
  set(DefaultLyxProfile OFF)
  set(DefaultExternalLibs OFF)
elseif(LYX_BUILD_TYPE STREQUAL "prerelease")
  set(DefaultLyxDebug OFF)
  set(DefaultLyxRelease OFF)
  set(DefaultLyxStdlibDebug OFF)
  set(DefaultLyxEnableAssertions OFF)
  set(DefaultLyxProfile OFF)
  set(DefaultExternalLibs ON)
elseif(LYX_BUILD_TYPE STREQUAL "release")
  set(DefaultLyxDebug OFF)
  set(DefaultLyxRelease ON)
  set(DefaultLyxStdlibDebug OFF)
  set(DefaultLyxEnableAssertions OFF)
  set(DefaultLyxProfile OFF)
  set(DefaultExternalLibs ON)
elseif(LYX_BUILD_TYPE STREQUAL "gprof")
  set(DefaultLyxDebug ON)
  set(DefaultLyxRelease OFF)
  set(DefaultLyxStdlibDebug OFF)
  set(DefaultLyxEnableAssertions OFF)
  set(DefaultLyxProfile ON)
  set(DefaultExternalLibs OFF)
else()
  message(FATAL_ERROR "Invalid build type (${LYX_BUILD_TYPE}) encountered")
endif()

# Options for all compilers/systems
LYX_OPTION(CPACK            "Use the CPack management (Implies LYX_INSTALL option)" OFF ALL)
LYX_OPTION(LOCALVERSIONING  "Add version info to created package name (only used if LYX_CPACK option set)" OFF ALL)
LYX_OPTION(INSTALL          "Build install projects/rules (implies a bunch of other options)" OFF ALL)
LYX_OPTION(NLS              "Enable Native Language Support (NLS)" ON ALL)
LYX_OPTION(REQUIRE_SPELLCHECK "Abort if no spellchecker available" OFF ALL)
LYX_OPTION(ASPELL           "Require aspell" OFF ALL)
LYX_OPTION(ENCHANT          "Require Enchant" OFF ALL)
LYX_OPTION(HUNSPELL         "Require Hunspell" OFF ALL)
LYX_OPTION(RELEASE          "Build release version, build debug when disabled" ${DefaultLyxRelease} ALL)
LYX_OPTION(DEBUG            "Enforce debug build"  ${DefaultLyxDebug} ALL)
LYX_OPTION(NO_OPTIMIZE      "Don't use any optimization/debug flags"  OFF ALL)
LYX_OPTION(ENABLE_ASSERTIONS "Run sanity checks in the program"  ${DefaultLyxEnableAssertions} ALL)
LYX_OPTION(PACKAGE_SUFFIX   "Use version suffix for packaging" ON ALL)
LYX_STRING(SUFFIX_VALUE     "Use this string as suffix" "")
LYX_OPTION(PCH              "Use precompiled headers" OFF ALL)
LYX_OPTION(MERGE_FILES      "Merge source files into one compilation unit" OFF ALL)
LYX_OPTION(MERGE_REBUILD    "Rebuild generated files from merged files build" OFF ALL)
LYX_OPTION(QUIET            "Don't generate verbose makefiles" OFF ALL)
LYX_OPTION(INSTALL_PREFIX   "Install path for LyX" OFF ALL)
LYX_OPTION(BUNDLE           "Build bundle  (experimental) " OFF ALL)
LYX_OPTION(ENABLE_URLTESTS  "Enable for URL tests" OFF ALL)
LYX_OPTION(ENABLE_EXPORT_TESTS "Enable for export tests" OFF ALL)
LYX_OPTION(ENABLE_KEYTESTS  "Enable for keytests" OFF ALL)
if (NOT CMAKE_VERSION VERSION_LESS "3.17")
  LYX_OPTION(ENABLE_VALGRIND_TESTS  "Enable for tests involving valgrind" OFF ALL)
else()
  if (LYX_ENABLE_VALGRIND_TESTS)
    unset(LYX_ENABLE_VALGRIND_TESTS CACHE)
  endif()
endif()
LYX_OPTION(ASAN             "Use address sanitizer" OFF ALL)
#LYX_COMBO(USE_FILEDIALOG    "Use native or QT file dialog" QT NATIVE)
LYX_COMBO(USE_QT            "Use Qt version as frontend" AUTO QT4 QT5)
LYX_COMBO(USE_IPO           "Interprocedural optimization" OFF AUTO ON)
#LYX_OPTION(3RDPARTY_BUILD   "Build 3rdparty libs" OFF ALL)
LYX_OPTION(DISABLE_CALLSTACK_PRINTING "do not print a callstack when crashing" OFF ALL)
LYX_OPTION(EXTERNAL_Z       "OFF := Build 3rdparty lib zlib" ${DefaultExternalLibs} ALL)
LYX_OPTION(EXTERNAL_DTL     "OFF := Build 3rdparty commands dt2dv and dv2dt" ${DefaultExternalLibs} ALL)
LYX_OPTION(EXTERNAL_ICONV   "OFF := Build 3rdparty lib iconvlib" ${DefaultExternalLibs} ALL)
LYX_OPTION(EXTERNAL_HUNSPELL "OFF := Build 3rdparty lib hunspelllib" ${DefaultExternalLibs} ALL)
LYX_COMBO(EXTERNAL_MYTHES   "OFF := Build 3rdparty lib mytheslib" AUTO OFF ON)

# GCC specific
LYX_OPTION(PROFILE              "Build with options for gprof" ${DefaultLyxProfile} GCC)
LYX_OPTION(EXTERNAL_BOOST       "Use external boost" ${DefaultExternalLibs} GCC)
LYX_OPTION(PROGRAM_SUFFIX       "Append version suffix to binaries" ON GCC)
LYX_OPTION(DEBUG_GLIBC          "Enable libstdc++ debug mode" OFF GCC)
LYX_OPTION(DEBUG_GLIBC_PEDANTIC "Enable libstdc++ pedantic debug mode" OFF GCC)
LYX_OPTION(STDLIB_DEBUG         "Use debug stdlib" ${DefaultLyxStdlibDebug} GCC)

# MSVC specific
LYX_OPTION(CONSOLE               "Show console on Windows" ON MSVC)
LYX_OPTION(VLD                   "Use VLD with MSVC" OFF MSVC)
LYX_OPTION(WALL                  "Enable all warnings" OFF MSVC)
LYX_OPTION(DEPENDENCIES_DOWNLOAD "Download dependencies for MSVC 10" OFF MSVC)

# APPLE specific
LYX_OPTION(DMG                   "Build as Mac bundle, needed for .dmg  (experimental) " OFF MAC)
LYX_OPTION(COCOA                 "Use Cocoa on Mac" OFF MAC)


if(help OR HELP)
	message(STATUS)
	message(STATUS "Available options: (dis/enable with -DLYX_*=OFF/ON)")
	message(STATUS)
	LYX_OPTION_LIST_ALL(help)
	message(STATUS)
	RETURN()
endif()

if ("${LYX_SUFFIX_VALUE}" MATCHES "")
  set(LYX_INSTALL_SUFFIX "${LYX_MAJOR_VERSION}.${LYX_MINOR_VERSION}")
else()
  set(LYX_INSTALL_SUFFIX "${LYX_SUFFIX_VALUE}")
endif()

# Check option dependencies
if (LYX_ENABLE_VALGRIND_TESTS)
	find_program(VALGRIND_EXECUTABLE "valgrind")
	if (NOT VALGRIND_EXECUTABLE)
		message(FATAL_ERROR "Cannot use valgrind tests, executable valgrind is missing. Please disable LYX_ENABLE_VALGRIND_TESTS")
	endif()
endif()
if (LYX_DEBUG_GLIBC OR LYX_DEBUG_GLIBC_PEDANTIC OR LYX_STDLIB_DEBUG)
  if (LYX_EXTERNAL_BOOST)
    message(FATAL_ERROR "Using external boost not compatible with debug mode for stdlib")
  endif()
endif()

if(LYX_DMG)
	set(LYX_BUNDLE ON)
	set(LYX_CPACK ON)
endif()

if(LYX_CPACK)
	set(LYX_INSTALL ON)
endif()

if(LYX_INSTALL)
	set(LYX_NLS ON)
        if(WIN32 AND NOT MINGW)
		set(LYX_HUNSPELL ON)
	endif()
	set(LYX_PACKAGE_SUFFIX ON)
	if(NOT LYX_DEBUG)
		set(LYX_RELEASE ON)
	endif()
	set(LYX_PROFILE OFF)
endif()


if(LYX_MERGE_FILES)
	set(LYX_PCH OFF)
else()
	set(LYX_MERGE_REBUILD OFF)
endif()

if(LYX_DEPENDENCIES_DOWNLOAD)
	message(STATUS)
	if(MSVC14)
		set(LYX_DEPENDENCIES_DIR ${TOP_BINARY_DIR}/msvc2015-deps)
		set(deps_files lyx-windows-deps-msvc2015.zip)
		set(deps_server http://ftp.lyx.de/LyX-Windows-Deps)
		set(GNUWIN32_DIR ${LYX_DEPENDENCIES_DIR}/lyx-windows-deps-msvc2015)
	elseif(MSVC10)
		set(LYX_DEPENDENCIES_DIR ${TOP_BINARY_DIR}/msvc2010-deps)
		set(deps_files lyx-windows-deps-msvc2010.zip)
		set(deps_server http://ftp.lyx.de/LyX-Windows-Deps)
		set(GNUWIN32_DIR ${LYX_DEPENDENCIES_DIR}/lyx-windows-deps-msvc2010)
	else()
		message(FATAL_ERROR "error: no dependency package known for the selected MSVC version.")
	endif()
    message(STATUS "Using downloaded dependencies in ${LYX_DEPENDENCIES_DIR}")
	foreach(it ${deps_files})
		set(already_downloaded already_downloaded-NOTFOUND CACHE PATH "downloaded" FORCE)
		find_file(already_downloaded ${it} "${LYX_DEPENDENCIES_DIR}/download")
		if(NOT already_downloaded)
			message(STATUS "Downloading ${it} ...")
			file(DOWNLOAD ${deps_server}/${it} ${LYX_DEPENDENCIES_DIR}/download/${it} SHOW_PROGRESS STATUS status LOG log)
			list(GET status 0 status_code)
			list(GET status 1 status_string)
			if(NOT status_code EQUAL 0)
				file(REMOVE ${LYX_DEPENDENCIES_DIR}/${it})
				message(FATAL_ERROR "error: downloading '${it}' failed. status_code: ${status_code}, status_string: ${status_string}. \nLog: ${log} ")
			endif()
			execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${LYX_DEPENDENCIES_DIR}/download/${it}
		                      WORKING_DIRECTORY ${LYX_DEPENDENCIES_DIR})
		endif()
	endforeach()
endif()

message(STATUS)

set(EXECUTABLE_OUTPUT_PATH  ${TOP_BINARY_DIR}/bin)
if(WIN32)
	set(LIBRARY_OUTPUT_PATH  ${EXECUTABLE_OUTPUT_PATH})
else()
	set(LIBRARY_OUTPUT_PATH  ${TOP_BINARY_DIR}/lib)
endif()

set(LYX_IPO_SUPPORTED OFF)
if (POLICY CMP0069)
  cmake_policy(SET CMP0069 NEW)
  if(LYX_USE_IPO MATCHES "AUTO")
    # Enable LTO if supported and not debugging
    if (NOT LYX_DEBUG)
      include(CheckIPOSupported)
      check_ipo_supported(RESULT LYX_IPO_SUPPORTED)
    endif()
  else()
    set(LYX_IPO_SUPPORTED ${LYX_USE_IPO})
  endif()
endif()
if (LYX_IPO_SUPPORTED)
  set(LYX_USE_IPO "ON" CACHE STRING "Use interprocedural optimization" FORCE)
else()
  set(LYX_USE_IPO "OFF" CACHE STRING "Use interprocedural optimization" FORCE)
endif()

# Set to some meaningful default
find_package(CXX11Compiler)
if(NOT CXX11COMPILER_FOUND)
	message(FATAL_ERROR "A C++11 compatible compiler is required.")
endif()
unset(LYX_GCC11_MODE)
if(UNIX OR MINGW)
	if (CMAKE_CXX_COMPILER_ID MATCHES "^([cC]lang|AppleClang)$")
	  # ignore the GCC_VERSION for clang
	  # and use the resulting CXX11_STD_REGEX found in FindCXX11Compiler.cmake
	  message(STATUS "Using clang")
	  if(CXX11_STD_REGEX)
	    set(LYX_USE_STD_REGEX 1)
	  else()
	    set(LYX_USE_STD_REGEX 0)
	  endif()
	else()
	  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpfullversion OUTPUT_VARIABLE GCC_VERSION ERROR_VARIABLE _error RESULT_VARIABLE _err OUTPUT_STRIP_TRAILING_WHITESPACE)
	  #message(STATUS "dumpfullversion: error = ${_error}, result = ${_err}")
	  if (_err)
	    # previous check failed, try again with _old_ parameter
	    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION ERROR_VARIABLE _error RESULT_VARIABLE _err OUTPUT_STRIP_TRAILING_WHITESPACE)
	    #message(STATUS "dumpversion: error = ${_error}, result = ${_err}")
	  endif()
	  message(STATUS "Using GCC version ${GCC_VERSION}")
	  if(GCC_VERSION VERSION_LESS 4.6)
		message(FATAL_ERROR "gcc >= 4.6 is required.")
	  elseif(GCC_VERSION VERSION_LESS 4.9)
		# <regex> in gcc is unusable in versions less than 4.9.0
		# see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631
		set(LYX_USE_STD_REGEX 0)
	  else()
		set(LYX_USE_STD_REGEX 1)
	  endif()
	endif()
	set(LYX_GCC11_MODE "${CXX11_FLAG}")
else()
	if(MSVC_VERSION LESS 1900)
		# Drop support for msvc versions prior to 1900 (Visual Studio 2015)
		message(FATAL_ERROR "Visual Studio >= 2015 is required.")
		set(LYX_USE_STD_REGEX 0)
	else()
		set(LYX_USE_STD_REGEX 1)
	endif()
endif()

if(LYX_3RDPARTY_BUILD)
  # LYX_3RDPARTY_BUILD is not cached anymore, but for compatibility reasons
  # this enables the build of all 3rd_party libs
  set(LYX_EXTERNAL_Z        OFF CACHE BOOL "Build 3rdparty lib zlib"    FORCE)
  set(LYX_EXTERNAL_ICONV    OFF CACHE BOOL "Build 3rdparty iconvlib"    FORCE)
  set(LYX_EXTERNAL_HUNSPELL OFF CACHE BOOL "Build 3rdparty hunspelllib" FORCE)
  set(LYX_EXTERNAL_MYTHES   OFF CACHE STRING "Build 3rdparty mytheslib" FORCE)
endif()

FIND_PROGRAM(LYX_GITVERSION git)
#message(STATUS "gitversion = ${LYX_GITVERSION}")
set(LYX_REVISION_VERSION ${LYX_RELEASE_LEVEL})
set(LYX_PACKAGE_RELEASE "UNDEFINED")
if(LYX_GITVERSION)
  if (LYX_LOCALVERSIONING)
    # Find the revision number for later use
    EXECUTE_PROCESS(COMMAND ${LYX_GITVERSION} describe --match 2.0.0 HEAD
      WORKING_DIRECTORY "${TOP_SRC_DIR}"
      RESULT_VARIABLE tmp_GIT_RESULT
      OUTPUT_VARIABLE tmp_LYX_PACKAGE_RELEASE
      ERROR_VARIABLE tmp_GIT_ERROR
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (tmp_GIT_RESULT EQUAL 0)
      if (tmp_LYX_PACKAGE_RELEASE MATCHES "^2\\.0\\.0\\-\([0-9]+\)\\-\(.*\)$")
	set(LYX_PACKAGE_RELEASE ${CMAKE_MATCH_2})
	# We will add offset of 40000 to get appropriate value to
	# previous svn.
	# We use this value than also to set the package-patch-value
	MATH(EXPR tmp_REVISION_VERSION "(${CMAKE_MATCH_1}+40000)")
	set(LYX_REVISION_VERSION "${LYX_RELEASE_LEVEL}-${tmp_REVISION_VERSION}git")
	set(ENABLE_DIST ON)
      endif()
    endif()
  endif()
endif()
if (LYX_PACKAGE_RELEASE MATCHES "UNDEFINED")
  set(LYX_PACKAGE_RELEASE  "1")
  if (LYX_RELEASE_PATCH GREATER 0)
    set(LYX_REVISION_VERSION "${LYX_RELEASE_LEVEL}.${LYX_RELEASE_PATCH}")
  endif()
endif()

# Set the programs (lyx, tex2lyx, etc.) suffix
# When building an OS X bundle, we will append
# the suffix only to the bundle, not to the programs
set(PROGRAM_SUFFIX "")
if(LYX_PROGRAM_SUFFIX AND NOT (APPLE AND LYX_BUNDLE))
	set(PROGRAM_SUFFIX "${LYX_INSTALL_SUFFIX}")
endif()
set(_lyx "${PACKAGE_BASE}${PROGRAM_SUFFIX}")
set(_tex2lyx tex2lyx${PROGRAM_SUFFIX})
set(_convert lyxconvert${PROGRAM_SUFFIX})

add_custom_target(lyx_version ALL
  COMMAND ${CMAKE_COMMAND} -DTOP_SRC_DIR=${TOP_SRC_DIR} -DTOP_CMAKE_PATH=${TOP_CMAKE_PATH} -DTOP_BINARY_DIR=${TOP_BINARY_DIR} -DLYX_DATE=${LYX_DATE} -P ${TOP_SCRIPT_PATH}/LyXGetVersion.cmake
  )
set_target_properties(lyx_version PROPERTIES FOLDER "applications")

# Default paths for installed utilities (tex2lyx, lyxclient, etc.)
set(LYX_UTILITIES_INSTALL_PATH bin)

if(LYX_BUNDLE)
	set(LYX_CPACK ON)

	message(STATUS)
	message(STATUS "Bundle creation is enabled (experimental):")
	message(STATUS "    make")
	message(STATUS "    make install/strip")
	message(STATUS "    make package")
	if(APPLE)
		set(MACOSX_BUNDLE TRUE)
		# This sets the bundle + executable names
		set(_lyx "${PACKAGE_BASE}${LYX_INSTALL_SUFFIX}")
		# This sets the Info.plist executable name
		set(osx_bundle_program_name ${_lyx})
		# Bundle name
		set(LYX_BUNDLE_NAME ${_lyx})

		# This will contain the list of files that need to be included
		# in the bundle and their location within the bundle
		set(OSX_BUNDLE_FILES "${TOP_SRC_DIR}/development/MacOSX/LyX.sdef;${TOP_SRC_DIR}/development/MacOSX/LyXapp.icns;${CMAKE_BINARY_DIR}/lyxrc.dist")
		set(OSX_BUNDLE_DIRS "Resources;Resources;Resources")

		# Process Info.plist and lyxrc.dist
		# See development/cmake/Install.cmake
		configure_file("${CMAKE_CURRENT_SOURCE_DIR}/development/MacOSX/Info.plist.in" "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")

		set(LYX_BUILD_BUNDLE MACOSX_BUNDLE)
		set(LYX_DATA_SUBDIR ${LYX_BUNDLE_NAME}.app/Contents/Resources/ CACHE STRING "Bundle Contents dir" FORCE)

		# Variables used by CPack
		set(CPACK_BUNDLE_NAME ${LYX_BUNDLE_NAME})
		set(CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
		set(MACOSX_BUNDLE_STARTUP_COMMAND ${LYX_BUNDLE_NAME}.app)

		# Set the install paths
		set(LYX_UTILITIES_INSTALL_PATH ${LYX_BUNDLE_NAME}.app/Contents/MacOS)
		if(NOT LYX_INSTALL_PREFIX)
			set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/LyX CACHE PATH "Mac bundle dir" FORCE)
			set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		endif()
		# Change the owner so that the install can work
		install(CODE "set(BU_CHMOD_BUNDLE_ITEMS 1)" COMPONENT Runtime)
	elseif(UNIX)
		message(STATUS "To embed Qt in this bundle don't build with your system Qt:")
		message(STATUS "  - fix PATH so a other qmake is found by cmake")
		message(STATUS "  - fix LD_LIBRARY_PATH so lyx doesn't use system's Qt")
	elseif(WIN32)
		message(STATUS "A zipped-only release could be created by building the 'PACKAGE' project")
	endif()
endif()

if(LYX_INSTALL_PREFIX)
	set(CMAKE_INSTALL_PREFIX ${LYX_INSTALL_PREFIX} CACHE PATH "LyX user's choice install prefix" FORCE)
	set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
endif()
set(LYX_INSTALL_PREFIX ${LYX_INSTALL_PREFIX} CACHE PATH "LyX user's choice install prefix" FORCE)

if(UNIX)
  set(SYSTEM_DATADIR "${CMAKE_INSTALL_PREFIX}/share")
elseif(CMAKE_INSTALL_PREFIX MATCHES "/lyx${LYX_INSTALL_SUFFIX}$")
  string(REGEX REPLACE "/lyx${LYX_INSTALL_SUFFIX}$" "/share" SYSTEM_DATADIR ${CMAKE_INSTALL_PREFIX})
else()
  set(SYSTEM_DATADIR "${CMAKE_INSTALL_PREFIX}")
endif()

if(NOT CMAKE_COMPILER_IS_GNUCXX)
  # Not a GCC compiler, programs do not have a suffix
  set(suffixing ${LYX_PACKAGE_SUFFIX})
elseif(WIN32 AND MINGW)
  # We want to use a suffix for the package in this case,
  # even if not for the program
  set(suffixing ${LYX_PACKAGE_SUFFIX})
else()
  message(STATUS "CMAKE_COMPILER_IS_GNUCXX = ${CMAKE_COMPILER_IS_GNUCXX}")
  set(suffixing ${LYX_PROGRAM_SUFFIX})
endif()

# The define PACKAGE below allows lyx-executable to find its default configuration files
# see routines
#    Package::messages_file()
#    get_default_user_support_dir()
#    relative_system_support_dir()
# in src/support/Package.cpp
if(suffixing)
	set(PACKAGE ${PACKAGE_BASE}${LYX_INSTALL_SUFFIX})
else()
	set(PACKAGE ${PACKAGE_BASE})
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	# see http://www.cmake.org/pipermail/cmake/2006-October/011559.html
	if (UNIX)
		# use the default "/usr/local"
		# but respect the user-choice on the command-line
		set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "LyX default install prefix" FORCE)
	endif()
endif()
if(WIN32)
	set(CMAKE_INSTALL_PREFIX LYX_INSTALLED CACHE PATH "LyX default install prefix" FORCE)
endif()
if(IS_ABSOLUTE)
	set(CMAKE_INSTALL_PREFIX_ABSOLUTE ${CMAKE_INSTALL_PREFIX})
else()
	set(CMAKE_INSTALL_PREFIX_ABSOLUTE ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_PREFIX})
endif()



if(NOT LYX_DATA_SUBDIR)
	if (WIN32)
		set(LYX_DATA_SUBDIR "Resources/" CACHE STRING "Subdirectory for all lyx-system-data" FORCE)
	else()
		set(LYX_DATA_SUBDIR "share/${_lyx}/" CACHE STRING "Subdirectory for all lyx-system-data" FORCE)
	endif()
endif()

set(LYX_ABS_INSTALLED_DATADIR "${CMAKE_INSTALL_PREFIX}/${LYX_DATA_SUBDIR}")
get_locale_destination(LYX_LOCALEDIR)
set(LYX_ABS_INSTALLED_LOCALEDIR "${CMAKE_INSTALL_PREFIX}/${LYX_LOCALEDIR}")
set(LYX_ABS_TOP_SRCDIR "${TOP_SRC_DIR}")

if(LYX_BUNDLE AND APPLE)
	set(LYX_MAN_DIR_tmp "${LYX_DATA_SUBDIR}")
else()
	if(WIN32)
		set(LYX_MAN_DIR_tmp "${CMAKE_BINARY_DIR}/usr/local/man/")
	elseif(UNIX)
		set(LYX_MAN_DIR_tmp "${CMAKE_INSTALL_PREFIX}/share/man/")
	else()
		set(LYX_MAN_DIR_tmp "${CMAKE_INSTALL_PREFIX}/man/")
	endif()
endif()
if (NOT LYX_MAN_DIR_tmp EQUAL "${LYX_MAN_DIR}")
	unset(LYX_MAN_DIR CACHE)
	set(LYX_MAN_DIR "${LYX_MAN_DIR_tmp}" CACHE STRING "Install location for man pages.")
endif()
unset(LYX_MAN_DIR_tmp)
mark_as_advanced(LYX_MAN_DIR)

# The Win installer cannot be built by CMake because one needs to install plugins for NSIS
# see the Readme.txt of the installer
#if(LYX_INSTALL AND WIN32)
#	message(STATUS "Configuring NSIS files")
#	configure_file(development/Win32/packaging/installer/lyx.nsi.cmake ${CMAKE_BINARY_DIR}/installer/lyx.nsi @ONLY)
#	configure_file(development/Win32/packaging/installer/settings.nsh.cmake ${CMAKE_BINARY_DIR}/installer/settings-cmake.nsh @ONLY)
#	configure_file(development/Win32/packaging/installer/include/declarations.nsh.cmake ${CMAKE_BINARY_DIR}/installer/declarations-cmake.nsh @ONLY)
#	message(STATUS "NSIS files are created in ${CMAKE_BINARY_DIR}/installer")
#endif()

if(NOT GROUP_CODE)
	#set(GROUP_CODE "The Golden Code")
	set(GROUP_CODE flat)
endif()

# lyx's source files
set(LYX_CPP_FILES [a-zA-Z]*.cpp)
set(LYX_HPP_FILES [a-zA-Z]*.h)
set(LYX_MOC_FILES moc_*.cpp)

include(ProjectSourceGroup)


if(LYX_PROFILE AND NOT MSVC)
	set(CMAKE_BUILD_TYPE Profile CACHE STRING "Profile build type" FORCE)
	set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_RELEASE} -pg")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS}  -pg")
else()
	set(LYX_PROFILE OFF)
endif()


if(LYX_NO_OPTIMIZE)
	set(CMAKE_BUILD_TYPE None)
	set(LYX_DEBUG OFF)
	set(LYX_RELEASE OFF)
	set(LYX_PROFILE OFF)
elseif(LYX_RELEASE)
	set(CMAKE_BUILD_TYPE Release)
	set(LYX_DEBUG OFF)
	set(LYX_PROFILE OFF)
elseif(LYX_PROFILE)
	set(CMAKE_BUILD_TYPE Profile)
	set(LYX_DEBUG OFF)
else()
	set(CMAKE_BUILD_TYPE Debug)
	set(LYX_DEBUG ON)
endif()


# When shared libs are supported enable this option
#LYX_OPTION(SHARED_LIBRARIES "Build shared libraries" OFF ALL)
if(LYX_SHARED_LIBRARIES)
	set(library_type SHARED)
else()
	set(library_type STATIC)
endif()

if(LYX_ENABLE_ASSERTIONS)
  set(LYX_CXX_FLAGS " -DENABLE_ASSERTIONS=1")
else()
  set(LYX_CXX_FLAGS "")
endif()

if(MSVC)
  if (CXX11_FLAG MATCHES "\\+\\+([0-9]+)")
    set(CMAKE_CXX_STANDARD ${CMAKE_MATCH_1})
    message(STATUS "CMAKE_CXX_STANDARD set to ${CMAKE_CXX_STANDARD}")
  endif()
  set(CMAKE_CXX_FLAGS       "${CMAKE_CXX_FLAGS}${LYX_CXX_FLAGS}")
else()
	if(NOT LYX_QUIET)
		set(CMAKE_VERBOSE_MAKEFILE ON)
	endif()
	if(LYX_GCC11_MODE MATCHES "\\+\\+([0-9][0-9])")
	  # Thanks to Brad King <brad.king@kitware.com>
	  # for the pointer to https://cmake.org/cmake/help/v3.6/variable/CMAKE_CXX_STANDARD.html
	  # This allows us to use QT5.7 with recent g++ (version >= 4.9) compilers
	  # and still use our own c++ extension tests
	  set(CMAKE_CXX_STANDARD ${CMAKE_MATCH_1})
	  message(STATUS "CMAKE_CXX_STANDARD set to ${CMAKE_CXX_STANDARD}")
	  set(LYX_CXX_FLAGS "-Wall -Wunused-parameter ${LYX_CXX_FLAGS}")
	else()
	  set(LYX_CXX_FLAGS "-Wall -Wunused-parameter ${LYX_GCC11_MODE}${LYX_CXX_FLAGS}")
	endif()
	if(LYX_STDLIB_DEBUG)
	  set(LYX_CXX_FLAGS "${LYX_CXX_FLAGS} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
        else()
          if(LYX_DEBUG_GLIBC)
            set(LYX_CXX_FLAGS "${LYX_CXX_FLAGS} -D_GLIBCXX_DEBUG")
          endif()
          if(LYX_DEBUG_GLIBC_PEDANTIC)
            set(LYX_CXX_FLAGS "${LYX_CXX_FLAGS} -D_GLIBCXX_DEBUG_PEDANTIC")
          endif()
	endif()
	set(CMAKE_CXX_FLAGS       "${LYX_CXX_FLAGS} -fno-strict-aliasing " CACHE STRING "Set CXX flags" FORCE)
	set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -D_DEBUG" CACHE STRING "Set debug flags" FORCE)
	if(MINGW)
		set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG" CACHE STRING "Set release flags for Mingw" FORCE)
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "Set release flags" FORCE)
	endif()
endif()

if(LYX_ASAN)
    set(CMAKE_CXX_FLAGS "-fsanitize=address -fno-omit-frame-pointer -g ${CMAKE_CXX_FLAGS}")
    message(STATUS)
    message(STATUS "Address sanitizer enabled. Usage:")
    message(STATUS "    wget https://llvm.org/svn/llvm-project/compiler-rt/trunk/lib/asan/scripts/asan_symbolize.py")
    message(STATUS "    chmod  +x ./asan_symbolize.py")
    message(STATUS "    ./bin/${_lyx} 2>&1 | ./asan_symbolize.py  | c++filt ")
    message(STATUS)
endif()

set(LYX_CXX_FLAGS_EXTRA "" CACHE STRING "Desired semicolon separated list of extra cxx compile flags, like '-Werror'")
mark_as_advanced(LYX_CXX_FLAGS_EXTRA)
if(LYX_CXX_FLAGS_EXTRA)
	foreach(_flag ${LYX_CXX_FLAGS_EXTRA})
		add_definitions(${_flag})
	endforeach()
endif()

if(LYX_XMINGW)
	list(APPEND CMAKE_FIND_ROOT_PATH ${GNUWIN32_DIR})
endif()

set(min_qt5_version "5.6")
if(LYX_USE_QT MATCHES "AUTO")
	# try qt5 first
	find_package(Qt5Core CONFIG QUIET)
	if(Qt5Core_FOUND)
		set(LYX_USE_QT "QT5" CACHE STRING "Valid qt version" FORCE)
		message(STATUS "Qt5Core_VERSION = ${Qt5Core_VERSION}")
		if(Qt5Core_VERSION VERSION_LESS ${min_qt5_version})
			find_package(Qt4 "4.5.0" QUIET)
			if(QT4_FOUND)
				set(LYX_USE_QT "QT4" CACHE STRING "Valid qt version" FORCE)
			endif()
		endif()
	else()
		set(LYX_USE_QT "QT4" CACHE STRING "Valid qt version" FORCE)
	endif()
endif()
if(LYX_USE_QT MATCHES "QT5")
	# set QPA_XCB if QT uses X11
	find_package(Qt5Core CONFIG REQUIRED)
	if (Qt5Core_FOUND)
		find_package(Qt5Widgets CONFIG REQUIRED)
		if(APPLE)
			find_package(Qt5MacExtras CONFIG REQUIRED)
		endif()
		find_package(Qt5X11Extras CONFIG QUIET)
		find_package(Qt5WinExtras CONFIG QUIET)
		set(QTVERSION ${Qt5Core_VERSION})
		if (QTVERSION VERSION_LESS ${min_qt5_version})
			message(STATUS "QTVERSION = \"${QTVERSION}\"")
			message(STATUS "This version is not recommended, try either option -DLYX_USE_QT=QT4 or")
			message(STATUS "install QT-Version >= \"${min_qt5_version}\"")
			# see thread in lyx-devel list
			# From: Jean-Pierre Chrétien <jeanpierre.chretien@free.fr>
			# Date 11.03.2017
			# Subject: cmake compilation error
			#message(FATAL_ERROR "Wrong Qt-Version")
		endif()
		macro (qt_use_modules lyxtarget)
			foreach (_tg ${ARGN})
				find_package(Qt5${_tg} CONFIG REQUIRED)
				target_link_libraries(${lyxtarget} Qt5::${_tg})
			endforeach()
		endmacro()
		macro (qt_add_resources)
			qt5_add_resources(${ARGN})
		endmacro()
		macro (qt_wrap_uifiles)
			qt5_wrap_ui(${ARGN})
		endmacro()
		message(STATUS "Found Qt-Version ${QTVERSION}")
		if(WIN32)
			set(LYX_QTMAIN_LIBRARY ${Qt5Core_QTMAIN_LIBRARIES})
		endif()
	endif()
elseif(LYX_USE_QT MATCHES "QT4")
	if(LYX_XMINGW)
		set(QT_MINGW_DIR ${LYX_QT4} CACHE PATH "Qt for Mingw" FORCE)
		list(APPEND CMAKE_FIND_ROOT_PATH ${QT_MINGW_DIR} ${GNUWIN32_DIR})
	endif()
	find_package(Qt4 "4.5.0" REQUIRED)
	macro (qt_use_modules)
	endmacro()
	macro (qt_add_resources)
		qt4_add_resources(${ARGN})
	endmacro()
	macro (qt_wrap_uifiles)
		qt4_wrap_ui(${ARGN})
	endmacro()
        set(LYX_QTMAIN_LIBRARY ${QT_QTSVG_LIBRARY})
	if(WIN32)
		list(APPEND LYX_QTMAIN_LIBRARY ${QT_QTMAIN_LIBRARIES})
	endif()
else()
  message(FATAL_ERROR "Unhandled value for LYX_USE_QT (${LYX_USE_QT})")
endif()

find_package(Magic)
if(Magic_FOUND)
  set(HAVE_MAGIC_H 1)
  include_directories(${Magic_INCLUDE_DIR})
endif()

include_directories(${TOP_BINARY_DIR} ${TOP_SRC_DIR}/src)

set(Spelling_FOUND OFF)
set(Include_used_spellchecker)   # String will be inserted into config.h

if (LYX_EXTERNAL_MYTHES MATCHES "AUTO")
  # try system library first
  find_package(MYTHESLIB)
  if (MYTHESLIB_FOUND)
    set(LYX_EXTERNAL_MYTHES ON CACHE STRING "OFF:= Build 3rdparty mytheslib" FORCE)
  else()
    set(LYX_EXTERNAL_MYTHES OFF CACHE STRING "OFF:= Build 3rdparty mytheslib" FORCE)
  endif()
endif()
if (LYX_EXTERNAL_MYTHES MATCHES "ON")
  find_package(MYTHESLIB REQUIRED)
else()
  add_subdirectory(3rdparty/mythes)
endif()
set(MYTHES_DIR ${MYTHESLIB_INCLUDE_DIR})

if(NOT LYX_EXTERNAL_HUNSPELL)
    add_subdirectory(3rdparty/hunspell)
    add_definitions(-DHUNSPELL_STATIC)
    set(HUNSPELL_FOUND ON)
    message(STATUS "  * Hunspell:")
    message(STATUS "     - include: ${HUNSPELL_INCLUDE_DIR}")
    message(STATUS "     - library: ${HUNSPELL_LIBRARY}")
endif()

foreach(_spell "ASPELL" "ENCHANT" "HUNSPELL")
  string(TOUPPER ${_spell} _upspell)
  if (NOT ${_upspell}_FOUND)
    find_package(${_spell})
  endif()
  if (${_upspell}_FOUND)
    include_directories(${${_upspell}_INCLUDE_DIR})
    set(Spelling_FOUND ON)
    message(STATUS "Building with USE_${_upspell}")
    set(Include_used_spellchecker "${Include_used_spellchecker}#define USE_${_upspell} 1\n")
  else()
    if(LYX_${_upspell})
      message(FATAL_ERROR "Required ${_spell} devel package not found")
    else()
      message(STATUS "${_upspell} not found, building without ${_spell} support")
    endif()
  endif()
endforeach()

if(GNUWIN32_DIR)
	list(APPEND CMAKE_PROGRAM_PATH "${GNUWIN32_DIR}/Python" )
endif()

# Search for python default version first
unset(PYTHON_EXECUTABLE CACHE)
unset(LYX_PYTHON_EXECUTABLE CACHE)
unset(PYTHON_VERSION_MAJOR)
unset(PYTHON_VERSION_MINOR)
unset(PYTHON_VERSION_STRING)
if (CMAKE_VERSION VERSION_LESS "3.13")
  find_package(PythonInterp 3.5 QUIET)
  if(NOT PYTHONINTERP_FOUND)
    find_package(PythonInterp 2.0 REQUIRED)
    if(NOT PYTHON_VERSION_STRING VERSION_LESS 2.8)
      message(FATAL_ERROR "Python interpreter found, but is not suitable")
    endif()
  endif()
  set(LYX_PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE} CACHE FILEPATH "Python to be used by LyX")
else()
  find_package(Python3 3.5 QUIET)
  if(NOT Python3_Interpreter_FOUND)
    unset(PYTHON_EXECUTABLE CACHE)
    find_package(Python2 2.0 REQUIRED)
    if(NOT Python2_VERSION VERSION_LESS 2.8)
      message(FATAL_ERROR "Python interpreter found, but is not suitable")
    endif()
    set(LYX_PYTHON_EXECUTABLE ${Python2_EXECUTABLE} CACHE FILEPATH "Python to be used by LyX")
  else()
    set(LYX_PYTHON_EXECUTABLE ${Python3_EXECUTABLE} CACHE FILEPATH "Python to be used by LyX")
  endif()
endif()

if(LYX_NLS)
	find_package(LyXGettext)
	if(LYX_PYTHON_EXECUTABLE AND GETTEXT_FOUND)
		add_subdirectory(po "${TOP_BINARY_DIR}/po")
	else()
		# Install only supplied .gmo-files
		file(GLOB _gmofiles RELATIVE "${TOP_SRC_DIR}/po" "${TOP_SRC_DIR}/po/*.gmo")
		message(STATUS "Installing provided .gmo-files only")
		foreach( _gmo ${_gmofiles})
			string(REGEX REPLACE "\\.gmo$" "" _lang ${_gmo})
			install(FILES "${TOP_SRC_DIR}/po/${_gmo}" DESTINATION ${LYX_LOCALEDIR}/${_lang}/LC_MESSAGES RENAME ${PACKAGE}.mo)
		endforeach()
	endif()
endif()

if(LYX_EXTERNAL_ICONV)
  find_package(ICONV REQUIRED)
else()
  add_subdirectory(3rdparty/libiconv)
  set(HAVE_ICONV_CONST 1)
endif()

if(LYX_EXTERNAL_Z)
  find_package(ZLIB REQUIRED)
else()
  add_subdirectory(3rdparty/zlib)
endif()

if(NOT LYX_EXTERNAL_DTL)
  add_subdirectory(3rdparty/dtl)
endif()

if(LYX_USE_STD_REGEX)
  # Set only include path.
  # Use internal boost, which is known to exist
  # we don't need any libraries
  set(Lyx_Boost_Libraries)
  add_definitions(-DBOOST_USER_CONFIG=<config.h>)
  include_directories(${TOP_SRC_DIR}/3rdparty/boost)
else()
  # Using boost-regex
  if(LYX_EXTERNAL_BOOST)
    message(STATUS "Searching for external boost")
    find_package(Boost COMPONENTS regex)
    if(Boost_FOUND)
      message(STATUS "Boost found")
      message(STATUS "Boost-libs = ${Boost_LIBRARIES}")
      set(Lyx_Boost_Libraries ${Boost_LIBRARIES})
      if (LYX_STDLIB_DEBUG OR LYX_DEBUG_GLIBC OR LYX_DEBUG_GLIBC_PEDANTIC)
	# Comment from 	Jean-Marc Lasgouttes:
	# In general, system boost libraries are incompatible with
	# the use of stdlib-debug in libstdc++. See ticket #9736 for
	# details.
	message(FATAL_ERROR "Compiling LyX with stdlib-debug and system boost libraries may lead to crashes. Consider using '-DLYX_STDLIB_DEBUG=OFF -DLYX_DEBUG_GLIBC=OFF -DLYX_DEBUG_GLIBC_PEDANTIC=OFF' or using '-DLYX_EXTERNAL_BOOST=OFF'")
      endif()
    else()
      message(FATAL_ERROR "Boost not found" ${Boost_ERROR_REASON})
    endif()
  else()
    # Using included boost
    set(Lyx_Boost_Libraries boost_regex)
    add_definitions(-DBOOST_USER_CONFIG=<config.h>)
    include_directories(${TOP_SRC_DIR}/3rdparty/boost)
    add_subdirectory(3rdparty/boost "${TOP_BINARY_DIR}/3rdparty/boost")
  endif()
endif()

if(WIN32)
	if(LYX_CONSOLE)
		set(LYX_QTMAIN_LIBRARY)
	else()
		set(WIN32_CONSOLE WIN32)
	endif()
	if(MSVC)
		# -DPSAPI_VERSION=1 is needed to run on vista (bug 10186)
		add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -DNOMINMAX -DPSAPI_VERSION=1)
		# disable checked iterators for msvc release builds to get maximum speed
		set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} /D_SECURE_SCL=0")
	else()
		# -DPSAPI_VERSION=1 is not needed for mingw, since the mingw psapi.h
		# does not use it and always declares the vista compatible API.
		# If this ever changes then -DPSAPI_VERSION might be needed here as well.
		add_definitions(-DWINVER=0x0500)
	endif()
endif()

# Compute qt-version from ${QTVERSION}

message(STATUS "")
foreach(_v PACKAGE PACKAGE_VERSION
	PROGRAM_SUFFIX LYX_VERSION LYX_DIR_VER LYX_USERDIR_VER
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
	configure_file(${TOP_CMAKE_PATH}/pcheaders.h      ${TOP_BINARY_DIR}/pcheaders.h)
	configure_file(${TOP_CMAKE_PATH}/config.cpp.cmake ${TOP_BINARY_DIR}/config_pch.cpp)
	add_definitions(-DLYX_ENABLE_PCH)

	if(MSVC)
		macro(lyx_add_msvc_pch name_)
			set_source_files_properties(${${name_}_sources} PROPERTIES
				COMPILE_FLAGS "/Yuconfig.h /Fp\$(IntDir)/config.pch")
			set_source_files_properties(${TOP_BINARY_DIR}/config_pch.cpp PROPERTIES
				COMPILE_FLAGS "/Ycconfig.h /Fp\$(IntDir)/config.pch")
			set(${name_}_sources ${${name_}_sources} ${TOP_BINARY_DIR}/config_pch.cpp
				${TOP_BINARY_DIR}/pcheaders.h)
			set(CMAKE_CXX_FLAGS_DEBUG	"${CMAKE_CXX_FLAGS_DEBUG} /DLYX_PCH_STL /DLYX_PCH_BOOST /DLYX_PCH_QT")
			set(CMAKE_CXX_FLAGS_RELEASE	"${CMAKE_CXX_FLAGS_RELEASE} /DLYX_PCH_STL /DLYX_PCH_BOOST /DLYX_PCH_QT")
			set(CMAKE_CXX_FLAGS_MINSIZEREL	"${CMAKE_CXX_FLAGS_MINSIZEREL} /DLYX_PCH_STL /DLYX_PCH_BOOST")
		endmacro(lyx_add_msvc_pch)
		macro(lyx_add_gcc_pch name_)
		endmacro()
	else()
		macro(lyx_add_msvc_pch name_)
		endmacro()
		macro(lyx_add_gcc_pch name_)
			add_definitions(-DLYX_PCH_STL -DLYX_PCH_BOOST -DLYX_PCH_QT)
			ADD_PRECOMPILED_HEADER(${name_} ${TOP_BINARY_DIR}/config_pch.cpp ${TOP_BINARY_DIR}/config.h.gch)
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
		set(vld_path ${CMAKE_CURRENT_SOURCE_DIR}/development/Win32/vld)
		add_subdirectory(${vld_path}/cmake)
		include(${vld_path}/cmake/vld.cmake) # forced include
		set(vld_dll vld)
	endif()

	if(LYX_WALL)
		# Use the highest warning level
		set(CMAKE_CXX_WARNING_LEVEL 4 CACHE STRING "Warning level" FORCE)

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

		set(CMAKE_CXX_FLAGS_DEBUG      "${CMAKE_CXX_FLAGS_DEBUG} /W4 ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_RELEASE    "${CMAKE_CXX_FLAGS_RELEASE} /W4 ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /W4 ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")

	else()
		set(CMAKE_CXX_WARNING_LEVEL 3 CACHE STRING "Warning level" FORCE)

		# add here warnings which should produce an error /weXXXX
		set(MSVC_W_ERROR   "/we4101 /we4189")
		# add here warnings which should be disabled /wdXXXX
		set(MSVC_W_DISABLE "/wd4288 /wd4355 /wd4800 /wd4996 /wd4267 /wd4180 /wd4231")

		set(CMAKE_CXX_FLAGS_DEBUG      "${CMAKE_CXX_FLAGS_DEBUG} ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_RELEASE    "${CMAKE_CXX_FLAGS_RELEASE} ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
		set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")

	endif()

	if (NOT Qt5Core_FOUND)
		set(CMAKE_CXX_FLAGS_DEBUG      "${CMAKE_CXX_FLAGS_DEBUG} /Zc:wchar_t-")
		set(CMAKE_CXX_FLAGS_RELEASE    "${CMAKE_CXX_FLAGS_RELEASE} /Zc:wchar_t-")
		set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /Zc:wchar_t-")
	endif()

	message(STATUS "----- Warning level      : ${CMAKE_CXX_WARNING_LEVEL}  ${WARNING_LEVEL_MESSAGE}")
	message(STATUS "----- Warnings as errors : ${MSVC_W_ERROR}")
	message(STATUS "----- Warnings disabled  : ${MSVC_W_DISABLE}")
	message(STATUS "")
endif()


# Xcode compiler
if (CMAKE_GENERATOR MATCHES Xcode)
	if (XCODE_VERSION VERSION_LESS "5.0")
		# Automatically select the Xcode compiler if not set
		if (NOT DEFINED CMAKE_XCODE_ATTRIBUTE_GCC_VERSION)
			# XCode >= 3.1 has gcc 4.2 (up to actual Xcode)
			if (XCODE_VERSION VERSION_GREATER "3.1")
				SET(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvmgcc42")
			ENDIF(XCODE_VERSION VERSION_GREATER "3.1")
		ENDIF(NOT DEFINED CMAKE_XCODE_ATTRIBUTE_GCC_VERSION)
		# Print the selected compiler version
   	MESSAGE(STATUS "---- Using XCode compiler CMAKE_XCODE_ATTRIBUTE_GCC_VERSION=${CMAKE_XCODE_ATTRIBUTE_GCC_VERSION}")
	ENDIF(XCODE_VERSION VERSION_LESS "5.0")
ENDIF(CMAKE_GENERATOR MATCHES Xcode)

if (WIN32 AND Qt5Core_FOUND)
	set(Z_PREFIX 1)
endif()

# compiler tests, config.h generation LYX_CONFIGURE_CHECKS
include(${LYX_CMAKE_DIR}/ConfigureChecks.cmake)
configure_file(${LYX_CMAKE_DIR}/configCompiler.h.cmake ${TOP_BINARY_DIR}/configCompiler.h)

unset(opts)
foreach(_option ${LYX_OPTIONS})
  if(${_option}_show_message)
    string(SUBSTRING "${_option}                            " 0 31 _var)
    get_property(_prop CACHE ${_option} PROPERTY STRINGS)
    list(FIND LYX_OPTION_STRINGS ${_option} _index)
    if (${_index} GREATER -1)
      set(_isset ${${_option}})
    elseif(${_option})
      set(_isset ON)
    else()
      set(_isset OFF)
    endif()
    string(SUBSTRING "${_isset}           " 0 10 _val)
    set(opts "${opts} ${_option}(${${_option}})")
  endif()
endforeach()
set(tmp_vi "CMake Build")
list(APPEND tmp_vi "  Host name:               ${CMAKE_HOST_SYSTEM_NAME}")
list(APPEND tmp_vi "  Host processor:          ${CMAKE_HOST_SYSTEM_PROCESSOR}")
list(APPEND tmp_vi "  Build flags:             ${opts}")
list(APPEND tmp_vi "  C++ Compiler:            ${CMAKE_CXX_COMPILER_ID} (${CMAKE_CXX_COMPILER_VERSION})")
list(APPEND tmp_vi "  C++ Compiler flags:      ${CMAKE_CXX_FLAGS}")
if (LYX_DEBUG)
list(APPEND tmp_vi "  C++ Compiler user flags: ${CMAKE_CXX_FLAGS_DEBUG}")
else()
list(APPEND tmp_vi "  C++ Compiler user flags: ${CMAKE_CXX_FLAGS_RELEASE}")
endif()
list(APPEND tmp_vi "  Qt version:              ${QTVERSION}")
list(APPEND tmp_vi "  LyX binary dir:          ${CMAKE_INSTALL_PREFIX}/${LYX_UTILITIES_INSTALL_PATH}")
get_data_destination(dest_data)
list(APPEND tmp_vi "  LyX files dir:           ${CMAKE_INSTALL_PREFIX}/${dest_data}")
string(REPLACE ";" "\\n" VERSION_INFO "${tmp_vi}")

configure_file(${LYX_CMAKE_DIR}/config.h.cmake ${TOP_BINARY_DIR}/config.h)
unset(opts)
unset(tmp_vi)

if(QTVERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+).*")
	MATH(EXPR QT4_VERSION "(${CMAKE_MATCH_1}<<16)|(${CMAKE_MATCH_2}<<8)|${CMAKE_MATCH_3}")
endif()

if (NOT HAVE_REGEX)
	set(LYX_USE_STD_REGEX 0)
endif()


set (cmd ${CMAKE_CTEST_COMMAND})
if (MSVC)
	set (cmd ${cmd} -C ${CMAKE_CFG_INTDIR})
else ()
	set (cmd ${cmd} -C ${CMAKE_BUILD_TYPE})
endif ()
add_custom_target (lyx_run_tests COMMAND ${cmd})
set_target_properties(lyx_run_tests PROPERTIES FOLDER "tests")

if(LYX_INSTALL)
	if(${LYX_PYTHON_EXECUTABLE} MATCHES "-NOTFOUND")
		message(STATUS "Python required to create doc!")
	else()
		if(UNIX)
			add_subdirectory(${LYX_CMAKE_DIR}/man "${TOP_BINARY_DIR}/man")
		endif()
		add_subdirectory(${LYX_CMAKE_DIR}/doc "${TOP_BINARY_DIR}/doc")
	endif()
endif()
include(../Install)

# By running the URL tests first, time can be saved when running the ctests in
# parallel since the URL tests take a long time to run (e.g. 10 minutes).
if(LYX_ENABLE_URLTESTS)
	add_subdirectory(development/checkurls "${TOP_BINARY_DIR}/checkurls")
endif()
add_subdirectory(src "${TOP_BINARY_DIR}/src")
add_subdirectory(lib/lyx2lyx "${TOP_BINARY_DIR}/lyx2lyx")
add_subdirectory(lib/scripts "${TOP_BINARY_DIR}/scripts")
add_subdirectory(lib/examples "${TOP_BINARY_DIR}/lib/examples")
add_subdirectory(sourcedoc "${TOP_BINARY_DIR}/sourcedoc")
add_subdirectory(development/autotests "${TOP_BINARY_DIR}/autotests")
add_subdirectory(development/batchtests "${TOP_BINARY_DIR}/batchtests")

message(STATUS)
message(STATUS "Build params, switch LYX_* options by -DLYX_*=ON or OFF, LYX_* combos by -DLYX_*=value:")
message(STATUS)

LYX_OPTION_LIST_ALL(used)


if(LYX_NLS)
	message(STATUS)
	message(STATUS "Building Native Language Support (LYX_NLS is set), used libraries:")
	message(STATUS)
	message(STATUS "  * iconv")
	message(STATUS "     - include: ${ICONV_INCLUDE_DIR}")
	message(STATUS "     - library: ${ICONV_LIBRARY}")
	message(STATUS "  * zlib")
	message(STATUS "     - include: ${ZLIB_INCLUDE_DIR}")
	message(STATUS "     - library: ${ZLIB_LIBRARY}")
	message(STATUS)
endif()

# Apple has builtin native spell checker,
# no need to warn for missing Aspell, Hunspell or Enchant
if(NOT Spelling_FOUND AND NOT APPLE)
  if(LYX_REQUIRE_SPELLCHECK)
    set(_mode "FATAL_ERROR")
  else()
    set(_mode "STATUS")
  endif()
  message(${_mode} "No spellcheck libraries found. LyX will be unable use spellchecking")
endif()

include("${TOP_CMAKE_PATH}/LyxPackaging.cmake")

if(ENABLE_DIST)
  # Use git to create the source package
  # git archive --format=tgz --output=/home/kornel/x.tar.gz master
  if(UNIX)
    set(DIST_FORMAT tgz)
  else()
    set(DIST_FORMAT zip)
  endif()
  set(DIST_NAME ${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${CMAKE_SYSTEM_NAME}.${DIST_FORMAT})

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${DIST_NAME}
    COMMAND  ${LYX_GITVERSION} archive --format=${DIST_FORMAT} --output=${CMAKE_CURRENT_BINARY_DIR}/${DIST_NAME} master
    WORKING_DIRECTORY ${TOP_SRC_DIR})
  add_custom_target(git-archive DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DIST_NAME})
endif()

# Last directory to be included so that the fixup code (OS X)
# can be run last
add_subdirectory(development/cmake/post_install)

message(STATUS)
