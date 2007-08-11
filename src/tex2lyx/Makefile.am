include $(top_srcdir)/config/common.am

EXTRA_DIST = tex2lyx.man pch.h test-structure.tex test-insets.tex

DISTCLEANFILES += tex2lyx.1 $(LINKED_FILES)

#noinst_LTLIBRARIES = libtexparser.la
#
#libtexparser_la_SOURCES = \
#	Parser.cpp \
#	Parser.h
#
#tex2lyx_LDADD = libtexparser.la

man_MANS = tex2lyx.1

bin_PROGRAMS = tex2lyx

AM_CPPFLAGS += $(PCH_FLAGS) -I$(srcdir)/.. $(BOOST_INCLUDES)

LINKED_FILES = \
	FloatList.cpp \
	Floating.cpp \
	Counters.cpp \
	Layout.h \
	Layout.cpp \
	TextClass.cpp \
	TextClass.h \
	Lexer.cpp 

BUILT_SOURCES = $(PCH_FILE) $(LINKED_FILES)

tex2lyx_SOURCES = \
	$(LINKED_FILES) \
	Spacing.h \
	boost.cpp \
	Context.cpp \
	Context.h \
	Font.cpp \
	Font.h \
	gettext.cpp \
	lengthcommon.cpp \
	Parser.cpp \
	Parser.h \
	tex2lyx.cpp \
	tex2lyx.h \
	preamble.cpp \
	math.cpp \
	table.cpp \
	text.cpp

tex2lyx_LDADD = \
	$(top_builddir)/src/support/liblyxsupport.la \
 	$(top_builddir)/src/liblyxcore.la \
 	$(top_builddir)/src/liblyxinsets.la \
 	$(top_builddir)/src/liblyxmathed.la \
 	$(top_builddir)/src/liblyxgraphics.la \
 	$(top_builddir)/src/frontends/liblyxfrontends.la \
 	$(top_builddir)/src/frontends/controllers/liblyxcontrollers.la \
 	$(top_builddir)/src/frontends/qt4/liblyxqt4.la \
	$(BOOST_LIBS) $(LIBICONV) @LIBS@

$(LINKED_FILES) :
	@rm -f $@ ; \
	$(LN_S) $(top_srcdir)/src/$@ .

rm_link_files:
	rm -f $(LINKED_FILES)

tex2lyx.1:
	cp -p $(srcdir)/tex2lyx.man tex2lyx.1
