include $(top_srcdir)/config/common.am

EXTRA_DIST = TODO.txt tex2lyx.1in $(TEST_FILES) $(TEST_RESULTS) pch.h \
	CMakeLists.txt test/CMakeLists.txt

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

AM_CPPFLAGS += $(PCH_FLAGS) -I$(top_srcdir)/src/tex2lyx \
	-I$(top_srcdir)/src -I$(top_builddir) -I$(top_builddir)/src \
	$(BOOST_INCLUDES)

TEST_FILES = \
	test/runtests.cmake \
	test/runtests.py \
	test/algo2e.tex \
	test/box-color-size-space-align.tex \
	test/CJK.tex \
	test/CJKutf8.tex \
	test/DummyDocument.tex \
	test/Dummy Document.tex \
	test/Dummy~Document.tex \
	test/foo.eps \
	test/foo.png \
	test/test-insets.tex \
	test/test-insets-basic.lyx \
	test/test.ltx \
	test/test-memoir.tex \
	test/test-modules.tex \
	test/test-refstyle-theorems.tex \
	test/test-scr.tex \
	test/test-structure.tex \
	test/XeTeX-polyglossia.tex \
	test/xfigtest.fig \
	test/xfigtest.pstex \
	test/xfigtest.pstex_t

TEST_RESULTS = \
	test/algo2e.lyx.lyx \
	test/box-color-size-space-align.lyx.lyx \
	test/CJK.lyx.lyx \
	test/CJKutf8.lyx.lyx \
	test/DummyDocument.lyx.lyx \
	test/Dummy Document.lyx.lyx \
	test/Dummy~Document.lyx.lyx \
	test/test-insets.lyx.lyx \
	test/test-insets-basic.lyx.lyx \
	test/test.lyx.lyx \
	test/test-memoir.lyx.lyx \
	test/test-modules.lyx.lyx \
	test/test-refstyle-theorems.lyx.lyx \
	test/test-scr.lyx.lyx \
	test/test-structure.lyx.lyx \
	test/XeTeX-polyglossia.lyx.lyx

alltests: check alltests-recursive

alltests-recursive: tex2lyx
	@$(PYTHON) "$(srcdir)/test/runtests.py"; \
	if test $$? -eq 0; then \
		echo -e "=====================\ntex2lyx tests passed.\n====================="; \
	else \
		echo -e "=====================\ntex2lyx tests failed.\n====================="; \
	fi

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
	dummy_impl.cpp \
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
	$(LIBICONV) $(BOOST_LIBS) \
	$(QT_LIB) $(QT_LDFLAGS) \
	@LIBS@ $(LIBSHLWAPI) $(LIBPSAPI)

if INSTALL_MACOSX
tex2lyx_LDFLAGS = -framework AppKit
endif

.PHONY: alltests alltests-recursive updatetests
