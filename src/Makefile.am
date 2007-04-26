include $(top_srcdir)/config/common.am

DISTCLEANFILES += config.h libintl.h version.C stamp-version version.C-tmp

MAINTAINERCLEANFILES += $(srcdir)/config.h.in

if BUILD_CLIENT_SUBDIR
CLIENT = client
endif

SUBDIRS = mathed insets graphics support frontends . $(CLIENT) tex2lyx

EXTRA_DIST = config.h.in stamp-h.in version.C.in \
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

OTHERLIBS = $(BOOST_LIBS) $(LIBICONV) $(INTLLIBS) $(AIKSAURUS_LIBS) @LIBS@ $(SOCKET_LIBS)

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

BUILT_SOURCES = $(PCH_FILE) version.C

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
	Bidi.cpp \
	Bidi.h \
	BufferView.cpp \
	BufferView.h \
	Bullet.cpp \
	Bullet.h \
	BranchList.cpp \
	BranchList.h \
	Chktex.cpp \
	Chktex.h \
	color.cpp \
	color.h \
	ConverterCache.cpp \
	ConverterCache.h \
	CutAndPaste.cpp \
	CutAndPaste.h \
	DepTable.cpp \
	DepTable.h \
	FloatList.cpp \
	FloatList.h \
	Floating.cpp \
	Floating.h \
	FontIterator.cpp \
	FontIterator.h \
	FuncStatus.cpp \
	FuncStatus.h \
	InsetList.cpp \
	InsetList.h \
	Color.cpp \
	Color.h \
	LaTeX.cpp \
	LaTeX.h \
	LaTeXFeatures.cpp \
	LaTeXFeatures.h \
	LyXAction.cpp \
	LyXAction.h \
	MenuBackend.cpp \
	MenuBackend.h \
	ParagraphList.h \
	ParagraphList_fwd.h \
	ParagraphParameters.cpp \
	ParagraphParameters.h \
	PrinterParams.cpp \
	PrinterParams.h \
	RowList_fwd.h \
	Spacing.cpp \
	Spacing.h \
	Thesaurus.cpp \
	Thesaurus.h \
	ToolbarBackend.cpp \
	ToolbarBackend.h \
	UpdateFlags.h \
	WordLangTuple.h \
	Author.cpp \
	Author.h \
	boost.cpp \
	Box.h \
	Box.cpp \
	Buffer.cpp \
	Buffer.h \
	buffer_funcs.cpp \
	buffer_funcs.h \
	BufferList.cpp \
	BufferList.h \
	BufferParams.cpp \
	BufferParams.h \
	bufferview_funcs.cpp \
	bufferview_funcs.h \
	Changes.cpp \
	Changes.h \
	config.h.in \
	Converter.cpp \
	Converter.h \
	Counters.cpp \
	Counters.h \
	CoordCache.cpp \
	CoordCache.h \
	Cursor.cpp \
	Cursor.h \
	CursorSlice.cpp \
	CursorSlice.h \
	debug.cpp \
	debug.h \
	Dimension.cpp \
	Dimension.h \
	DocIterator.cpp \
	DocIterator.h \
	DispatchResult.h \
	Encoding.cpp \
	Encoding.h \
	ErrorList.cpp \
	ErrorList.h \
	Exporter.cpp \
	Exporter.h \
	gettext.cpp \
	gettext.h \
	factory.h \
	factory.cpp \
	Format.cpp \
	Format.h \
	FuncRequest.h \
	FuncRequest.cpp \
	Graph.cpp \
	Graph.h \
	Importer.cpp \
	Importer.h \
	Intl.cpp \
	Intl.h \
	InsetIterator.cpp \
	InsetIterator.h \
	kb_keymap.cpp \
	kb_keymap.h \
	kb_sequence.cpp \
	kb_sequence.h \
	Language.cpp \
	Language.h \
	Session.cpp \
	Session.h \
	layout.h \
	lengthcommon.cpp \
	lengthcommon.h \
	Lexer.cpp \
	Lexer.h \
	lfuns.h \
	lyx_cb.cpp \
	lyx_cb.h \
	LyX.cpp \
	LyX.h \
	lyx_sty.cpp \
	lyx_sty.h \
	LyXFont.cpp \
	LyXFont.h \
	lyxfind.cpp \
	lyxfind.h \
	LyXFunc.cpp \
	LyXFunc.h \
	LyXGlueLength.cpp \
	LyXGlueLength.h \
	LyXLayout.cpp \
	LyXLayout.h \
	lyxlayout_ptr_fwd.h \
	LyXLength.cpp \
	LyXLength.h \
	LyXRC.cpp \
	LyXRC.h \
	Row.cpp \
	Row.h \
	LyXServer.cpp \
	LyXServer.h \
	LyXServerSocket.cpp \
	LyXServerSocket.h \
	LyXText.h \
	LyXTextClass.cpp \
	LyXTextClass.h \
	LyXTextClassList.cpp \
	LyXTextClassList.h \
	LyXVC.cpp \
	LyXVC.h \
	Messages.cpp \
	Messages.h \
	MetricsInfo.cpp \
	MetricsInfo.h \
	Mover.cpp \
	Mover.h \
	output.cpp \
	output.h \
	OutputParams.cpp \
	OutputParams.h \
	output_docbook.cpp \
	output_docbook.h \
	output_latex.cpp \
	output_latex.h \
	output_plaintext.cpp \
	output_plaintext.h \
	paper.h \
	Paragraph.cpp \
	Paragraph.h \
	ParagraphMetrics.cpp \
	ParagraphMetrics.h \
	paragraph_funcs.cpp \
	paragraph_funcs.h \
	ParIterator.cpp \
	ParIterator.h \
	$(ASPELL) $(PSPELL) $(ISPELL) SpellBase.cpp \
	SpellBase.h \
	rowpainter.cpp \
	rowpainter.h \
	sgml.cpp \
	sgml.h \
	tex-accent.cpp \
	tex-accent.h \
	tex-strings.cpp \
	tex-strings.h \
	TexRow.cpp \
	TexRow.h \
	text.cpp \
	text2.cpp \
	text3.cpp \
	TextMetrics.cpp \
	TextMetrics.h \
	TocBackend.cpp \
	TocBackend.h \
	toc.cpp \
	toc.h \
	Trans.cpp \
	Trans.h \
	KmodInfo.h \
	TransState.cpp \
	TransState.h \
	Undo.cpp \
	Undo.h \
	VCBackend.cpp \
	VCBackend.h \
	version.C \
	version.h \
	VSpace.cpp \
	VSpace.h

version.C: stamp-version
	@:

stamp-version: version.C-tmp
	if cmp -s $< version.C ; then \
		: ;\
	else \
		rm -f version.C ;\
		cp $< version.C ;\
	fi
	echo timestamp > $@
