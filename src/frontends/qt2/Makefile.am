AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libqt2.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ -I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS} ${FRONTEND_INCLUDES} $(BOOST_INCLUDES) \
	-I${top_srcdir}/src/frontends/xforms
DEFINES = -DQT_CLEAN_NAMESPACE

# moc generated files
BUILTSOURCES = FormCopyrightDialog_moc.C \
	FormCopyrightDialogBase_moc.C

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
	../xforms/FormParagraph.lo \
	../xforms/FormPreferences.lo \
	../xforms/FormPrint.lo \
	../xforms/FormRef.lo \
	../xforms/FormTabular.lo \
	../xforms/FormTabularCreate.lo \
	../xforms/FormToc.lo \
	../xforms/FormUrl.lo \
	../xforms/Menubar_pimpl.lo \
	../xforms/RadioButtonGroup.lo \
	../xforms/Toolbar_pimpl.lo \
	../xforms/form_citation.lo \
	../xforms/form_copyright.lo \
	../xforms/form_document.lo \
	../xforms/form_error.lo \
	../xforms/form_graphics.lo \
	../xforms/form_index.lo \
	../xforms/form_paragraph.lo \
	../xforms/form_preferences.lo \
	../xforms/form_print.lo \
	../xforms/form_ref.lo \
	../xforms/form_tabular.lo \
	../xforms/form_tabular_create.lo \
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
	FormCopyrightDialogBase.h

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
	uic -o $@ $<
FormCopyrightDialogBase.C: FormCopyrightDialogBase.ui
	uic -impl FormCopyrightDialogBase.h -o $@ $<
FormCopyrightDialogBase_moc.C: FormCopyrightDialogBase.h
	moc -o $@ $<
FormCopyrightDialog_moc.C: FormCopyrightDialog.h
	moc -o $@ $<
