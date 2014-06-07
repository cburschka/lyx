# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
#

project(support)

file(GLOB support_sources ${TOP_SRC_DIR}/src/support/${LYX_CPP_FILES})
file(GLOB moc_files ${TOP_SRC_DIR}/src/support/${LYX_MOC_FILES})
list(REMOVE_ITEM support_sources ${moc_files} .)
if(APPLE)
	list(APPEND dont_merge ${TOP_SRC_DIR}/src/support/AppleSpeller.m)
	list(APPEND dont_merge ${TOP_SRC_DIR}/src/support/AppleScript.m)
endif()

file(GLOB support_headers ${TOP_SRC_DIR}/src/support/${LYX_HPP_FILES})

file(GLOB support_mythes_sources ${TOP_SRC_DIR}/src/support/mythes/*.cxx)
file(GLOB support_mythes_headers ${TOP_SRC_DIR}/src/support/mythes/*.hxx)

file(GLOB support_linkback_sources ${TOP_SRC_DIR}/src/support/linkback/*.m*)
file(GLOB support_linkback_headers ${TOP_SRC_DIR}/src/support/linkback/*.h)

set_source_files_properties(${support_linkback_sources} 
	${TOP_SRC_DIR}/src/support/AppleSpeller.m
	${TOP_SRC_DIR}/src/support/AppleScript.m
		PROPERTIES COMPILE_FLAGS "-std=c99")

list(REMOVE_ITEM support_sources
	${TOP_SRC_DIR}/src/support/os_win32.cpp
	${TOP_SRC_DIR}/src/support/os_unix.cpp
	${TOP_SRC_DIR}/src/support/os_cygwin.cpp
	${TOP_SRC_DIR}/src/support/os_os2.C
	${TOP_SRC_DIR}/src/support/gettext.cpp)

if(APPLE)
	message(STATUS "Mac LinkBack support")
else()
	set(support_linkback_sources "")
	set(support_linkback_headers "")
endif()

if(UNIX AND CMAKE_COMPILER_IS_GNUCC AND NOT APPLE)
	add_definitions(-DLYX_CALLSTACK_PRINTING)
endif()

add_subdirectory(tests)

# needed to compile tex2lyx in merged mode
set(dont_merge ${dont_merge} ${TOP_SRC_DIR}/src/support/gettext.cpp)

lyx_add_msvc_pch(support)

lyx_automoc(${support_sources})

include_directories(
	${TOP_SRC_DIR}/src/support
	${TOP_BINARY_DIR}/src/support
	${TOP_SRC_DIR}/src/support/mythes
	${QT_INCLUDES}
	${ICONV_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR})


if(NOT LYX_MERGE_FILES)
	set(support_sources ${support_sources} ${support_mythes_sources} ${support_linkback_sources})
	set(support_headers ${support_headers} ${support_mythes_headers} ${support_linkback_headers})
	add_library(support ${library_type} ${support_sources} ${support_headers} ${dont_merge})
else()
	# GCC bug: gcc resolves ::bind as boost::bind
	set(support_separate ${TOP_SRC_DIR}/src/support/socktools.cpp ${TOP_SRC_DIR}/src/support/Messages.cpp)
	list(REMOVE_ITEM support_sources ${support_separate})
	lyx_const_touched_files(_allinone  support_sources)
	set(depends_moc ${support_headers})
	set_source_files_properties(_allinone_const.C
		PROPERTIES OBJECT_DEPENDS "${depends_moc}")
	set_source_files_properties(_allinone_touched.C
		PROPERTIES OBJECT_DEPENDS "${depends_moc}")
	add_library(support ${library_type} ${_allinone_files} ${support_separate}
		${support_mythes_sources} ${support_linkback_sources} ${support_headers} ${dont_merge})
endif()
set_target_properties(support PROPERTIES FOLDER "applications/LyX")

if(USE_MACOSX_PACKAGING AND Qt5Core_FOUND)
       qt_use_modules(support Core Gui)
else()
       qt_use_modules(support Core)
endif()

target_link_libraries(support ${Lyx_Boost_Libraries} ${QT_QTCORE_LIBRARY} ${ZLIB_LIBRARY})

lyx_add_gcc_pch(support)

if(APPLE)
	target_link_libraries(support "objc" "-framework AppKit" "-framework CoreFoundation")
	if(CMAKE_COMPILER_IS_GNUCXX)
		set_source_files_properties(${support_sources} PROPERTIES COMPILE_FLAGS "-fobjc-exceptions")
	endif()
elseif(WIN32)
	target_link_libraries(support shlwapi psapi)
endif()

project_source_group("${GROUP_CODE}" support_sources support_headers)

