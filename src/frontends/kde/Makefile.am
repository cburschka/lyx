AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libkde.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ -I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS} ${FRONTEND_INCLUDES} $(BOOST_INCLUDES) \
	-I${top_srcdir}/src/frontends/xforms

# just to make sure, automake makes them 
BUILTSOURCES =	formcitationdialog_moc.C \
		formcopyrightdialog_moc.C \
		formcopyrightdialogdata_moc.C \
		formindexdialog_moc.C \
		formindexdialogdata_moc.C \
		formprintdialog_moc.C \
		formprintdialogdata_moc.C \
		formrefdialog_moc.C \
		formtocdialog_moc.C \
		formurldialog_moc.C

DISTCLEANFILES = $(BUILTSOURCES) *.orig *.rej *~ *.bak core

libkde_la_OBJADD = \
        ../xforms/FormBase.lo \
        ../xforms/FormDocument.lo \
        ../xforms/form_document.lo \
        ../xforms/FormError.lo \
        ../xforms/form_error.lo \
        ../xforms/FormGraphics.lo \
        ../xforms/form_graphics.lo \
	../xforms/FormInset.lo \
	../xforms/FormParagraph.lo \
	../xforms/form_paragraph.lo \
        ../xforms/FormPreferences.lo \
        ../xforms/form_preferences.lo \
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
	FormCitation.C \
	FormCitation.h \
	FormCopyright.C \
	FormCopyright.h \
	FormIndex.C \
	FormIndex.h \
	FormPrint.C \
	FormPrint.h \
	FormRef.C \
	FormRef.h \
	FormToc.C \
	FormToc.h \
	FormUrl.C \
	FormUrl.h \
	formcitationdialog.C \
	formcitationdialog.h \
	formcopyrightdialog.C \
	formcopyrightdialog.h \
	formcopyrightdialogdata.C \
	formcopyrightdialogdata.h \
	formindexdialog.C \
	formindexdialog.h \
	formindexdialogdata.C \
	formindexdialogdata.h \
	formprintdialog.C \
	formprintdialog.h \
	formprintdialogdata.C \
	formprintdialogdata.h \
	formrefdialog.C \
	formrefdialog.h \
	formtocdialog.C \
	formtocdialog.h \
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

# dialog file dependencies

formcopyrightdialogdata.C formcopyrightdialogdata.h: dlg/formcopyrightdialog.dlg
	$(QTARCH) -g dlg/formcopyrightdialog.dlg
formindexdialogdata.C formindexdialogdata.h: dlg/formindexdialog.dlg
	$(QTARCH) -g dlg/formindexdialog.dlg
formprintdialogdata.C formprintdialogdata.h: dlg/formprintdialog.dlg
	$(QTARCH) -g dlg/formprintdialog.dlg

# add a dependency for every moc file to be full portable
# I've added a key binding to emacs for this. 

formcitationdialog.C: formcitationdialog_moc.C
formcitationdialog_moc.C: formcitationdialog.h
	$(MOC) formcitationdialog.h -o formcitationdialog_moc.C

formcopyrightdialog.C: formcopyrightdialog_moc.C
formcopyrightdialog_moc.C: formcopyrightdialog.h
	$(MOC) formcopyrightdialog.h -o formcopyrightdialog_moc.C
formcopyrightdialogdata_moc.C: formcopyrightdialogdata.h
	$(MOC) formcopyrightdialogdata.h -o formcopyrightdialogdata_moc.C

formindexdialog.C: formindexdialog_moc.C
formindexdialog_moc.C: formindexdialog.h
	$(MOC) formindexdialog.h -o formindexdialog_moc.C
formindexdialogdata_moc.C: formindexdialogdata.h
	$(MOC) formindexdialogdata.h -o formindexdialogdata_moc.C

formprintdialog.C: formprintdialog_moc.C
formprintdialog_moc.C: formprintdialog.h
	$(MOC) formprintdialog.h -o formprintdialog_moc.C
formprintdialogdata_moc.C: formprintdialogdata.h
	$(MOC) formprintdialogdata.h -o formprintdialogdata_moc.C

formrefdialog.C: formrefdialog_moc.C
formrefdialog_moc.C: formrefdialog.h
	$(MOC) formrefdialog.h -o formrefdialog_moc.C

formtocdialog.C: formtocdialog_moc.C
formtocdialog_moc.C: formtocdialog.h
	$(MOC) formtocdialog.h -o formtocdialog_moc.C
 
formurldialog.C: formurldialog_moc.C
formurldialog_moc.C: formurldialog.h
	$(MOC) formurldialog.h -o formurldialog_moc.C
