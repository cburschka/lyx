AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libkde.la
INCLUDES = -I${top_srcdir}/src/ -I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS} ${FRONTEND_INCLUDES} \
	-I${top_srcdir}/src/frontends/xforms

# just to make sure, automake makes them 
BUILTSOURCES =	formcopyrightdialog_moc.C \
		formurldialog_moc.C \
		formindexdialog_moc.C \
		formcitationdialog_moc.C \
		formtocdialog_moc.C \
		formrefdialog_moc.C

DISTCLEANFILES = $(BUILTSOURCES) *.orig *.rej *~ *.bak core

libkde_la_OBJADD = \
        ../xforms/FormBase.lo \
        ../xforms/FormCommand.lo \
        ../xforms/FormDocument.lo \
        ../xforms/form_document.lo \
        ../xforms/FormError.lo \
        ../xforms/form_error.lo \
        ../xforms/FormGraphics.lo \
        ../xforms/form_graphics.lo \
        ../xforms/FormPreferences.lo \
        ../xforms/form_preferences.lo \
	../xforms/FormPrint.lo \
	../xforms/form_print.lo \
        ../xforms/FormTabular.lo \
        ../xforms/form_tabular.lo \
        ../xforms/input_validators.lo \
        ../xforms/RadioButtonGroup.lo \
        ../xforms/Toolbar_pimpl.lo \
	../xforms/Menubar_pimpl.lo

LIBS=
LDFLAGS= $(libkde_la_OBJADD)
LYXDATADIRS = dlg
ETAGS_ARGS = --lang=c++
libkde_la_SOURCES = \
	Dialogs.C \
	GUIRunTime.C \
	QtLyXView.h \
	FormCopyright.C \
	FormCopyright.h \
	formcopyrightdialog.C \
	formcopyrightdialog.h \
	formcopyrightdialogdata.C \
	FormUrl.C \
	FormUrl.h \
	formurldialog.C \
	formurldialog.h \
	FormIndex.C \
	FormIndex.h \
	formindexdialog.C \
	formindexdialog.h \
	FormCitation.C \
	FormCitation.h \
	formcitationdialog.C \
	formcitationdialog.h \
	FormToc.C \
	FormToc.h \
	formtocdialog.C \
	formtocdialog.h \
	FormRef.C \
	FormRef.h \
	formrefdialog.C \
	formrefdialog.h \
	$(BUILTSOURCES)

# These still have to be added. Sooner or later. ARRae-20000129
#	GUI_defaults.C \
#	GUI_initialize.C \
#	GUI_postlyxrc.C \
#	GUI_runtime.C \
#	GUI_applymenu.C

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

# add a dependency for every moc file to be full portable
# I've added a key binding to emacs for this. 

formcopyrightdialog.C: formcopyrightdialog_moc.C
formcopyrightdialog_moc.C: formcopyrightdialog.h
	$(MOC) formcopyrightdialog.h -o formcopyrightdialog_moc.C

formurldialog.C: formurldialog_moc.C
formurldialog_moc.C: formurldialog.h
	$(MOC) formurldialog.h -o formurldialog_moc.C

formindexdialog.C: formindexdialog_moc.C
formindexdialog_moc.C: formindexdialog.h
	$(MOC) formindexdialog.h -o formindexdialog_moc.C

formcitationdialog.C: formcitationdialog_moc.C
formcitationdialog_moc.C: formcitationdialog.h
	$(MOC) formcitationdialog.h -o formcitationdialog_moc.C

formtocdialog.C: formtocdialog_moc.C
formtocdialog_moc.C: formtocdialog.h
	$(MOC) formtocdialog.h -o formtocdialog_moc.C

formrefdialog.C: formrefdialog_moc.C
formrefdialog_moc.C: formrefdialog.h
	$(MOC) formrefdialog.h -o formrefdialog_moc.C
