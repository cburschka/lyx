AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libkde.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
SUBDIRS = dlg moc
INCLUDES = -I${top_srcdir}/src/ \
	-I${top_srcdir}/src/frontends/ \
	-I${top_srcdir}/src/frontends/controllers \
	${SIGC_CFLAGS} ${FRONTEND_INCLUDES} $(BOOST_INCLUDES) \
	-I${top_srcdir}/src/frontends/xforms

DISTCLEANFILES = $(BUILTSOURCES) *.orig *.rej *~ *.bak core

libkde_la_DEPENDENCIES = moc/libkdemoc.la dlg/libkdedlg.la dlg/moc/libkdedlgmoc.la

libkde_la_OBJADD = \
	moc/libkdemoc.la \
	dlg/libkdedlg.la \
	dlg/moc/libkdedlgmoc.la \
        ../xforms/FormBaseDeprecated.lo \
	../xforms/FormBase.lo \
	../xforms/FormBibitem.lo \
	../xforms/form_bibitem.lo \
	../xforms/FormBibtex.lo \
	../xforms/form_bibtex.lo \
	../xforms/FormCharacter.lo \
	../xforms/form_character.lo \
        ../xforms/FormCredits.lo \
        ../xforms/form_credits.lo \
        ../xforms/FormError.lo \
        ../xforms/form_error.lo \
	../xforms/FormExternal.lo \
	../xforms/form_external.lo \
        ../xforms/FormGraphics.lo \
        ../xforms/form_graphics.lo \
	../xforms/FormInclude.lo \
	../xforms/form_include.lo \
	../xforms/FormInset.lo \
	../xforms/FormMathsBitmap.lo \
	../xforms/FormMathsDeco.lo \
	../xforms/form_maths_deco.lo \
	../xforms/FormMathsDelim.lo \
	../xforms/form_maths_delim.lo \
	../xforms/FormMathsMatrix.lo \
	../xforms/form_maths_matrix.lo \
	../xforms/FormMathsPanel.lo \
	../xforms/form_maths_panel.lo \
	../xforms/FormMathsSpace.lo \
	../xforms/form_maths_space.lo \
	../xforms/FormMinipage.lo \
	../xforms/form_minipage.lo \
        ../xforms/FormPreamble.lo \
        ../xforms/form_preamble.lo \
        ../xforms/FormPreferences.lo \
        ../xforms/form_preferences.lo \
	../xforms/FormSearch.lo \
	../xforms/form_search.lo \
	../xforms/FormSplash.lo \
	../xforms/form_splash.lo \
        ../xforms/FormTabular.lo \
        ../xforms/form_tabular.lo \
	../xforms/MathsSymbols.lo \
	../xforms/xformsBC.lo \
	../xforms/bmtable.lo \
	../xforms/Color.lo \
	../xforms/combox.lo \
	../xforms/xforms_helpers.lo \
        ../xforms/input_validators.lo \
        ../xforms/RadioButtonGroup.lo \
        ../xforms/Toolbar_pimpl.lo \
	../xforms/Menubar_pimpl.lo

LIBS=
LDFLAGS= $(libkde_la_OBJADD)
ETAGS_ARGS = --lang=c++
libkde_la_SOURCES = \
	Dialogs.C \
	FileDialog.C \
	FileDialog_private.C \
	FileDialog_private.h \
	GUIRunTime.C \
	QtLyXView.h \
	Timeout_pimpl.C \
	Timeout_pimpl.h \
	kdeBC.C \
	kdeBC.h \
	KFormBase.C \
	KFormBase.h \
	FormCitation.C \
	FormCitation.h \
	FormCopyright.C \
	FormCopyright.h \
	FormDocument.C \
	FormDocument.h \
	FormIndex.C \
	FormIndex.h \
	FormLog.C \
	FormLog.h \
	FormParagraph.C \
	FormParagraph.h \
	FormPrint.C \
	FormPrint.h \
	FormRef.C \
	FormRef.h \
	FormTabularCreate.C \
	FormTabularCreate.h \
	FormUrl.C \
	FormUrl.h \
	FormVCLog.C \
	FormVCLog.h \
	citationdlg.C \
	citationdlg.h \
	copyrightdlg.C \
	copyrightdlg.h \
	docdlg.C \
	docdlg.h \
	indexdlg.C \
	indexdlg.h \
	logdlg.C \
	logdlg.h \
	paradlg.C \
	paradlg.h \
	parageneraldlg.C \
	parageneraldlg.h \
	paraextradlg.C \
	paraextradlg.h \
	printdlg.C \
	printdlg.h \
	refdlg.C \
	refdlg.h \
	tabcreatedlg.C \
	tabcreatedlg.h \
	urldlg.C \
	urldlg.h \
	vclogdlg.C \
	vclogdlg.h

# FIXME 
#	FormToc.C \
#	FormToc.h \
#	tocdlg.C \
#	tocdlg.h \
 
moc/FileDialog_private_moc.C: FileDialog_private.C FileDialog_private.h
moc/citationdlg_moc.C: citationdlg.C citationdlg.h
moc/copyrightdlg_moc.C: copyrightdlg.C copyrightdlg.h
moc/docdlg_moc.C: docdlg.C docdlg.h
moc/logdlg_moc.C: logdlg.C logdlg.h
moc/indexdlg_moc.C: indexdlg.C indexdlg.h
moc/paradlg_moc.C: paradlg.C paradlg.h
moc/parageneraldlg_moc.C: parageneraldlg.C parageneraldlg.h
moc/paraextradlg_moc.C: paraextradlg.C paraextradlg.h
moc/printdlg_moc.C: printdlg.C printdlg.h
moc/refdlg_moc.C: refdlg.C refdlg.h
moc/tabcreatedlg_moc.C: tabcreatedlg.C tabcreatedlg.h
moc/tocdlg_moc.C: tocdlg.C tocdlg.h
moc/urldlg_moc.C: urldlg.C urldlg.h
moc/vclogdlg_moc.C: vclogdlg.C vclogdlg.h
