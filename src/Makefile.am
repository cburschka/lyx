include $(top_srcdir)/config/common.am

DISTCLEANFILES += config.h libintl.h

MAINTAINERCLEANFILES += $(srcdir)/config.h.in

SUBDIRS = mathed insets graphics support frontends

bin_PROGRAMS = lyx

lyx_DEPENDENCIES = mathed/libmathed.o insets/libinsets.o \
	graphics/libgraphics.o \
	frontends/libfrontends.o \
	support/libsupport.o \
	@INCLUDED_SIGC@

lyx_LDADD = $(lyx_DEPENDENCIES) @INTLLIBS@ $(SIGC_LIBS) \
	$(PSPELL_LIBS) @AIKSAURUS_LIBS@ @FRONTEND_LDFLAGS@ @FRONTEND_LIBS@

#lyx_LDFLAGS=-Wl,-O1

EXTRA_DIST = config.h.in stamp-h.in cheaders ext_l10n.h version.C.in \
	Sectioning.h \
	Sectioning.C \
	Variables.C \
	Variables.h \
	counters.C \
	counters.h

INCLUDES = $(SIGC_CFLAGS) $(BOOST_INCLUDES) $(PSPELL_INCLUDES)
localedir = $(datadir)/locale
BUILT_SOURCES = version.C
lyx_SOURCES = \
	BufferView.C \
	BufferView.h \
	BufferView2.C \
	BufferView_pimpl.C \
	BufferView_pimpl.h \
	Bullet.C \
	Bullet.h \
	Chktex.C \
	Chktex.h \
	ColorHandler.C \
	ColorHandler.h \
	CutAndPaste.C \
	CutAndPaste.h \
	DepTable.C \
	DepTable.h \
	FloatList.C \
	FloatList.h \
	Floating.C \
	Floating.h \
	FontInfo.C \
	FontInfo.h \
	FontLoader.C \
	FontLoader.h \
	FuncStatus.C \
	FuncStatus.h \
	LColor.C \
	LColor.h \
	LString.h \
	LaTeX.C \
	LaTeX.h \
	LaTeXFeatures.C \
	LaTeXFeatures.h \
	Lsstream.h \
	LyXAction.C \
	LyXAction.h \
	LyXView.C \
	LyXView.h \
	MenuBackend.C \
	MenuBackend.h \
	Painter.C \
	Painter.h \
	PainterBase.C \
	PainterBase.h \
	ParagraphParameters.C \
	ParagraphParameters.h \
	ParameterStruct.h \
	PrinterParams.h \
	ShareContainer.h \
	Spacing.C \
	Spacing.h \
	TextCache.C \
	TextCache.h \
	Thesaurus.C \
	Thesaurus.h \
	ToolbarDefaults.C \
	ToolbarDefaults.h \
	WorkArea.C \
	WorkArea.h \
	XFormsView.C \
	XFormsView.h \
	box.h \
	box.C \
	broken_headers.h \
	buffer.C \
	buffer.h \
	bufferlist.C \
	bufferlist.h \
	bufferparams.C \
	bufferparams.h \
	bufferview_funcs.C \
	bufferview_funcs.h \
	chset.C \
	chset.h \
	commandtags.h \
	config.h.in \
	converter.C \
	converter.h \
	debug.C \
	debug.h \
	encoding.C \
	encoding.h \
	exporter.C \
	exporter.h \
	font.C \
	font.h \
	gettext.C \
	gettext.h \
	importer.C \
	importer.h \
	intl.C \
	intl.h \
	iterators.C \
	iterators.h \
	kbmap.C \
	kbmap.h \
	kbsequence.C \
	kbsequence.h \
	language.C \
	language.h \
	lastfiles.C \
	lastfiles.h \
	layout.h \
	lengthcommon.C \
	lengthcommon.h \
	lyx_cb.C \
	lyx_cb.h \
	lyx_gui.C \
	lyx_gui.h \
	lyx_main.C \
	lyx_main.h \
	lyx_sty.C \
	lyx_sty.h \
	lyxcursor.C \
	lyxcursor.h \
	lyxfont.C \
	lyxfont.h \
	lyxfind.C \
	lyxfind.h \
	lyxfunc.C \
	lyxfunc.h \
	lyxgluelength.C \
	lyxgluelength.h \
	lyxlayout.C \
	lyxlayout.h \
	lyxlength.C \
	lyxlength.h \
	lyxlex.C \
	lyxlex.h \
	lyxlex_pimpl.C \
	lyxlex_pimpl.h \
	lyxlookup.C \
	lyxlookup.h \
	lyxrc.C \
	lyxrc.h \
	lyxrow.C \
	lyxrow.h \
	lyxscreen.h \
	lyxserver.C \
	lyxserver.h \
	lyxtext.h \
	lyxtextclass.C \
	lyxtextclass.h \
	lyxtextclasslist.C \
	lyxtextclasslist.h \
	lyxvc.C \
	lyxvc.h \
	main.C \
	minibuffer.C \
	minibuffer.h \
	paragraph.C \
	paragraph.h \
	paragraph_pimpl.C \
	paragraph_pimpl.h \
	screen.C \
	sp_base.h \
	sp_spell.C \
	sp_ispell.h \
	sp_pspell.h \
	stl_string_fwd.h \
	tabular.C \
	tabular.h \
	tabular-old.C \
	tabular_funcs.C \
	tabular_funcs.h \
	tex-accent.C \
	tex-accent.h \
	tex-strings.C \
	tex-strings.h \
	texrow.C \
	texrow.h \
	text.C \
	text2.C \
	tracer.C \
	tracer.h \
	trans.C \
	trans.h \
	trans_decl.h \
	trans_mgr.C \
	trans_mgr.h \
	undo.C \
	undo.h \
	undo_funcs.C \
	undo_funcs.h \
	undostack.C \
	undostack.h \
	vc-backend.C \
	vc-backend.h \
	version.C \
	version.h \
	vspace.C \
	vspace.h

lyx_main.o: lyx_main.C lyx_main.h config.h version.h lyx_gui.h \
	lyxrc.h support/path.h support/filetools.h \
	bufferlist.h debug.h support/FileInfo.h lastfiles.h intl.h \
	lyxserver.h layout.h gettext.h kbmap.h commandtags.h language.h 
	$(CXXCOMPILE) -DLYX_DIR=\"$(pkgdatadir)\" \
	  -DTOP_SRCDIR=\"$(top_srcdir)\" -c $(top_srcdir)/src/lyx_main.C

main.o: main.C config.h lyx_main.h gettext.h LString.h support/filetools.h \
	support/os.h frontends/GUIRunTime.h
	$(CXXCOMPILE) -DLOCALEDIR=\"$(localedir)\" -c $(top_srcdir)/src/main.C

dist-hook:
	cd $(distdir) ; rm -rf `find cheaders -name \*CVS\*`
