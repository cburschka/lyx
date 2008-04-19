# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(lyx)

include_directories(${TOP_SRC_DIR}/src)

add_subdirectory(frontends) 
add_subdirectory(graphics) 
add_subdirectory(insets) 
add_subdirectory(mathed) 
add_subdirectory(support) 
add_subdirectory(tex2lyx) 


file(GLOB lyx_sources ${TOP_SRC_DIR}/src/${LYX_CPP_FILES})
file(GLOB lyx_headers ${TOP_SRC_DIR}/src/${LYX_HPP_FILES})

list(REMOVE_ITEM lyx_sources
	${TOP_SRC_DIR}/src/ASpell.cpp
	${TOP_SRC_DIR}/src/PSpell.cpp
	${TOP_SRC_DIR}/src/ISpell.cpp
	${TOP_SRC_DIR}/src/Variables.cpp
	${TOP_SRC_DIR}/src/Section.cpp)
	
if(ASPELL_FOUND)
	include_directories(${ASPELL_INCLUDE_DIR})
	set(lyx_sources ${lyx_sources} ${TOP_SRC_DIR}/src/ASpell.cpp)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR}
	${ZLIB_INCLUDE_DIR})

lyx_add_msvc_pch(lyx)


if(NOT MERGE_FILES)
	set(lyx_sources ${lyx_sources})
else()
	lyx_const_touched_files(_allinone  lyx_sources)
	set(lyx_sources ${_allinone_files})
endif()

if(LYX_LEAK_DETECTION)
	configure_file(${vld_path}/vld.ini ${CMAKE_CURRENT_BINARY_DIR}/vld.ini COPYONLY)
	configure_file(${vld_path}/vld.ini ${CMAKE_CURRENT_BINARY_DIR}/memory_leak_report.txt COPYONLY)
	set(vld_files ${CMAKE_CURRENT_BINARY_DIR}/vld.ini ${CMAKE_CURRENT_BINARY_DIR}/memory_leak_report.txt)
endif()

add_executable(lyx
	${WIN32_CONSOLE}
	${lyx_sources} 
	${lyx_headers}
	${vld_files}
	)

target_link_libraries(lyx
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

if(ASPELL_FOUND)
	target_link_libraries(lyx ${ASPELL_LIBRARY})
endif()

if(APPLE)
  target_link_libraries(lyx "-bind_at_load" ) 
  target_link_libraries(lyx "-framework Carbon" ) 
endif()

if(MINGW)
  target_link_libraries(lyx ole32)
endif()

project_source_group("${GROUP_CODE}" lyx_sources lyx_headers)

install(TARGETS lyx DESTINATION bin)

