# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(tex2lyx)


set(LINKED_FILES
	../FloatList.C
	../Floating.C
	../counters.C
	../lyxlayout.h
	../lyxlayout.C
	../lyxtextclass.C
	../lyxtextclass.h
	../lyxlex.C
	../lyxlex_pimpl.C
)

set(tex2lyx_sources
   	${LINKED_FILES}
	boost.C 
	context.C 
	gettext.C 
	lengthcommon.C 
	lyxfont.C 
	texparser.C 
	tex2lyx.C 
	preamble.C 
	math.C 
	table.C 
	text.C 
)

set(tex2lyx_headers
	Spacing.h 
	context.h 
	gettext.h 
	lyxfont.h 
	texparser.h 
	tex2lyx.h 
)

add_executable(tex2lyx ${tex2lyx_SRCS})

target_link_libraries(tex2lyx  support )

project_source_group("${GROUP_CODE}" tex2lyx_sources tex2lyx_headers)

