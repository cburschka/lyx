include $(top_srcdir)/config/common.am

DISTCLEANFILES += config.h libintl.h

MAINTAINERCLEANFILES += $(srcdir)/config.h.in

SUBDIRS = mathed insets graphics support frontends tex2lyx

EXTRA_DIST = config.h.in stamp-h.in cheaders version.C.in \
	Sectioning.h \
	Sectioning.C \
	Variables.C \
	Variables.h \
	paper.h

LYX_PRE_LIBS = mathed/libmathed.la insets/libinsets.la \
	frontends/libfrontends.la 

LYX_POST_LIBS = frontends/controllers/libcontrollers.la \
	graphics/libgraphics.la \
	support/libsupport.la

if USE_INCLUDED_BOOST
BOOST_LIBS = ../boost/libs/regex/src/libboostregex.la \
	../boost/libs/signals/src/libboostsignals.la
else
BOOST_LIBS = -lboost_regex -lboost_signals
endif

OTHERLIBS = $(BOOST_LIBS) $(INTLLIBS) $(AIKSAURUS_LIBS) @LIBS@

bin_PROGRAMS = lyx 
noinst_PROGRAMS = $(FRONTENDS_PROGS)
EXTRA_PROGRAMS = lyx-xforms lyx-qt lyx-gtk

lyx_xforms_LDADD = $(lyx_OBJECTS) $(LYX_PRE_LIBS) \
	frontends/xforms/libxforms.la $(LYX_POST_LIBS) $(OTHERLIBS) 

lyx_xforms_SOURCES = main.C

lyx_qt_LDADD = $(lyx_OBJECTS) $(LYX_PRE_LIBS) \
	frontends/qt2/libqt2.la $(LYX_POST_LIBS) $(OTHERLIBS)
lyx_qt_SOURCES = main.C

lyx_gtk_LDADD = $(lyx_OBJECTS) $(LYX_PRE_LIBS) \
	frontends/gtk/libgtk.la $(LYX_POST_LIBS) $(OTHERLIBS)
lyx_gtk_SOURCES = main.C

lyx$(EXEEXT): $(FRONTENDS_PROGS)
	rm -f $@
	$(LN_S) $< $@

#lyx_LDFLAGS=-Wl,-O1

INCLUDES = $(BOOST_INCLUDES)

BUILT_SOURCES = version.C

lyx_SOURCES = \
	BufferView.C \
	BufferView.h \
	BufferView_pimpl.C \
	BufferView_pimpl.h \
	Bullet.C \
	Bullet.h \
	BranchList.C \
	BranchList.h \
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
	LaTeX.C \
	LaTeX.h \
	LaTeXFeatures.C \
	LaTeXFeatures.h \
	latexrunparams.h \
	LyXAction.C \
	LyXAction.h \
	MenuBackend.C \
	MenuBackend.h \
	paragraph_funcs.C \
	paragraph_funcs.h \
	ParagraphList.h \
	ParagraphParameters.C \
	ParagraphParameters.h \
	ParameterStruct.h \
	PrinterParams.C \
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
	buffer_funcs.C \
	buffer_funcs.h \
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
	dimension.C \
	dimension.h \
	encoding.C \
	encoding.h \
	errorlist.C \
	errorlist.h \
	exporter.C \
	exporter.h \
	gettext.C \
	gettext.h \
	factory.h \
	factory.C \
	format.C \
	format.h \
	funcrequest.h \
	funcrequest.C \
	graph.C \
	graph.h \
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
	messages.C \
	messages.h \
	metricsinfo.C \
	metricsinfo.h \
	paper.h \
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
	tex-accent.C \
	tex-accent.h \
	tex-strings.C \
	tex-strings.h \
	texrow.C \
	texrow.h \
	text.C \
	text2.C \
	text3.C \
	textcursor.C \
	textcursor.h \
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
	vspace.h

dist-hook:
	cd $(distdir) ; rm -rf `find cheaders -name \*CVS\*`

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
