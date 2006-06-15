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

set(tex2lyx_SRCS 
   	${LINKED_FILES}
	Spacing.h 
	boost.C 
	context.C 
	context.h 
	gettext.C 
	gettext.h 
	lengthcommon.C 
	lyxfont.C 
	lyxfont.h 
	texparser.C 
	texparser.h 
	tex2lyx.C 
	tex2lyx.h 
	preamble.C 
	math.C 
	table.C 
	text.C 
)

add_executable(tex2lyx ${tex2lyx_SRCS})

target_link_libraries(tex2lyx  support )


