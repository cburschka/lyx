include $(top_srcdir)/config/common.am

INCLUDES = -I$(srcdir)/../ $(BOOST_INCLUDES)
EXTRA_DIST = test-structure.tex test-insets.tex
DISTCLEANFILES += $(linked_files)

#noinst_LTLIBRARIES = libtexparser.la
#
#libtexparser_la_SOURCES = \
#	texparser.C \
#	texparser.h
#
#tex2lyx_LDADD = libtexparser.la

bin_PROGRAMS = tex2lyx

BUILT_SOURCES = \
	FloatList.C \
	Floating.C \
	counters.C \
	lengthcommon.C \
	lyxlayout.h \
	lyxlayout.C \
	lyxtextclass.C \
	lyxtextclass.h \
	lyxlex.C \
	lyxlex_pimpl.C

tex2lyx_SOURCES = \
	$(BUILT_SOURCES) \
	Spacing.h \
	boost.C \
	context.C \
	context.h \
	gettext.C \
	gettext.h \
	lyxfont.C \
	lyxfont.h \
	texparser.C \
	texparser.h \
	tex2lyx.C \
	tex2lyx.h \
	preamble.C \
	math.C \
	table.C \
	text.C

tex2lyx_LDADD = \
	../support/libsupport.la \
	../../boost/libs/regex/src/libboostregex.la -lz

$(BUILT_SOURCES) :
	@rm -f $@ ; \
	$(LN_S) $(top_srcdir)/src/$@ .

rm_link_files:
	rm -f $(linked_files)
