AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libqt2.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ -I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS} ${FRONTEND_INCLUDES} $(BOOST_INCLUDES) \
	-I${top_srcdir}/src/frontends/xforms \
	-I$(top_srcdir)/src/frontends/controllers
CXXFLAGS += -DQT_CLEAN_NAMESPACE -fno-rtti -fno-exceptions

# moc generated files
BUILTSOURCES = \
	moc_FileDialog_private.C \
	moc_FormCitationDialog.C	\
	moc_FormCitationDialogImpl.C	\
	       moc_chardlg.C \
	       moc_chardlgimpl.C \
	       moc_paragraphdlg.C \
	       moc_paragraphdlgimpl.C \
	       moc_printdlg.C \
	       moc_printdlgimpl.C \
	       moc_searchdlg.C \
	       moc_searchdlgimpl.C \
	       moc_tabularcreatedlg.C \
	       moc_tabularcreatedlgimpl.C \
	       moc_emptytable.C

LYXDATADIRS = dlg
DISTCLEANFILES = $(BUILTSOURCES) *.orig *.rej *~ *.bak core

libqt2_la_OBJADD = \
	../controllers/ButtonController.lo \
	../xforms/combox.lo \
	../xforms/Color.lo \
	../xforms/FormBaseDeprecated.lo	\
	../xforms/FormBibitem.lo \
	../xforms/FormBibtex.lo \
	../xforms/FormBrowser.lo \
	../xforms/FormDocument.lo \
	../xforms/FormError.lo \
	../xforms/FormExternal.lo \
	../xforms/FormGraphics.lo \
	../xforms/FormInclude.lo \
	../xforms/FormIndex.lo \
	../xforms/FormInset.lo \
	../xforms/FormLog.lo \
	../xforms/FormMinipage.lo \
	../xforms/FormPreamble.lo \
	../xforms/FormPreferences.lo \
	../xforms/FormRef.lo \
	../xforms/FormTabular.lo \
	../xforms/FormToc.lo \
	../xforms/FormUrl.lo \
	../xforms/FormVCLog.lo \
	../xforms/MathsSymbols.lo \
	../xforms/Menubar_pimpl.lo \
	../xforms/RadioButtonGroup.lo \
	../xforms/Timeout_pimpl.lo \
	../xforms/Toolbar_pimpl.lo \
	../xforms/bmtable.lo \
	../xforms/form_bibitem.lo \
	../xforms/form_bibtex.lo \
	../xforms/form_browser.lo \
	../xforms/form_credits.lo \
	../xforms/form_document.lo \
	../xforms/form_error.lo \
	../xforms/form_external.lo \
	../xforms/form_index.lo \
	../xforms/form_graphics.lo \
	../xforms/form_include.lo \
	../xforms/form_index.lo \
	../xforms/form_minipage.lo \
	../xforms/form_preamble.lo \
	../xforms/form_preferences.lo \
	../xforms/form_ref.lo \
	../xforms/form_tabular.lo \
	../xforms/form_toc.lo \
	../xforms/form_url.lo \
	../xforms/input_validators.lo \
	../xforms/xformsBC.lo \
	../xforms/xforms_helpers.lo

LIBS=
LDFLAGS= $(libqt2_la_OBJADD)
ETAGS_ARGS = --lang=c++
 
libqt2_la_SOURCES = \
	$(BUILTSOURCES) \
	Dialogs.C \
	GUIRunTime.C \
	FormCharacter.C	\
	FormCharacter.h	\
	chardlg.C \
	chardlg.h \
	chardlgimpl.C \
	chardlgimpl.h \
	FileDialog.C \
	FileDialog_private.C \
	FormBase.C	\
	FormBase.h	\
	FormCitation.C	\
	FormCitation.h	\
	FormCitationDialog.C	\
	FormCitationDialog.h	\
	FormCitationDialogImpl.C	\
	FormCitationDialogImpl.h	\
	FormCopyright.C	\
	FormCopyright.h	\
	FormCopyrightDialog.C	\
	FormCopyrightDialog.h	\
	FormCopyrightDialogImpl.C	\
	FormCopyrightDialogImpl.h	\
	FormParagraph.C	\
	FormParagraph.h	\
	paragraphdlg.C \
	paragraphdlg.h \
	paragraphdlgimpl.C \
	paragraphdlgimpl.h \
	FormPrint.C	\
	FormPrint.h	\
	printdlg.C \
	printdlg.h \
	printdlgimpl.C \
	printdlgimpl.h \
	emptytable.C \
	emptytable.h \
	FormSearch.C	\
	FormSearch.h	\
	searchdlg.C \
	searchdlg.h \
	searchdlgimpl.C \
	searchdlgimpl.h \
	FormSplash.C	\
	FormSplash.h	\
	FormSplashImpl.C	\
	FormSplashImpl.h	\
	FormTabularCreate.C \
	FormTabularCreate.h \
	tabularcreatedlg.C \
	tabularcreatedlg.h \
	tabularcreatedlgimpl.C \
	tabularcreatedlgimpl.h	\
	qt2BC.C	\
	qt2BC.h	

