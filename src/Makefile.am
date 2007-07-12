include $(top_srcdir)/config/common.am

DISTCLEANFILES += config.h libintl.h version.cpp stamp-version version.cpp-tmp

MAINTAINERCLEANFILES += $(srcdir)/config.h.in

if BUILD_CLIENT_SUBDIR
CLIENT = client
endif

SUBDIRS = mathed insets graphics support frontends . $(CLIENT) tex2lyx

EXTRA_DIST = config.h.in stamp-h.in version.cpp.in \
	Section.h \
	Section.cpp \
	Variables.cpp \
	Variables.h \
	paper.h \
	pch.h

LYX_PRE_LIBS = mathed/libmathed.la insets/libinsets.la \
	frontends/libfrontends.la

LYX_POST_LIBS = frontends/controllers/libcontrollers.la \
	graphics/libgraphics.la \
	support/libsupport.la

BOOST_LIBS = $(BOOST_REGEX) $(BOOST_SIGNALS) $(BOOST_FILESYSTEM) $(BOOST_IOSTREAMS)

OTHERLIBS = $(BOOST_LIBS) $(INTLLIBS) $(AIKSAURUS_LIBS) @LIBS@ $(SOCKET_LIBS)

bin_PROGRAMS = lyx
noinst_PROGRAMS = $(FRONTENDS_PROGS)
EXTRA_PROGRAMS = lyx-qt4

lyx_qt4_LDADD = $(lyx_OBJECTS) $(LYX_PRE_LIBS) \
	frontends/qt4/libqt4.la $(LYX_POST_LIBS) $(OTHERLIBS)
lyx_qt4_SOURCES = main.cpp

lyx$(EXEEXT): $(FRONTENDS_PROGS)
	rm -f $@
	$(LN_S) $< $@

#lyx_LDFLAGS=-Wl,-O1

BUILT_SOURCES = $(PCH_FILE) version.cpp

AM_CPPFLAGS += $(PCH_FLAGS) $(BOOST_INCLUDES)

if USE_ASPELL
ASPELL = ASpell.cpp ASpell_local.h
endif
if USE_PSPELL
PSPELL = PSpell.cpp PSpell.h
endif
if USE_ISPELL
ISPELL = ISpell.cpp ISpell.h
endif



