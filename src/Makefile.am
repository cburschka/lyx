include $(top_srcdir)/config/common.am

DISTCLEANFILES += config.h libintl.h

MAINTAINERCLEANFILES += $(srcdir)/config.h.in

SUBDIRS = mathed insets graphics support frontends tex2lyx

bin_PROGRAMS = lyx

LYX_CONV_LIBS = mathed/libmathed.la insets/libinsets.la \
	frontends/libfrontends.la \
	graphics/libgraphics.la \
	support/libsupport.la

if USE_INCLUDED_BOOST
BOOST_LIBS = ../boost/libs/regex/src/libboostregex.la \
	../boost/libs/signals/src/libboostsignals.la
else
BOOST_LIBS = -lboost_regex -lboost_signals
endif

lyx_LDADD = $(LYX_CONV_LIBS) $(BOOST_LIBS) $(INTLLIBS) \
	$(AIKSAURUS_LIBS) @LIBS@

#lyx_LDFLAGS=-Wl,-O1

EXTRA_DIST = config.h.in stamp-h.in cheaders version.C.in \
	Sectioning.h \
	Sectioning.C \
	Variables.C \
	Variables.h \
	tracer.C \
	tracer.h

INCLUDES = $(BOOST_INCLUDES)

localedir = $(datadir)/locale

BUILT_SOURCES = version.C

lyx_SOURCES = \
	BufferView.C \
	BufferView.h \
	BufferView_pimpl.C \
	BufferView_pimpl.h \
	Bullet.C \
	Bullet.h \
	Chktex.C \
	Chktex.h \
	CutAndPaste.C \
	CutAndPaste.h \
	DepTable.C \
	DepTable.h \
	FloatList.C \
	FloatList.h \
	Floating.C \
	Floating.h \
	FuncStatus.C \
	FuncStatus.h \
	InsetList.C \
	InsetList.h \
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
	MenuBackend.C \
	MenuBackend.h \
	paragraph_funcs.C \
	paragraph_funcs.h \
	ParagraphList.C \
	ParagraphList.h \
	ParagraphParameters.C \
	ParagraphParameters.h \
	ParameterStruct.h \
	PrinterParams.h \
	RowList.h \
	ShareContainer.h \
	Spacing.C \
	Spacing.h \
	TextCache.C \
	TextCache.h \
	Thesaurus.C \
	Thesaurus.h \
	ToolbarBackend.C \
	ToolbarBackend.h \
	WordLangTuple.h \
	aspell.C \
	aspell_local.h \
	author.C \
	author.h \
	boost.C \
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
	changes.C \
	changes.h \
	chset.C \
	chset.h \
	lfuns.h \
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
	gettext.C \
	gettext.h \
	factory.h \
	factory.C \
	funcrequest.h \
	funcrequest.C \
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
	lyxlayout_ptr_fwd.h \
	lyxlength.C \
	lyxlength.h \
	lyxlex.C \
	lyxlex.h \
	lyxlex_pimpl.C \
	lyxlex_pimpl.h \
	lyxrc.C \
	lyxrc.h \
	lyxrow.C \
	lyxrow.h \
	lyxrow_funcs.C \
	lyxrow_funcs.h \
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
	messages.C \
	messages.h \
	metricsinfo.C \
	metricsinfo.h \
	paragraph.C \
	paragraph.h \
	paragraph_pimpl.C \
	paragraph_pimpl.h \
	SpellBase.h \
	ispell.C \
	ispell.h \
	pspell.C \
	pspell.h \
	rowpainter.C \
	rowpainter.h \
	sgml.C \
	sgml.h \
	tabular.C \
	tabular.h \
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
	text3.C \
	text_funcs.C \
	text_funcs.h \
	toc.C \
	toc.h \
	trans.C \
	trans.h \
	trans_decl.h \
	trans_mgr.C \
	trans_mgr.h \
	undo.C \
	undo.h \
	undo_funcs.C \
	undo_funcs.h \
	vc-backend.C \
	vc-backend.h \
	version.C \
	version.h \
	vspace.C \
	vspace.h \
	graph.C \
	graph.h \
	format.C \
	format.h

lyx_main.o: lyx_main.C lyx_main.h config.h version.h \
	lyxrc.h support/path.h support/filetools.h \
	bufferlist.h debug.h support/FileInfo.h lastfiles.h intl.h \
	lyxserver.h layout.h gettext.h kbmap.h lfuns.h language.h
	$(CXXCOMPILE) -DLYX_DIR=\"$(pkgdatadir)\" \
	  -DTOP_SRCDIR=\"$(top_srcdir)\" -c $(top_srcdir)/src/lyx_main.C

messages.o: messages.C messages.h config.h LString.h support/filetools.h
	$(CXXCOMPILE) -DLOCALEDIR=\"$(localedir)\" -c $(top_srcdir)/src/messages.C

dist-hook:
	cd $(distdir) ; rm -rf `find cheaders -name \*CVS\*`
