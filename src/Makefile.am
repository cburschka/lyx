AUTOMAKE_OPTIONS = foreign
SUBDIRS = mathed insets graphics support frontends
DISTCLEANFILES= *.orig *.rej *~ *.bak core libintl.h config.h
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in $(srcdir)/config.h.in
bin_PROGRAMS = lyx
## libfrontends is given twice on the link line because it needs stuff
## from FRONTEND_GUILIB and inversely. If you do not like it, feel
## free to provide another solution... Maybe signals would solve that
## magically, but I do not understand them much (JMarc)
lyx_DEPENDENCIES = mathed/libmathed.la insets/libinsets.la \
	graphics/libgraphics.la \
	frontends/libfrontends.la \
	support/libsupport.la \
	@INCLUDED_SIGC@
lyx_LDADD = $(lyx_DEPENDENCIES) @INTLLIBS@ $(LYX_LIBS) $(SIGC_LIBS) \
	$(PSPELL_LIBS) @FRONTEND_LDFLAGS@ @FRONTEND_LIBS@
#lyx_LDFLAGS=-Wl,-O1
EXTRA_DIST = config.h.in stamp-h.in cheaders
ETAGS_ARGS = --lang=c++
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = $(SIGC_CFLAGS) $(BOOST_INCLUDES) $(PSPELL_INCLUDES) @FRONTEND_INCLUDES@
localedir = $(datadir)/locale
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
	LyXSendto.C \
	LyXView.C \
	LyXView.h \
	Makefile.in \
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
	Sectioning.h \
	Sectioning.C \
	ShareContainer.h \
	Spacing.C \
	Spacing.h \
	TextCache.C \
	TextCache.h \
	ToolbarDefaults.C \
	ToolbarDefaults.h \
	UpdateInset.C \
	UpdateInset.h \
	Variables.C \
	Variables.h \
	WorkArea.C \
	WorkArea.h \
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
	counters.C \
	counters.h \
	debug.C \
	debug.h \
	encoding.C \
	encoding.h \
	exporter.C \
	exporter.h \
	ext_l10n.h \
	figure.h \
	figure_form.C \
	figure_form.h \
	figureForm.C \
	figureForm.h \
	font.C \
	font.h \
	func_status.h \
	gettext.C \
	gettext.h \
	importer.C \
	importer.h \
	intl.C \
	intl.h \
	kbmap.C \
	kbmap.h \
	kbsequence.C \
	kbsequence.h \
	language.C \
	language.h \
	lastfiles.C \
	lastfiles.h \
	layout.C \
	layout.h \
	lyx_cb.C \
	lyx_cb.h \
	lyx_gui.C \
	lyx_gui.h \
	lyx_gui_misc.C \
	lyx_gui_misc.h \
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
	lyxvc.C \
	lyxvc.h \
	main.C \
	minibuffer.C \
	minibuffer.h \
	nt_defines.h \
	os2_defines.h \
	os2_errortable.h \
	paragraph.C \
	paragraph.h \
	paragraph_pimpl.C \
	paragraph_pimpl.h \
	print_form.C \
	print_form.h \
	screen.C \
	sp_form.C \
	sp_form.h \
	spellchecker.C \
	spellchecker.h \
	stl_string_fwd.h \
	tabular.C \
	tabular.h \
	tabular-old.C \
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
	undostack.C \
	undostack.h \
	vc-backend.C \
	vc-backend.h \
	version.h \
	vms_defines.h \
	vspace.C \
	vspace.h

lyx_main.o: lyx_main.C lyx_main.h config.h version.h lyx_gui.h \
	lyx_gui_misc.h lyxrc.h support/path.h support/filetools.h \
	bufferlist.h debug.h support/FileInfo.h lastfiles.h intl.h \
	lyxserver.h layout.h gettext.h kbmap.h commandtags.h language.h
	$(CXXCOMPILE) -DLYX_DIR=\"$(pkgdatadir)\" \
	  -DTOP_SRCDIR=\"$(top_srcdir)\" -c $(top_srcdir)/src/lyx_main.C

main.o: main.C config.h lyx_main.h gettext.h LString.h support/filetools.h
	$(CXXCOMPILE) -DLOCALEDIR=\"$(localedir)\" -c $(top_srcdir)/src/main.C

dist-hook:
	cd $(distdir) ; rm -rf `find cheaders -name \*CVS\*`
