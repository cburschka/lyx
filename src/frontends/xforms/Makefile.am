AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libxforms.la
INCLUDES = -I${top_srcdir}/src/ \
	-I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS}
LIBS=
LDFLAGS=
LYXDATADIRS = forms
ETAGS_ARGS = --lang=c++
# Alphabetical order please.  It makes it easier to figure out what's missing. 
libxforms_la_SOURCES = \
	ButtonController.h \
	Dialogs.C \
	GUIRunTime.C \
	FormBase.C \
	FormBase.h \
	FormCitation.C \
	FormCitation.h \
	form_citation.C \
	form_citation.h \
	FormCommand.C \
	FormCommand.h \
	FormCopyright.C \
	FormCopyright.h \
	form_copyright.C \
	form_copyright.h \
	FormDocument.C \
	FormDocument.h \
	form_document.C \
	form_document.h \
	FormError.C \
	FormError.h \
	form_error.C \
	form_error.h \
	FormGraphics.C \
	FormGraphics.h \
	form_graphics.C \
	form_graphics.h \
	FormIndex.C \
	FormIndex.h \
	form_index.C \
	form_index.h \
	FormParagraph.C \
	FormParagraph.h \
	form_paragraph.C \
	form_paragraph.h \
	FormPreferences.C \
	FormPreferences.h \
	form_preferences.C \
	form_preferences.h \
	FormPrint.C \
	FormPrint.h \
	form_print.C \
	form_print.h \
	FormRef.C \
	FormRef.h \
	form_ref.C \
	form_ref.h \
	FormTabular.C \
	FormTabular.h \
	form_tabular.C \
	form_tabular.h \
	FormToc.C \
	FormToc.h \
	form_toc.C \
	form_toc.h \
	FormUrl.C \
	FormUrl.h \
	form_url.C \
	form_url.h \
	input_validators.h \
	input_validators.c \
	Menubar_pimpl.C \
	Menubar_pimpl.h \
	RadioButtonGroup.C \
	RadioButtonGroup.h \
	Toolbar_pimpl.C \
	Toolbar_pimpl.h \
	xform_macros.h

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
