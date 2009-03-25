# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, 2008, 2009 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c)       2008, 2009 Kornel Benko, <Kornel.Benko@berlin.de>
#

set(_tex2lyx tex2lyx${PROGRAM_SUFFIX})
project(${_tex2lyx})

# There is no header file lengthcommon.h
set(LINKED_sources ${TOP_SRC_DIR}/src/lengthcommon.cpp)
set(LINKED_headers)

foreach(_src insets/InsetLayout Color Counters
    Encoding FloatList Floating
    Layout LayoutModuleList Lexer ModuleList TextClass)
  list(APPEND LINKED_sources ${TOP_SRC_DIR}/src/${_src}.cpp)
  list(APPEND LINKED_headers ${TOP_SRC_DIR}/src/${_src}.h)
endforeach(_src)

file(GLOB tex2lyx_sources ${TOP_SRC_DIR}/src/tex2lyx/${LYX_CPP_FILES})

file(GLOB tex2lyx_headers ${TOP_SRC_DIR}/src/tex2lyx/${LYX_HPP_FILES})

include_directories(BEFORE ${TOP_SRC_DIR}/src/tex2lyx
  ${TOP_SRC_DIR}/src/support/minizip ${ZLIB_INCLUDE_DIR})

add_definitions(-DTEX2LYX)

if(NOT MERGE_FILES)
	add_executable(${_tex2lyx} ${tex2lyx_sources} ${LINKED_sources}
		${tex2lyx_headers} ${LINKED_headers})
else()
	lyx_const_touched_files(_allinone  tex2lyx_sources)
	lyx_const_touched_files(_allinone_linked  LINKED_sources)
	add_executable(${_tex2lyx} ${_allinone_files} ${_allinone_linked_files})
endif()


target_link_libraries(${_tex2lyx}
	support
	boost_regex
	${QT_QTCORE_LIBRARY}
	${LIBINTL_LIBRARIES}
	${ICONV_LIBRARY})

if(WIN32)
	target_link_libraries(${_tex2lyx} shlwapi ole32)
endif()

if(APPLE)
	target_link_libraries(${_tex2lyx} "-framework Carbon")
endif()

project_source_group("${GROUP_CODE}" tex2lyx_sources tex2lyx_headers)

install(TARGETS ${_tex2lyx} DESTINATION bin)

