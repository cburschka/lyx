include $(top_srcdir)/config/common.am

EXTRA_DIST = TODO.txt tex2lyx.1in $(TEST_FILES) $(TEST_RESULTS) pch.h

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
	test/runtests.py \
	test/box-color-size-space-align.tex \
	test/CJK.tex \
	test/CJKutf8.tex \
	test/DummyDocument.tex \
	test/Dummy~Document.tex \
	test/Dummy\ Document.tex \
	test/foo.eps \
	test/foo.png \
	test/test-insets.tex \
	test/test.ltx \
	test/test-modules.tex \
	test/test-refstyle-references.tex \
	test/test-structure.tex \
	test/XeTeX-polyglossia.tex \
	test/xfigtest.fig \
	test/xfigtest.pstex \
	test/xfigtest.pstex_t

TEST_RESULTS = \
	test/box-color-size-space-align.lyx.lyx \
	test/CJK.lyx.lyx \
	test/CJKutf8.lyx.lyx \
	test/DummyDocument.lyx.lyx \
	test/Dummy~Document.lyx.lyx \
	test/Dummy\ Document.lyx.lyx \
	test/test-insets.lyx.lyx \
	test/test.lyx.lyx \
	test/test-modules.lyx.lyx \
	test/test-structure.lyx.lyx \
	test/XeTeX-polyglossia.lyx.lyx

TESTS = test/runtests.py
updatetests: tex2lyx
	$(PYTHON) "$(srcdir)/test/runtests.py" ./tex2lyx "$(top_srcdir)/lib/scripts" "$(srcdir)/test"

LINKED_FILES = \
	../Author.cpp \
	../Color.cpp \
	../Counters.cpp \
	../Encoding.cpp \
	../FloatList.cpp \
	../Floating.cpp \
	../FontInfo.cpp \
	../graphics/GraphicsParams.cpp \
	../insets/ExternalTemplate.cpp \
	../insets/ExternalTransforms.cpp \
	../insets/InsetLayout.cpp \
	../LaTeXPackages.cpp \
	../Layout.cpp \
	../LayoutFile.cpp \
	../LayoutModuleList.cpp \
	../Length.cpp \
	../lengthcommon.cpp \
	../Lexer.cpp \
	../ModuleList.cpp \
	../Spacing.cpp \
	../TextClass.cpp \
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
	Preamble.cpp \
	Preamble.h \
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
