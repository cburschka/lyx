include $(top_srcdir)/config/common.am

EXTRA_DIST = TODO.txt tex2lyx.1in $(TEST_FILES) pch.h

#noinst_LIBRARIES = libtexparser.a
#
#libtexparser_a_SOURCES = \
#	Parser.cpp \
#	Parser.h
#
#tex2lyx_LDADD = libtexparser.a

man_MANS = tex2lyx.1

bin_PROGRAMS = tex2lyx

DEFAULT_INCLUDES = 

AM_CPPFLAGS += -DTEX2LYX $(PCH_FLAGS) -I$(top_srcdir)/src/tex2lyx \
	-I$(top_srcdir)/src -I$(top_builddir) $(BOOST_INCLUDES)

TEST_FILES = \
	test/box-color-size-space-align.tex \
	test/DummyDocument.tex \
	test/foo.eps \
	test/foo.png \
	test/test-insets.tex \
	test/test.ltx \
	test/test-structure.tex

LINKED_FILES = \
	../Color.cpp \
	../Counters.cpp \
	../Encoding.cpp \
	../FloatList.cpp \
	../Floating.cpp \
	../FontInfo.cpp \
	../insets/InsetLayout.cpp \
	../Layout.cpp \
	../LayoutFile.cpp \
	../LayoutModuleList.cpp \
	../lengthcommon.cpp \
	../Lexer.cpp \
	../ModuleList.cpp \
	../Spacing.cpp \
	../TextClass.cpp \
	../TextClass.h \
	../version.cpp

BUILT_SOURCES = $(PCH_FILE)

tex2lyx_SOURCES = \
	$(LINKED_FILES) \
	boost.cpp \
	Context.cpp \
	Context.h \
	math.cpp \
	Parser.cpp \
	Parser.h \
	preamble.cpp \
	table.cpp \
	tex2lyx.cpp \
	tex2lyx.h \
	text.cpp

tex2lyx_LDADD = \
	$(top_builddir)/src/support/liblyxsupport.a \
	$(INTLLIBS) $(LIBICONV) $(BOOST_LIBS) \
	$(QT4_LIB) $(QT4_LDFLAGS) \
	@LIBS@ $(LIBSHLWAPI) $(LIBPSAPI)

if INSTALL_MACOSX
tex2lyx_LDFLAGS = -framework AppKit
endif
