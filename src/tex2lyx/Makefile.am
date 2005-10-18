include $(top_srcdir)/config/common.am

EXTRA_DIST = tex2lyx.man pch.h test-structure.tex test-insets.tex

DISTCLEANFILES += tex2lyx.1 $(LINKED_FILES)

#noinst_LTLIBRARIES = libtexparser.la
#
#libtexparser_la_SOURCES = \
#	texparser.C \
#	texparser.h
#
#tex2lyx_LDADD = libtexparser.la

man_MANS = tex2lyx.1

bin_PROGRAMS = tex2lyx

AM_CPPFLAGS += $(PCH_FLAGS) -I$(srcdir)/.. $(BOOST_INCLUDES)

BOOST_LIBS = $(BOOST_REGEX) $(BOOST_FILESYSTEM)

if USE_COMPRESSION
COMPRESSIONLIB = -lz
endif

LINKED_FILES = \
	FloatList.C \
	Floating.C \
	counters.C \
	lyxlayout.h \
	lyxlayout.C \
	lyxtextclass.C \
	lyxtextclass.h \
	lyxlex.C \
	lyxlex_pimpl.C

BUILT_SOURCES = $(PCH_FILE) $(LINKED_FILES)

tex2lyx_SOURCES = \
	$(LINKED_FILES) \
	Spacing.h \
	boost.C \
	context.C \
	context.h \
	gettext.C \
	gettext.h \
	lengthcommon.C \
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
	$(top_builddir)/src/support/libsupport.la \
	$(BOOST_LIBS) $(COMPRESSIONLIB)

$(LINKED_FILES) :
	@rm -f $@ ; \
	$(LN_S) $(top_srcdir)/src/$@ .

rm_link_files:
	rm -f $(LINKED_FILES)

tex2lyx.1:
	cp -p $(srcdir)/tex2lyx.man tex2lyx.1
