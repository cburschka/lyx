# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c) 2008-2011 Kornel Benko, <Kornel.Benko@berlin.de>

cmake_minimum_required(VERSION 2.6.4)

set(LYX_PROJECT LyX)

enable_testing()

get_filename_component(lyx_dir_readme ${CMAKE_SOURCE_DIR}/README ABSOLUTE)
get_filename_component(TOP_SRC_DIR ${lyx_dir_readme} PATH)

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
	cmake_policy(SET CMP0003 OLD)
	cmake_policy(SET CMP0005 OLD)
	# Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION
	cmake_policy(SET CMP0006 NEW)
	if(POLICY CMP0028)
		cmake_policy(SET CMP0028 OLD)
	endif()
	if(POLICY CMP0043)
		# COMPILE_DEFINITIONS are not used yet. Enable new behavior.
		cmake_policy(SET CMP0043 NEW)
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

# Options for all compilers/systems
LYX_OPTION(CPACK            "Use the CPack management (Implies LYX_INSTALL option)" OFF ALL)
LYX_OPTION(LOCALVERSIONING  "Add version info to created package name (only used if LYX_CPACK option set)" OFF ALL)
LYX_OPTION(INSTALL          "Build install projects/rules (implies a bunch of other options)" OFF ALL)
LYX_OPTION(NLS              "Enable Native Language Support (NLS)" ON ALL)
LYX_OPTION(REQUIRE_SPELLCHECK "Abort if no spellchecker available" OFF ALL)
LYX_OPTION(ASPELL           "Require aspell" OFF ALL)
LYX_OPTION(ENCHANT          "Require Enchant" OFF ALL)
LYX_OPTION(HUNSPELL         "Require Hunspell" OFF ALL)
LYX_OPTION(DEVEL_VERSION    "Build developer version" OFF ALL) 
LYX_OPTION(RELEASE          "Build release version, build debug when disabled" OFF ALL)
LYX_OPTION(DEBUG            "Enforce debug build"  OFF ALL)
LYX_OPTION(NO_OPTIMIZE      "Don't use any optimization/debug flags"  OFF ALL)
LYX_OPTION(PACKAGE_SUFFIX   "Use version suffix for packaging" ON ALL)
LYX_OPTION(PCH              "Use precompiled headers" OFF ALL)
LYX_OPTION(MERGE_FILES      "Merge source files into one compilation unit" OFF ALL)
LYX_OPTION(MERGE_REBUILD    "Rebuild generated files from merged files build" OFF ALL)
LYX_OPTION(QUIET            "Don't generate verbose makefiles" OFF ALL)
LYX_OPTION(INSTALL_PREFIX   "Install path for LyX" OFF ALL)
LYX_OPTION(BUNDLE           "Build bundle  (experimental) " OFF ALL)
LYX_OPTION(ENABLE_URLTESTS  "Enable for URL tests" OFF ALL)
LYX_OPTION(ENABLE_EXPORT_TESTS "Enable for export tests" OFF ALL)
LYX_OPTION(ASAN             "Use address sanitizer" OFF ALL)
LYX_COMBO(USE_QT            "Use Qt version as frontend" QT4 QT5)

# GCC specific
LYX_OPTION(PROFILE              "Build profile version" OFF GCC)
LYX_OPTION(EXTERNAL_BOOST       "Use external boost" OFF GCC)
LYX_OPTION(PROGRAM_SUFFIX       "Append version suffix to binaries" ON GCC)
LYX_OPTION(DEBUG_GLIBC          "Enable libstdc++ debug mode" OFF GCC)
LYX_OPTION(DEBUG_GLIBC_PEDANTIC "Enable libstdc++ pedantic debug mode" OFF GCC)
LYX_OPTION(STDLIB_DEBUG         "Use debug stdlib" OFF GCC)
LYX_OPTION(PROFILE              "Build with options for gprof" OFF GCC)
LYX_OPTION(ENABLE_CXX11         "Build with options for c++11-mode" OFF GCC)

