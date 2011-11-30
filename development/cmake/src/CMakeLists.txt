# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
#

set(_lyx ${PACKAGE_BASE}${PROGRAM_SUFFIX})
project(${_lyx})

include_directories(${TOP_SRC_DIR}/src)

add_subdirectory(frontends)
add_subdirectory(graphics)
add_subdirectory(insets)
add_subdirectory(mathed)
add_subdirectory(support)
add_subdirectory(tex2lyx)
if (UNIX)
	add_subdirectory(client)
endif()

file(GLOB lyx_sources ${TOP_SRC_DIR}/src/${LYX_CPP_FILES})
file(GLOB moc_files ${TOP_SRC_DIR}/src/${LYX_MOC_FILES})
list(REMOVE_ITEM  lyx_sources ${moc_files} .)
file(GLOB lyx_headers ${TOP_SRC_DIR}/src/${LYX_HPP_FILES})

list(REMOVE_ITEM lyx_sources
	${TOP_SRC_DIR}/src/Variables.cpp
	${TOP_SRC_DIR}/src/Section.cpp
	${TOP_SRC_DIR}/src/lyxcore.cpp
	${TOP_SRC_DIR}/src/lyxinsets.cpp
	${TOP_SRC_DIR}/src/lyxmathed.cpp)

list(REMOVE_ITEM lyx_headers
	${TOP_SRC_DIR}/src/Variables.h
	${TOP_SRC_DIR}/src/Section.h)

if(NOT APPLE)
	list(REMOVE_ITEM lyx_sources ${TOP_SRC_DIR}/src/AppleSpellChecker.cpp)
	list(REMOVE_ITEM lyx_headers ${TOP_SRC_DIR}/src/AppleSpellChecker.h)
endif()

if(NOT ASPELL_FOUND)
	list(REMOVE_ITEM lyx_sources ${TOP_SRC_DIR}/src/AspellChecker.cpp)
	list(REMOVE_ITEM lyx_headers ${TOP_SRC_DIR}/src/AspellChecker.h)
else()
	set(dont_merge ${dont_merge} ${TOP_SRC_DIR}/src/AspellChecker.cpp)
endif()

if(NOT ENCHANT_FOUND)
	list(REMOVE_ITEM lyx_sources ${TOP_SRC_DIR}/src/EnchantChecker.cpp)
	list(REMOVE_ITEM lyx_headers ${TOP_SRC_DIR}/src/EnchantChecker.h)
else()
	set(dont_merge ${dont_merge} ${TOP_SRC_DIR}/src/EnchantChecker.cpp)
endif()

if(NOT HUNSPELL_FOUND)
	list(REMOVE_ITEM lyx_sources ${TOP_SRC_DIR}/src/HunspellChecker.cpp)
	list(REMOVE_ITEM lyx_headers ${TOP_SRC_DIR}/src/HunspellChecker.h)
else()
	set(dont_merge ${dont_merge} ${TOP_SRC_DIR}/src/HunspellChecker.cpp)
endif()


lyx_automoc(${TOP_SRC_DIR}/src/Compare.cpp)
if(WIN32)
	lyx_automoc(${TOP_SRC_DIR}/src/Server.cpp)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR}
	${ZLIB_INCLUDE_DIR} ${QT_INCLUDES})

lyx_add_msvc_pch(lyx)

if (LYX_DEVEL_VERSION)
	add_definitions(-DDEVEL_VERSION)
endif()

if (LYX_MERGE_FILES)
	if(dont_merge)
		list(REMOVE_ITEM lyx_sources ${dont_merge})
	endif()
	lyx_const_touched_files(_allinone lyx_sources)
	set(lyx_sources ${_allinone_files} ${dont_merge})
endif()

if (LYX_VLD)
	configure_file(${vld_path}/vld.ini ${CMAKE_CURRENT_BINARY_DIR}/vld.ini COPYONLY)
	configure_file(${vld_path}/vld.ini ${CMAKE_CURRENT_BINARY_DIR}/memory_leak_report.txt COPYONLY)
	set(vld_files ${CMAKE_CURRENT_BINARY_DIR}/vld.ini ${CMAKE_CURRENT_BINARY_DIR}/memory_leak_report.txt)
endif()

if(WIN32)
	set(FILE_RC ${CMAKE_SOURCE_DIR}/lyx.rc)
	message(STATUS "Using icon defined in resource file: ${FILE_RC}")
endif()

lyx_find_info_files(LyXInfoFiles ${TOP_SRC_DIR}/*)
lyx_find_info_files(LyXCMakeFiles ${TOP_SRC_DIR}/development/cmake/*.txt)
lyx_find_info_files(LyXCMakeFiles ${TOP_SRC_DIR}/development/cmake/*.cmake)
lyx_find_info_files(LyXCMakeFiles ${TOP_SRC_DIR}/development/cmake/*.h)
lyx_find_info_files(LyXCMakeFiles ${TOP_SRC_DIR}/development/cmake/*.msvc)
lyx_find_info_files(LyXCMakeFiles ${TOP_SRC_DIR}/development/cmake/modules/*)
lyx_find_info_files(LyXUiFiles ${TOP_SRC_DIR}/lib/ui/*)

add_executable(${_lyx}
	${WIN32_CONSOLE}
	${lyx_sources}
	${lyx_headers}
	${vld_files}
	${FILE_RC}
	${lyx_info_files}
	${lyx_cmake_files}
)

lyx_add_gcc_pch(${_lyx})

target_link_libraries(${_lyx}
	mathed
	insets
	frontends
	frontend_qt4
	graphics
	support
	${LIBINTL_LIBRARIES}
	${ICONV_LIBRARY}
	${QT_QTMAIN_LIBRARY}
	${vld_dll})

if(HUNSPELL_FOUND)
	target_link_libraries(${_lyx} ${HUNSPELL_LIBRARY})
endif()
if(ASPELL_FOUND)
	target_link_libraries(${_lyx} ${ASPELL_LIBRARY})
endif()
if(ENCHANT_FOUND)
	target_link_libraries(${_lyx} ${ENCHANT_LIBRARY})
endif()

if(APPLE)
	target_link_libraries(${_lyx} "-bind_at_load")
	target_link_libraries(${_lyx} "-framework AppKit")
endif()

if(MINGW)
	target_link_libraries(${_lyx} ole32)
endif()

project_source_group("${GROUP_CODE}" lyx_sources lyx_headers)

install(TARGETS ${_lyx} DESTINATION bin)

