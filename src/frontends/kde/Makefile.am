AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libkde.la
INCLUDES = -I${top_srcdir}/src/ -I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS} ${FRONTEND_INCLUDES} \
	-I${top_srcdir}/src/frontends/xforms

# just to make sure, automake makes them 
BUILTSOURCES =	formcopyrightdialog_moc.C \
		formurldialog_moc.C

DISTCLEANFILES = $(BUILTSOURCES) *.orig *.rej *~ *.bak core

libkde_la_OBJADD = \
        ../xforms/FormCitation.lo \
        ../xforms/form_citation.lo \
        ../xforms/FormCommand.lo \
        ../xforms/FormDocument.lo \
        ../xforms/form_document.lo \
        ../xforms/FormGraphics.lo \
        ../xforms/form_graphics.lo \
        ../xforms/FormIndex.lo \
        ../xforms/form_index.lo \
        ../xforms/FormPreferences.lo \
        ../xforms/form_preferences.lo \
	../xforms/FormPrint.lo \
	../xforms/form_print.lo \
        ../xforms/FormRef.lo \
        ../xforms/form_ref.lo \
        ../xforms/FormTabular.lo \
        ../xforms/form_tabular.lo \
        ../xforms/FormToc.lo \
        ../xforms/form_toc.lo \
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
	FormCopyright.C \
	FormCopyright.h \
	formcopyrightdialog.C \
	formcopyrightdialog.h \
	formcopyrightdialogdata.C \
	FormUrl.C \
	FormUrl.h \
	formurldialog.C \
	formurldialog.h \
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
