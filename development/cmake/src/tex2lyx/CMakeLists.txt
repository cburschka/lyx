# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(tex2lyx)



set(LINKED_FILES
	${TOP_SRC_DIR}/src/FloatList.C
	${TOP_SRC_DIR}/src/Floating.C
	${TOP_SRC_DIR}/src/counters.C
	${TOP_SRC_DIR}/src/lyxlayout.C
	${TOP_SRC_DIR}/src/lyxtextclass.C
	${TOP_SRC_DIR}/src/lyxlex.C
)

set(tex2lyx_sources
	${TOP_SRC_DIR}/src/tex2lyx/boost.C
	${TOP_SRC_DIR}/src/tex2lyx/context.C
	${TOP_SRC_DIR}/src/tex2lyx/gettext.C
	${TOP_SRC_DIR}/src/tex2lyx/lengthcommon.C
	${TOP_SRC_DIR}/src/tex2lyx/lyxfont.C
	${TOP_SRC_DIR}/src/tex2lyx/texparser.C
	${TOP_SRC_DIR}/src/tex2lyx/tex2lyx.C
	${TOP_SRC_DIR}/src/tex2lyx/preamble.C
	${TOP_SRC_DIR}/src/tex2lyx/math.C
	${TOP_SRC_DIR}/src/tex2lyx/table.C
	${TOP_SRC_DIR}/src/tex2lyx/text.C
)

set(tex2lyx_headers
	${TOP_SRC_DIR}/src/lyxlayout.h
	${TOP_SRC_DIR}/src/lyxtextclass.h
	${TOP_SRC_DIR}/src/tex2lyx/Spacing.h
	${TOP_SRC_DIR}/src/tex2lyx/context.h
	${TOP_SRC_DIR}/src/tex2lyx/lyxfont.h
	${TOP_SRC_DIR}/src/tex2lyx/texparser.h
	${TOP_SRC_DIR}/src/tex2lyx/tex2lyx.h
)

include_directories(BEFORE ${TOP_SRC_DIR}/src/tex2lyx)

if(MSVC)
	SET_SOURCE_FILES_PROPERTIES(${TOP_SRC_DIR}/src/lyxlayout.C ${TOP_SRC_DIR}/src/lyxtextclass.C
	 	PROPERTIES COMPILE_FLAGS 
		"/FI${TOP_SRC_DIR}/src/tex2lyx/lyxfont.h /FI${TOP_SRC_DIR}/src/tex2lyx/Spacing.h")
else(MSVC)
	SET_SOURCE_FILES_PROPERTIES(${TOP_SRC_DIR}/src/lyxlayout.C ${TOP_SRC_DIR}/src/lyxtextclass.C
	 	PROPERTIES COMPILE_FLAGS 
		"-include ${TOP_SRC_DIR}/src/tex2lyx/lyxfont.h -include ${TOP_SRC_DIR}/src/tex2lyx/Spacing.h")
endif(MSVC)

#TODO
#if(NOT MERGE_FILES)
	add_executable(tex2lyx ${tex2lyx_sources} ${LINKED_FILES} ${tex2lyx_headers})
#else(NOT MERGE_FILES)
#	set(tex2lyx_sources_all ${tex2lyx_sources} ${LINKED_FILES})
#	lyx_merge_files(${CMAKE_CURRENT_BINARY_DIR}/tex2lyx_allinone.C tex2lyx_sources_all)
#	add_executable(tex2lyx ${CMAKE_CURRENT_BINARY_DIR}/tex2lyx_allinone.C)
#endif(NOT MERGE_FILES)


target_link_libraries(tex2lyx  
	support
	boost_iostreams
	boost_filesystem
	boost_regex
	${QT_QTCORE_LIBRARY} 
	intl
	)
	
if(WIN32)
	target_link_libraries(tex2lyx shlwapi)
endif(WIN32)

if(APPLE)
	target_link_libraries(tex2lyx "-framework Carbon")
endif(APPLE)

project_source_group("${GROUP_CODE}" tex2lyx_sources tex2lyx_headers)

