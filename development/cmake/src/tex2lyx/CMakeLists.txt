# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

set(_tex2lyx tex2lyx${PROGRAM_SUFFIX})
project(${_tex2lyx})

set(LINKED_sources
	${TOP_SRC_DIR}/src/insets/InsetLayout.cpp
	${TOP_SRC_DIR}/src/Encoding.cpp
	${TOP_SRC_DIR}/src/FloatList.cpp
	${TOP_SRC_DIR}/src/Floating.cpp
	${TOP_SRC_DIR}/src/Counters.cpp
	${TOP_SRC_DIR}/src/Layout.cpp
	${TOP_SRC_DIR}/src/TextClass.cpp
	${TOP_SRC_DIR}/src/Lexer.cpp
	${TOP_SRC_DIR}/src/lengthcommon.cpp
	${TOP_SRC_DIR}/src/Color.cpp
	${TOP_SRC_DIR}/src/ModuleList.cpp)

set(LINKED_headers
	${TOP_SRC_DIR}/src/Layout.h
	${TOP_SRC_DIR}/src/TextClass.h)

set(tex2lyx_sources
	${TOP_SRC_DIR}/src/tex2lyx/boost.cpp
	${TOP_SRC_DIR}/src/tex2lyx/Context.cpp
	${TOP_SRC_DIR}/src/tex2lyx/Font.cpp
	${TOP_SRC_DIR}/src/tex2lyx/gettext.cpp
	${TOP_SRC_DIR}/src/tex2lyx/Parser.cpp
	${TOP_SRC_DIR}/src/tex2lyx/tex2lyx.cpp
	${TOP_SRC_DIR}/src/tex2lyx/preamble.cpp
	${TOP_SRC_DIR}/src/tex2lyx/math.cpp
	${TOP_SRC_DIR}/src/tex2lyx/table.cpp
	${TOP_SRC_DIR}/src/tex2lyx/text.cpp)

set(tex2lyx_headers
	${TOP_SRC_DIR}/src/tex2lyx/Spacing.h
	${TOP_SRC_DIR}/src/tex2lyx/Context.h
	${TOP_SRC_DIR}/src/tex2lyx/Font.h
	${TOP_SRC_DIR}/src/tex2lyx/Parser.h
	${TOP_SRC_DIR}/src/tex2lyx/tex2lyx.h)

include_directories(BEFORE ${TOP_SRC_DIR}/src/tex2lyx
	${TOP_SRC_DIR}/src/support/minizip ${ZLIB_INCLUDE_DIR})

add_definitions(-DTEX2LYX)

if(NOT MERGE_FILES)
	add_executable(${_tex2lyx} ${tex2lyx_sources} ${LINKED_sources}
		${tex2lyx_headers} ${LINKED_headers})
else()
	lyx_const_touched_files(_allinone  tex2lyx_sources)
	lyx_const_touched_files(_allinone_linked  LINKED_sources)
	add_executable(${_tex2lyx}  ${_allinone_files} ${_allinone_linked_files})
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

install(TARGETS ${_tex2lyx} DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