# just copied from old lyx repository
dist-hook:
	for subdir in $(LYXDATADIRS) ; do \
	test -d $(distdir)/$$subdir \
	|| mkdir $(distdir)/$$subdir \
	|| exit 1; \
	chmod 777 $(distdir)/$$subdir; \
	list=`(cd $(srcdir)/$$subdir && ls -1 | grep -v CVS)`; \
	echo $$list ; \
	for fil in $$list ; do \
	cp -p $(srcdir)/$$subdir/$$fil $(distdir)/$$subdir ; \
	done ; \
	done

FormCitation.C: FormCitationDialog.h
FormCitationDialog.h: FormCitationDialog.ui
	$(UIC) -o $@ $<
FormCitationDialog.C: FormCitationDialog.ui
	$(UIC) -impl FormCitationDialog.h -o $@ $<
moc_FormCitationDialog.C: FormCitationDialog.h
	$(MOC) -o $@ $<
moc_FormCitationDialogImpl.C: FormCitationDialogImpl.h
	$(MOC) -o $@ $<

FormCopyright.C: FormCopyrightDialogImpl.h
FormCopyrightDialog.h: FormCopyrightDialog.ui
	$(UIC) -o $@ $<
FormCopyrightDialog.C: FormCopyrightDialog.ui
	$(UIC) -impl FormCopyrightDialog.h -o $@ $<
moc_FormCopyrightDialog.C: FormCopyrightDialog.h
	$(MOC) -o $@ $<
moc_FormCopyrightDialogImpl.C: FormCopyrightDialogImpl.h
	$(MOC) -o $@ $<

FormParagraph.C: paragraphdlg.h
paragraphdlg.h: paragraphdlg.ui
	$(UIC) -o $@ $<
paragraphdlg.C: paragraphdlg.ui
	$(UIC) -impl paragraphdlg.h -o $@ $<
moc_paragraphdlg.C: paragraphdlg.h
	$(MOC) -o $@ $<
moc_paragraphdlgimpl.C: paragraphdlgimpl.h
	$(MOC) -o $@ $<

FormPrint.C: printdlg.h
printdlg.h: printdlg.ui
	$(UIC) -o $@ $<
printdlg.C: printdlg.ui
	$(UIC) -impl printdlg.h -o $@ $<
moc_printdlg.C: printdlg.h
	$(MOC) -o $@ $<
moc_printdlgimpl.C: printdlgimpl.h
	$(MOC) -o $@ $<

FormTabularCreate.C: tabularcreatedlg.h
tabularcreatedlg.h: tabularcreatedlg.ui
	$(UIC) -o $@ $<
tabularcreatedlg.C: tabularcreatedlg.ui
	$(UIC) -impl tabularcreatedlg.h -o $@ $<
moc_tabularcreatedlg.C: tabularcreatedlg.h
	$(MOC) -o $@ $<
moc_tabularcreatedlgimpl.C: tabularcreatedlgimpl.h
	$(MOC) -o $@ $<
moc_emptytable.C: emptytable.h
	$(MOC) -o $@ $<

FormCharacter.C: chardlg.h
chardlg.h: chardlg.ui
	$(UIC) -o $@ $<
chardlg.C: chardlg.ui
	$(UIC) -impl chardlg.h -o $@ $<
moc_chardlg.C: chardlg.h
	$(MOC) -o $@ $<
moc_chardlgimpl.C: chardlgimpl.h
	$(MOC) -o $@ $<

FormSearch.C: searchdlg.h
searchdlg.h: searchdlg.ui
	$(UIC) -o $@ $<
searchdlg.C: searchdlg.ui
	$(UIC) -impl searchdlg.h -o $@ $<
moc_searchdlg.C: searchdlg.h
	$(MOC) -o $@ $<
moc_searchdlgimpl.C: searchdlgimpl.h
	$(MOC) -o $@ $<

moc_FileDialog_private.C: FileDialog_private.h
	$(MOC) -o $@ $<
