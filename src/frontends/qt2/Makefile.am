AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libqt2.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ -I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS} ${FRONTEND_INCLUDES} $(BOOST_INCLUDES) \
	-I${top_srcdir}/src/frontends/xforms
DEFINES = -DQT_CLEAN_NAMESPACE
CXXFLAGS= $(DEFINES)

# moc generated files
BUILTSOURCES = 	moc_FormCopyrightDialogBase.C \
                moc_FormCopyrightDialog.C \
	       moc_paragraphdlg.C \
	       moc_paragraphdlgimpl.C \
	       moc_printdlg.C \
	       moc_printdlgimpl.C \
	       moc_tabularcreatedlg.C \
	       moc_tabularcreatedlgimpl.C \
	       moc_emptytable.C

		
LYXDATADIRS = dlg
DISTCLEANFILES = $(BUILTSOURCES) *.orig *.rej *~ *.bak core

libqt2_la_OBJADD = \
	../xforms/ButtonController.lo \
	../xforms/Color.lo \
	../xforms/FormBase.lo \
	../xforms/FormCitation.lo \
	../xforms/FormDocument.lo \
	../xforms/FormError.lo \
	../xforms/FormGraphics.lo \
	../xforms/FormIndex.lo \
	../xforms/FormInset.lo \
	../xforms/FormPreferences.lo \
	../xforms/FormRef.lo \
	../xforms/FormTabular.lo \
	../xforms/FormToc.lo \
	../xforms/FormUrl.lo \
	../xforms/Menubar_pimpl.lo \
	../xforms/RadioButtonGroup.lo \
	../xforms/Toolbar_pimpl.lo \
	../xforms/Timeout_pimpl.lo \
	../xforms/form_citation.lo \
	../xforms/form_copyright.lo \
	../xforms/form_document.lo \
	../xforms/form_error.lo \
	../xforms/form_index.lo \
	../xforms/form_graphics.lo \
	../xforms/form_preferences.lo \
	../xforms/form_ref.lo \
	../xforms/form_tabular.lo \
	../xforms/form_toc.lo \
	../xforms/form_url.lo \
	../xforms/input_validators.lo \
	../xforms/xform_helpers.lo

LIBS=
LDFLAGS= $(libqt2_la_OBJADD)
ETAGS_ARGS = --lang=c++
 
libqt2_la_SOURCES = \
	$(BUILTSOURCES) \
	Dialogs.C \
	GUIRunTime.C \
	FormCopyright.C	\
	FormCopyright.h	\
	FormCopyrightDialog.C \
	FormCopyrightDialog.h \
	FormCopyrightDialogBase.C \
	FormCopyrightDialogBase.h \
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
	FormTabularCreate.C \
	FormTabularCreate.h \
	tabularcreatedlg.C \
	tabularcreatedlg.h \
	tabularcreatedlgimpl.C \
	tabularcreatedlgimpl.h

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

FormCopyright.C: FormCopyrightDialogBase.h
FormCopyrightDialogBase.h: FormCopyrightDialogBase.ui
	$(UIC) -o $@ $<
FormCopyrightDialogBase.C: FormCopyrightDialogBase.ui
	$(UIC) -impl FormCopyrightDialogBase.h -o $@ $<
moc_FormCopyrightDialogBase.C: FormCopyrightDialogBase.h
	$(MOC) -o $@ $<
moc_FormCopyrightDialog.C: FormCopyrightDialog.h
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