lyx_SOURCES = \
	$(ASPELL) $(PSPELL) $(ISPELL) SpellBase.cpp \
	Author.cpp \
	Author.h \
	Bidi.cpp \
	Bidi.h \
	boost.cpp \
	Box.cpp \
	Box.h \
	BranchList.cpp \
	BranchList.h \
	Buffer.cpp \
	buffer_funcs.cpp \
	buffer_funcs.h \
	Buffer.h \
	BufferList.cpp \
	BufferList.h \
	BufferParams.cpp \
	BufferParams.h \
	BufferView.cpp \
	bufferview_funcs.cpp \
	bufferview_funcs.h \
	BufferView.h \
	Bullet.cpp \
	Bullet.h \
	Changes.cpp \
	Changes.h \
	Chktex.cpp \
	Chktex.h \
	Color.cpp \
	Color.h \
	config.h.in \
	ConverterCache.cpp \
	ConverterCache.h \
	Converter.cpp \
	Converter.h \
	CoordCache.cpp \
	CoordCache.h \
	Counters.cpp \
	Counters.h \
	Cursor.cpp \
	Cursor.h \
	CursorSlice.cpp \
	CursorSlice.h \
	CutAndPaste.cpp \
	CutAndPaste.h \
	debug.cpp \
	debug.h \
	DepTable.cpp \
	DepTable.h \
	Dimension.cpp \
	Dimension.h \
	DispatchResult.h \
	DocIterator.cpp \
	DocIterator.h \
	Encoding.cpp \
	Encoding.h \
	ErrorList.cpp \
	ErrorList.h \
	Exporter.cpp \
	Exporter.h \
	factory.cpp \
	factory.h \
	Floating.cpp \
	Floating.h \
	FloatList.cpp \
	FloatList.h \
	Font.cpp \
	Font.h \
	FontIterator.cpp \
	FontIterator.h \
	Format.cpp \
	Format.h \
	FuncRequest.cpp \
	FuncRequest.h \
	FuncStatus.cpp \
	FuncStatus.h \
	gettext.cpp \
	gettext.h \
	Graph.cpp \
	Graph.h \
	Importer.cpp \
	Importer.h \
	InsetIterator.cpp \
	InsetIterator.h \
	InsetList.cpp \
	InsetList.h \
	Intl.cpp \
	Intl.h \
	KeyMap.cpp \
	KeyMap.h \
	KeySequence.cpp \
	KeySequence.h \
	Language.cpp \
	Language.h \
	LaTeX.cpp \
	LaTeXFeatures.cpp \
	LaTeXFeatures.h \
	LaTeX.h \
	Layout.cpp \
	Layout.h \
	Length.cpp \
	Length.h \
	lengthcommon.cpp \
	lengthcommon.h \
	Lexer.cpp \
	Lexer.h \
	lfuns.h \
	LyXAction.cpp \
	LyXAction.h \
	callback.cpp \
	callback.h \
	LyX.cpp \
	lyxfind.cpp \
	lyxfind.h \
	LyXFunc.cpp \
	LyXFunc.h \
	LyX.h \
	lyxlayout_ptr_fwd.h \
	LyXRC.cpp \
	LyXRC.h \
	Server.cpp \
	Server.h \
	ServerSocket.cpp \
	ServerSocket.h \
	LyXVC.cpp \
	LyXVC.h \
	MenuBackend.cpp \
	MenuBackend.h \
	Messages.cpp \
	Messages.h \
	MetricsInfo.cpp \
	MetricsInfo.h \
	Mover.cpp \
	Mover.h \
	output.cpp \
	output_docbook.cpp \
	output_docbook.h \
	output.h \
	output_latex.cpp \
	output_latex.h \
	OutputParams.cpp \
	OutputParams.h \
	output_plaintext.cpp \
	output_plaintext.h \
	paper.h \
	Paragraph.cpp \
	paragraph_funcs.cpp \
	paragraph_funcs.h \
	Paragraph.h \
	ParagraphList.h \
	ParagraphMetrics.cpp \
	ParagraphMetrics.h \
	ParagraphParameters.cpp \
	ParagraphParameters.h \
	ParIterator.cpp \
	ParIterator.h \
	PrinterParams.cpp \
	PrinterParams.h \
	Row.cpp \
	Row.h \
	rowpainter.cpp \
	rowpainter.h \
	Session.cpp \
	Session.h \
	sgml.cpp \
	sgml.h \
	Spacing.cpp \
	Spacing.h \
	SpellBase.h \
	TexRow.cpp \
	TexRow.h \
	Text.h \
	Text.cpp \
	Text2.cpp \
	Text3.cpp \
	TextClass.cpp \
	TextClass.h \
	TextClassList.cpp \
	TextClassList.h \
	TextMetrics.cpp \
	TextMetrics.h \
	Thesaurus.cpp \
	Thesaurus.h \
	TocBackend.cpp \
	TocBackend.h \
	toc.cpp \
	toc.h \
	ToolbarBackend.cpp \
	ToolbarBackend.h \
	Trans.cpp \
	Trans.h \
	Undo.cpp \
	Undo.h \
	update_flags.h \
	VCBackend.cpp \
	VCBackend.h \
	version.cpp \
	version.h \
	WordLangTuple.h \
	VSpace.cpp \
	VSpace.h

version.cpp: stamp-version
	@:

stamp-version: version.cpp-tmp
	if cmp -s $< version.cpp ; then \
		: ;\
	else \
		rm -f version.cpp ;\
		cp $< version.cpp ;\
	fi
	echo timestamp > $@
