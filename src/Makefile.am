AUTOMAKE_OPTIONS = foreign
SUBDIRS = mathed insets support
DISTCLEANFILES = libintl.h config.h
MAINTAINERCLEANFILES = Makefile.in config.h.in
bin_PROGRAMS = lyx
lyx_LDADD = mathed/mathed.o insets/insets.o support/libsupport.a \
	@INTLLIBS@ @LYX_LIBS@
lyx_DEPENDENCIES = mathed insets support/libsupport.a
EXTRA_DIST = config.h.in stamp-h.in
ETAGS_ARGS = --c++
INCLUDES = -I${top_srcdir}/images
lyx_SOURCES = \
	BackStack.h \
	BufferView.C \
	BufferView.h \
	Bullet.C \
	Bullet.h \
	Chktex.C \
	Chktex.h \
	DepTable.C \
	DepTable.h \
	FontInfo.C \
	FontInfo.h \
	FontLoader.C \
	FontLoader.h \
	ImportLaTeX.C \
	ImportLaTeX.h \
	ImportNoweb.C \
	ImportNoweb.h \
	LString.h \
	LaTeX.C \
	LaTeX.h \
	LaTeXFeatures.C \
	LaTeXFeatures.h \
	LaTeXLog.C \
	Literate.C \
	Literate.h \
	LyXAction.C \
	LyXAction.h \
	LyXSendto.C \
	LyXView.C \
	LyXView.h \
	Makefile.in \
	PaperLayout.C \
	ParagraphExtra.C \
	Spacing.C \
	Spacing.h \
	TableLayout.C \
	bibforms.h \
	bmtable.C \
	bmtable.h \
	broken_const.h \
	broken_headers.h \
	buffer.C \
	buffer.h \
	bufferlist.C \
	bufferlist.h \
	bufferparams.C \
	bufferparams.h \
	bullet_forms.C \
	bullet_forms.h \
	bullet_forms_cb.C \
	chset.C \
	chset.h \
	combox.C \
	combox.h \
	commandtags.h \
	config.h.in \
	credits.C \
	credits.h \
	credits_form.C \
	credits_form.h \
	debug.h \
	definitions.h \
	figure.h \
	filedlg.C \
	filedlg.h \
	form1.C \
	form1.h \
	gettext.h \
	include_form.h \
	intl.C \
	intl.h \
	kbmap.C \
	kbmap.h \
	lastfiles.C \
	lastfiles.h \
	latexoptions.C \
	latexoptions.h \
	layout.C \
	layout.h \
	layout_forms.C \
	layout_forms.h \
	lyx.C \
	lyx.h \
	lyx_cb.C \
	lyx_cb.h \
	lyx_gui.C \
	lyx_gui.h \
	lyx_gui_misc.C \
	lyx_gui_misc.h \
	lyx_main.C \
	lyx_main.h \
	lyx_sendfax.C \
	lyx_sendfax.h \
	lyx_sendfax_main.C \
	lyx_sty.C \
	lyx_sty.h \
	lyxcursor.h \
	lyxdraw.C \
	lyxdraw.h \
	lyxfont.C \
	lyxfont.h \
	lyxfr0.C \
	lyxfr0.h \
	lyxfr1.C \
	lyxfr1.h \
	lyxfunc.C \
	lyxfunc.h \
	lyxlex.C \
	lyxlex.h \
	lyxlookup.C \
	lyxlookup.h \
	lyxparagraph.h \
	lyxrc.C \
	lyxrc.h \
	lyxrow.h \
	lyxscreen.h \
	lyxserver.C \
	lyxserver.h \
	lyxtext.h \
	lyxvc.C \
	lyxvc.h \
	main.C \
	menus.C \
	menus.h \
	minibuffer.C \
	minibuffer.h \
	nt_defines.h \
	os2_defines.h \
	os2_errortable.h \
	paragraph.C \
	pathstack.C \
	pathstack.h \
	print_form.C \
	print_form.h \
	screen.C \
	sp_form.C \
	sp_form.h \
	spellchecker.C \
	spellchecker.h \
	table.C \
	table.h \
	tex-accent.C \
	tex-accent.h \
	tex-defs.h \
	tex-strings.C \
	tex-strings.h \
	texoptions.C \
	texrow.C \
	texrow.h \
	text.C \
	text2.C \
	toolbar.C \
	toolbar.h \
	trans.C \
	trans.h \
	trans_decl.h \
	trans_mgr.C \
	trans_mgr.h \
	undo.h \
	version.h \
	vms_defines.h \
	vspace.C \
	vspace.h

lyx_main.o: lyx_main.C lyx_main.h config.h version.h debug.h gettext.h
	$(CXXCOMPILE) -DLYX_DIR=\"$(LYX_DIR)\" -DTOP_SRCDIR=\"$(top_srcdir)\" \
	-DLYX_NAME=\"$(PACKAGE)\" -c $(top_srcdir)/src/lyx_main.C

main.o: main.C config.h lyx_main.h gettext.h LString.h support/filetools.h
	$(CXXCOMPILE) -DPACKAGE=\"$(PACKAGE)\" -DLOCALEDIR=\"$(localedir)\" \
	-c ${top_srcdir}/src/main.C
