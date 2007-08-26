# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(lyx-${qt_postfix})

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
endif(ASPELL_FOUND)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

lyx_add_msvc_pch(lyx)


if(NOT MERGE_FILES)
	set(lyx_sources ${lyx_sources})
else(NOT MERGE_FILES)
	lyx_const_touched_files(_allinone  lyx_sources)
	set(lyx_sources ${_allinone_files})
endif(NOT MERGE_FILES)

add_executable(lyx-${qt_postfix}
	${WIN32_CONSOLE}
	${lyx_sources} 
	${lyx_headers}
	)

target_link_libraries(lyx-${qt_postfix}
	mathed
	insets
	frontends
	frontend_${qt_postfix}
	graphics
	support
	intl
	${QT_QTMAIN_LIBRARY}
	boost_iostreams
)

if(ASPELL_FOUND)
	target_link_libraries(lyx-${qt_postfix} ${ASPELL_LIBRARY})
endif(ASPELL_FOUND)

if(APPLE)
  target_link_libraries(lyx-${qt_postfix} "-bind_at_load" ) 
  target_link_libraries(lyx-${qt_postfix} "-framework Carbon" ) 
endif(APPLE)

project_source_group("${GROUP_CODE}" lyx_sources lyx_headers)

install(TARGETS lyx-${qt_postfix} DESTINATION bin)

