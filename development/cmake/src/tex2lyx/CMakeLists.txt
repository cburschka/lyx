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
	${TOP_SRC_DIR}/src/lyxlayout.h
	${TOP_SRC_DIR}/src/lyxlayout.C
	${TOP_SRC_DIR}/src/lyxtextclass.C
	${TOP_SRC_DIR}/src/lyxtextclass.h
	${TOP_SRC_DIR}/src/lyxlex.C
	${TOP_SRC_DIR}/src/lyxlex_pimpl.C
)

set(tex2lyx_sources
	${TOP_SRC_DIR}/src/tex2lyx/Spacing.h
	${TOP_SRC_DIR}/src/tex2lyx/boost.C
	${TOP_SRC_DIR}/src/tex2lyx/context.C
	${TOP_SRC_DIR}/src/tex2lyx/
	${TOP_SRC_DIR}/src/tex2lyx/context.h
	${TOP_SRC_DIR}/src/tex2lyx/gettext.C
	${TOP_SRC_DIR}/src/tex2lyx/lengthcommon.C
	${TOP_SRC_DIR}/src/tex2lyx/lyxfont.C
	${TOP_SRC_DIR}/src/tex2lyx/lyxfont.h
	${TOP_SRC_DIR}/src/tex2lyx/texparser.C
	${TOP_SRC_DIR}/src/tex2lyx/texparser.h
	${TOP_SRC_DIR}/src/tex2lyx/tex2lyx.C
	${TOP_SRC_DIR}/src/tex2lyx/tex2lyx.h
	${TOP_SRC_DIR}/src/tex2lyx/preamble.C
	${TOP_SRC_DIR}/src/tex2lyx/math.C
	${TOP_SRC_DIR}/src/tex2lyx/table.C
	${TOP_SRC_DIR}/src/tex2lyx/text.C
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

add_executable(tex2lyx ${tex2lyx_sources} ${LINKED_FILES})

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

project_source_group("${GROUP_CODE}" tex2lyx_sources tex2lyx_headers)