# MSVC specific
LYX_OPTION(CONSOLE               "Show console on Windows, enforce with =FORCE" ON MSVC)
LYX_OPTION(VLD                   "Use VLD with MSVC" OFF MSVC)
LYX_OPTION(WALL                  "Enable all warnings" OFF MSVC)
LYX_OPTION(CONFIGURE_CHECKS      "Also run configure checks for MSVC" OFF MSVC)
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

# Check option dependencies

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
	if(LYX_CONSOLE MATCHES "FORCE")
		set(LYX_CONSOLE ON)
	else()
		set(LYX_CONSOLE OFF)
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
	set(LYX_DEPENDENCIES_DIR ${TOP_BINARY_DIR}/msvc2010-deps)
	message(STATUS "Using downloaded dependencies in ${LYX_DEPENDENCIES_DIR}")
	set(deps_files lyx20-deps-msvc2010-x86.zip)
	set(deps_server http://downloads.sourceforge.net/project/lyx/Win_installers/Dependencies)
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
	set(GNUWIN32_DIR ${LYX_DEPENDENCIES_DIR}/deps20)
    if(MSVC12)
        # handle error in msvc12 when linking against msvc10 libs
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /vd2")
    endif()
endif()



message(STATUS)

set(EXECUTABLE_OUTPUT_PATH  ${TOP_BINARY_DIR}/bin)
if(WIN32)
	set(LIBRARY_OUTPUT_PATH  ${EXECUTABLE_OUTPUT_PATH})
else()
	set(LIBRARY_OUTPUT_PATH  ${TOP_BINARY_DIR}/lib)
endif()


set(LYX_GCC11_MODE)
if(UNIX OR MINGW)
	execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
	message(STATUS "Using GCC version ${GCC_VERSION}")
	if(GCC_VERSION VERSION_LESS 4.9)
		# <regex> in gcc is unusable in versions less than 4.9.0
		# see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631
		set(LYX_USE_STD_REGEX 0)
	endif()
	if (LYX_ENABLE_CXX11)
		find_package(CXX11Compiler)
		if(NOT CXX11COMPILER_FOUND)
			message(FATAL_ERROR "A C++11 compatible compiler is required.")
		endif()
		set(LYX_GCC11_MODE "${CXX11_FLAG}")
	endif()
else()
	set(LYX_USE_STD_REGEX 0)
#	if(MSVC10)
#		set(LYX_USE_STD_REGEX 1) #TODO should we use it in ECMAScript mode?
#	endif()
endif()


set(VERSION_INFO "CMake Build")

# Try to get some informations from configure.ac

include(LyXPaths)
file(STRINGS "${TOP_SRC_DIR}/configure.ac" _config_lines)

if(WIN32)
	set(USE_WINDOWS_PACKAGING ON)
elseif(APPLE)
	set(USE_MACOSX_PACKAGING ON)
else()
	set(USE_POSIX_PACKAGING ON)
endif()

macro(setstripped _varname)
	if(${ARGC} GREATER 1)
		string(STRIP "${ARGV1}" _v)
		if(USE_POSIX_PACKAGING)
			string(TOLOWER ${_v} ${_varname})
		else()
			set(${_varname} ${_v})
		endif()
	else()
		set(${_varname})
	endif()
endmacro(setstripped)

foreach(_c_l ${_config_lines} )
	if(_c_l MATCHES "^AC_INIT\\(\([^,]+\),\([^,]+\), *\\[\([^,]+\)\\] *,\(.*\)")
		setstripped(PACKAGE_BASE ${CMAKE_MATCH_1})
		setstripped(PACKAGE_VERSION ${CMAKE_MATCH_2})
		setstripped(PACKAGE_BUGREPORT ${CMAKE_MATCH_3})
		if(PACKAGE_VERSION MATCHES "^\([0-9]+\)\\.\([0-9]+\)\(\\.\([0-9]+\)\)?.*$")
			set(LYX_MAJOR_VERSION ${CMAKE_MATCH_1})
			set(LYX_MINOR_VERSION ${CMAKE_MATCH_2})
			set(LYX_PATCH_VERSION ${CMAKE_MATCH_4})
			set(LYX_DIR_VER "LYX_DIR_${CMAKE_MATCH_1}${CMAKE_MATCH_2}x")
			set(LYX_USERDIR_VER "LYX_USERDIR_${CMAKE_MATCH_1}${CMAKE_MATCH_2}x")
			set(LYX_INSTALL_SUFFIX "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}")
		endif()
	endif()
	if(_c_l MATCHES "^AC_SUBST\\( *LYX_DATE *, *\\[\\\"(.*)\\\"\\].*")
		set(LYX_DATE "${CMAKE_MATCH_1}")
	endif()
endforeach(_c_l)

FIND_PROGRAM(LYX_GITVERSION git)
#message(STATUS "gitversion = ${LYX_GITVERSION}")
set(LYX_REVISION_VERSION ${LYX_PATCH_VERSION})
if(LYX_GITVERSION)
  if (LYX_LOCALVERSIONING)
    # Find the revision number and for later use
    EXECUTE_PROCESS(COMMAND ${LYX_GITVERSION} describe --match 2.0.0 HEAD
      WORKING_DIRECTORY "${TOP_SRC_DIR}" OUTPUT_VARIABLE LYX_PACKAGE_RELEASE OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (LYX_PACKAGE_RELEASE MATCHES "^2\\.0\\.0\\-\([0-9]+\)\\-.*$")
      # We will add offset of 40000 to get appropriate value to
      # previous svn.
      # We use this value than also to set the package-patch-value
      MATH(EXPR tmp_REVISION_VERSION "(${CMAKE_MATCH_1}+40000)")
      set(LYX_REVISION_VERSION "${LYX_PATCH_VERSION}-${tmp_REVISION_VERSION}git")
      set(ENABLE_DIST ON)
    endif()
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
		configure_file("${CMAKE_CURRENT_SOURCE_DIR}/development/MacOSX/Info.plist.in" "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
		configure_file("${CMAKE_CURRENT_SOURCE_DIR}/development/MacOSX/lyxrc.dist.in" "${CMAKE_CURRENT_BINARY_DIR}/lyxrc.dist")

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

string(REGEX REPLACE "/lyx${LYX_INSTALL_SUFFIX}$" "/share" SYSTEM_DATADIR ${CMAKE_INSTALL_PREFIX})

if(LYX_PACKAGE_SUFFIX)
	set(PACKAGE ${PACKAGE_BASE}${LYX_INSTALL_SUFFIX})
else()
	set(PACKAGE ${PACKAGE_BASE})
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
if(IS_ABSOLUTE)
	set(CMAKE_INSTALL_PREFIX_ABSOLUTE ${CMAKE_INSTALL_PREFIX})
else()
	set(CMAKE_INSTALL_PREFIX_ABSOLUTE ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_PREFIX})
endif()



if(NOT LYX_DATA_SUBDIR)
	if (WIN32)
		set(LYX_DATA_SUBDIR "Resources/" CACHE STRING "Subdirectory for all lyx-system-data" FORCE)
	else()
		set(LYX_DATA_SUBDIR "" CACHE STRING "Subdirectory for all lyx-system-data" FORCE)
	endif()
endif()

set(LYX_ABS_INSTALLED_DATADIR "${CMAKE_INSTALL_PREFIX}")
set(LYX_LOCALEDIR "locale")
set(LYX_ABS_INSTALLED_LOCALEDIR "${LYX_ABS_INSTALLED_DATADIR}/${LYX_DATA_SUBDIR}${LYX_LOCALEDIR}")
set(LYX_ABS_TOP_SRCDIR "${TOP_SRC_DIR}")

if(LYX_BUNDLE AND APPLE)
	set(LYX_MAN_DIR "${LYX_DATA_SUBDIR}" CACHE STRING "Install location for man pages.")
else()
	set(LYX_MAN_DIR "/usr/local/man/man1" CACHE STRING "Install location for man pages.")
endif()
mark_as_advanced(LYX_MAN_DIR)

if(LYX_INSTALL AND WIN32)
	message(STATUS "Configuring NSIS files")
	configure_file(development/Win32/packaging/installer/lyx.nsi.cmake ${CMAKE_BINARY_DIR}/installer/lyx.nsi @ONLY)
	configure_file(development/Win32/packaging/installer/settings.nsh.cmake ${CMAKE_BINARY_DIR}/installer/settings-cmake.nsh @ONLY)
	configure_file(development/Win32/packaging/installer/include/declarations.nsh.cmake ${CMAKE_BINARY_DIR}/installer/declarations-cmake.nsh @ONLY)
	message(STATUS "NSIS files are created in ${CMAKE_BINARY_DIR}/installer")
endif()

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
	set(CMAKE_BUILD_TYPE Profile CACHE TYPE STRING FORCE)
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

if(NOT MSVC)
	if(NOT LYX_QUIET)
		set(CMAKE_VERBOSE_MAKEFILE ON)
	endif()
    set(LYX_CXX_FLAGS "-Wall -Wunused-parameter ${LYX_GCC11_MODE}")
	if(LYX_STDLIB_DEBUG)
		set(LYX_CXX_FLAGS "${LYX_CXX_FLAGS} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
	endif()
	set(CMAKE_CXX_FLAGS       "${LYX_CXX_FLAGS} -fno-strict-aliasing " CACHE TYPE STRING FORCE)
	set(CMAKE_CXX_FLAGS_DEBUG "${LYX_CXX_FLAGS} -fno-strict-aliasing -O0 -g3 -D_DEBUG" CACHE TYPE STRING FORCE)
	if(MINGW)
		set(CMAKE_CXX_FLAGS_RELEASE "${LYX_CXX_FLAGS} -O2 -DNDEBUG" CACHE TYPE STRING FORCE)
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "${LYX_CXX_FLAGS} -O3 -DNDEBUG" CACHE TYPE STRING FORCE)
	endif()
endif()

set(LYX_CXX_FLAGS_EXTRA "" CACHE STRING "Desired semicolon separated list of extra cxx compile flags, like '-Werror'")
mark_as_advanced(LYX_CXX_FLAGS_EXTRA)
if(LYX_CXX_FLAGS_EXTRA)
	foreach(_flag ${LYX_CXX_FLAGS_EXTRA})
		add_definitions(${_flag})
	endforeach()
endif()

if(LYX_XMINGW)
    set(QT_MINGW_DIR ${LYX_QT4} CACHE PATH "Qt for Mingw" FORCE)
    list(APPEND CMAKE_FIND_ROOT_PATH ${QT_MINGW_DIR} ${GNUWIN32_DIR})
endif()
if(LYX_USE_QT MATCHES "QT5")
	find_package(Qt5Core REQUIRED)
	if (Qt5Core_FOUND)
		find_package(Qt5Widgets REQUIRED)
		if(APPLE)
			find_package(Qt5MacExtras REQUIRED)
		endif()
		find_package(Qt5X11Extras QUIET)
		find_package(Qt5WinExtras QUIET)
		set(QTVERSION ${Qt5Core_VERSION})
		macro (qt_use_modules)
			qt5_use_modules(${ARGN})
		endmacro()
		macro (qt_add_resources)
			qt5_add_resources(${ARGN})
		endmacro()
		macro (qt_wrap_uifiles)
			qt5_wrap_ui(${ARGN})
		endmacro()
		message(STATUS "Found Qt-Version ${QTVERSION}")
	endif()
elseif(LYX_USE_QT MATCHES "QT4")
	find_package(Qt4 "4.5.0" REQUIRED)
	macro (qt_use_modules)
	endmacro()
	macro (qt_add_resources)
		qt4_add_resources(${ARGN})
	endmacro()
	macro (qt_wrap_uifiles)
		qt4_wrap_ui(${ARGN})
	endmacro()
else()
  message(FATAL_ERROR "Unhandled value for LYX_USE_QT (${LYX_USE_QT})")
endif()

find_package(Magic)
if(Magic_FOUND)
	set(HAVE_MAGIC_H 1)
endif()

include_directories(${TOP_BINARY_DIR} ${TOP_SRC_DIR}/src)

set(Spelling_FOUND OFF)
set(Include_used_spellchecker)   # String will be inserted into config.h

foreach(_spell "ASPELL" "Enchant" "Hunspell")
  string(TOUPPER ${_spell} _upspell)
  find_package(${_spell})
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

find_package(PythonInterp 2.7 QUIET)
if(PYTHONINTERP_FOUND)
  if(PYTHON_VERSION_STRING VERSION_GREATER 2.8)
    unset(PYTHONINTERP_FOUND)
  endif()
endif()

if (NOT PYTHONINTERP_FOUND)
  unset(PYTHON_EXECUTABLE CACHE)
  unset(PYTHON_VERSION_MAJOR)
  unset(PYTHON_VERSION_MINOR)
  unset(PYTHON_VERSION_STRING)
  find_package(PythonInterp 3.3 REQUIRED)
endif()

set(LYX_PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE} CACHE FILEPATH "Python to be used by LyX")

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
			install(FILES "${TOP_SRC_DIR}/po/${_gmo}" DESTINATION ${LYX_DATA_SUBDIR}${LYX_LOCALEDIR}/${_lang}/LC_MESSAGES RENAME ${PACKAGE}.mo)
		endforeach()
	endif()
endif()


find_package(ICONV REQUIRED)
find_package(ZLIB REQUIRED)

if(LYX_EXTERNAL_BOOST)
	message(STATUS "Searching for boost")
	if(LYX_USE_STD_REGEX)
		find_package(Boost COMPONENTS signals)
	else()
		find_package(Boost COMPONENTS signals regex)
	endif()
	if(Boost_FOUND)
		message(STATUS "Boost found")
		message(STATUS "Boost-libs = ${Boost_LIBRARIES}")
		set(Lyx_Boost_Libraries ${Boost_LIBRARIES})
		if (LYX_STDLIB_DEBUG)
			# Comment from 	Jean-Marc Lasgouttes:
			# In general, system boost libraries are incompatible with
			# the use of stdlib-debug in libstdc++. See ticket #9736 for
			# details.
			message(WARNING "Compiling LyX with stdlib-debug and system boost libraries may lead to crashes. Consider using '-DLYX_STDLIB_DEBUG=OFF' or using '-DLYX_EXTERNAL_BOOST=OFF'")
		endif()
	else()
		message(FATAL_ERROR "Boost not found" ${Boost_ERROR_REASON})
	endif()
else()
	if(LYX_USE_STD_REGEX)
		set(Lyx_Boost_Libraries boost_signals)
	else()
		set(Lyx_Boost_Libraries boost_signals boost_regex)
	endif()
	add_definitions(-DBOOST_USER_CONFIG="<config.h>")
	add_definitions(-DBOOST_SIGNALS_NO_DEPRECATION_WARNING=1)
	include_directories(${TOP_SRC_DIR}/boost)
	add_subdirectory(boost "${TOP_BINARY_DIR}/boost")
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

# Compute qt4-version from ${QTVERSION}

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


if(LYX_DEBUG_GLIBC)
	set(_GLIBCXX_DEBUG ON)
endif()

if(LYX_DEBUG_GLIBC_PEDANTIC)
	set(_GLIBCXX_DEBUG_PEDANTIC ON)
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

# compiler tests, config.h generation
if(MSVC AND NOT LYX_CONFIGURE_CHECKS)
	configure_file(${LYX_CMAKE_DIR}/configCompiler.h.msvc ${TOP_BINARY_DIR}/configCompiler.h)
else()
	include(${LYX_CMAKE_DIR}/ConfigureChecks.cmake)
	configure_file(${LYX_CMAKE_DIR}/configCompiler.h.cmake ${TOP_BINARY_DIR}/configCompiler.h)
endif()

configure_file(${LYX_CMAKE_DIR}/config.h.cmake ${TOP_BINARY_DIR}/config.h)

if(QTVERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+).*")
	MATH(EXPR QT4_VERSION "(${CMAKE_MATCH_1}<<16)|(${CMAKE_MATCH_2}<<8)|${CMAKE_MATCH_3}")
endif()


set (cmd ${CMAKE_CTEST_COMMAND})
if (MSVC)
	set (cmd ${cmd} -C ${CMAKE_CFG_INTDIR})
else ()
	set (cmd ${cmd} -C ${CMAKE_BUILD_TYPE})
endif ()
add_custom_target (lyx_run_tests COMMAND ${cmd})
set_target_properties(lyx_run_tests PROPERTIES FOLDER "tests")

if(LYX_ASAN)
    set(LYX_CXX_FLAGS "-fsanitize=address -fno-omit-frame-pointer -g")
    message(STATUS)
    message(STATUS "Address sanitizer enabled. Usage:")
    message(STATUS "    wget https://llvm.org/svn/llvm-project/compiler-rt/trunk/lib/asan/scripts/asan_symbolize.py")
    message(STATUS "    chmod  +x ./asan_symbolize.py")
    message(STATUS "    ./bin/lyx2.2 2>&1 | ./asan_symbolize.py  | c++filt ")
    message(STATUS)
endif()

if(LYX_INSTALL)
	if(${LYX_PYTHON_EXECUTABLE} MATCHES "-NOTFOUND")
		message(STATUS "Python required to create doc!")
	else()
		if(UNIX)
			add_subdirectory(${LYX_CMAKE_DIR}/man "${TOP_BINARY_DIR}/man")
		endif()
		add_subdirectory(${LYX_CMAKE_DIR}/doc "${TOP_BINARY_DIR}/doc")
	endif()
	include(../Install)
endif()

add_subdirectory(src "${TOP_BINARY_DIR}/src")
add_subdirectory(lib/lyx2lyx "${TOP_BINARY_DIR}/lyx2lyx")
add_subdirectory(lib/scripts "${TOP_BINARY_DIR}/scripts")
add_subdirectory(lib/examples "${TOP_BINARY_DIR}/lib/examples")
add_subdirectory(sourcedoc "${TOP_BINARY_DIR}/sourcedoc")
add_subdirectory(development/autotests "${TOP_BINARY_DIR}/autotests")
if(LYX_ENABLE_URLTESTS)
	add_subdirectory(development/checkurls "${TOP_BINARY_DIR}/checkurls")
endif()

message(STATUS)
message(STATUS "Build params, switch LYX_* options by -DLYX_*=ON or OFF, LYX_* combos by -DLYX_*=value:")
message(STATUS)

LYX_OPTION_LIST_ALL(used)


if(LYX_NLS)
	message(STATUS)
	message(STATUS "Building Native Language Support (LYX_NLS is set), used libraries:")
	message(STATUS)
	message(STATUS "  * iconv")
	message(STATUS "     - header : ${ICONV_HEADER}")
	message(STATUS "     - library: ${ICONV_LIBRARY}")
	message(STATUS "  * zlib")
	message(STATUS "     - header : ${ZLIB_HEADER}")
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
