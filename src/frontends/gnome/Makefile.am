AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libgnome.la
INCLUDES = ${FRONTEND_INCLUDES} -I${top_srcdir}/src/ \
	-I${top_srcdir}/src/frontends/ -I${top_srcdir}/src/frontends/xforms \
	${SIGC_CFLAGS}
libgnome_la_OBJADD = \
        ../xforms/FormBase.lo \
	../xforms/FormCitation.lo \
	../xforms/form_citation.lo \
	../xforms/FormCommand.lo \
	../xforms/FormDocument.lo \
	../xforms/form_document.lo \
        ../xforms/FormError.lo \
        ../xforms/form_error.lo \
	../xforms/FormGraphics.lo \
	../xforms/form_graphics.lo \
	../xforms/FormIndex.lo \
	../xforms/form_index.lo \
	../xforms/FormParagraph.lo \
	../xforms/form_paragraph.lo \
	../xforms/FormPreferences.lo \
	../xforms/form_preferences.lo \
	../xforms/FormRef.lo \
	../xforms/form_ref.lo \
	../xforms/FormTabular.lo \
	../xforms/form_tabular.lo \
	../xforms/FormToc.lo \
	../xforms/form_toc.lo \
	../xforms/input_validators.lo \
	../xforms/RadioButtonGroup.lo \
	../xforms/Toolbar_pimpl.lo
LIBS= 
LDFLAGS= $(libgnome_la_OBJADD)
LYXDATADIRS = 
#ETAGS_ARGS = --lang=c++
libgnome_la_SOURCES = \
	Dialogs.C \
	GUIRunTime.C \
	FormCitation.C \
	FormCitation.h \
	FormCopyright.C \
	FormCopyright.h \
	FormPrint.C \
	FormPrint.h \
	FormUrl.C \
	FormUrl.h \
	FormIndex.C \
	FormIndex.h \
	FormToc.C \
	FormToc.h \
	Menubar_pimpl.C \
	Menubar_pimpl.h \
	mainapp.C \
	mainapp.h \
	support.c \
	support.h \
	diaprint_callbacks.c \
	diaprint_callbacks.h \
	diaprint_interface.c \
	diaprint_interface.h \
	diainserturl_callbacks.c  \
	diainserturl_interface.c  \
	diainserturl_callbacks.h \
	diainserturl_interface.h \
	diainsertindex_callbacks.c  \
	diainsertindex_interface.c  \
	diainsertindex_callbacks.h \
	diainsertindex_interface.h \
	diatoc_callbacks.c  \
	diatoc_interface.c  \
	diatoc_callbacks.h \
	diatoc_interface.h \
	diainsertcitation_interface.c \
	diainsertcitation_interface.h \
	diainsertcitation_callbacks.c \
	diainsertcitation_callbacks.h \
	pixbutton.h

# These still have to be added. Sooner or later. ARRae-20000411
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
