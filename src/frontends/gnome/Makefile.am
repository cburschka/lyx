AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libgnome.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = ${FRONTEND_INCLUDES} -I${top_srcdir}/src/ \
	-I${top_srcdir}/src/frontends/ \
	-I${top_srcdir}/src/frontends/xforms \
	-I${top_srcdir}/src/frontends/controllers \
	${SIGC_CFLAGS} $(BOOST_INCLUDES)

libgnome_la_OBJADD = \
	../xforms/xforms_helpers.lo \
	../xforms/Toolbar_pimpl.lo \
	../xforms/combox.lo \
	../xforms/MathsSymbols.lo \
	../xforms/FormMathsBitmap.lo \
	../xforms/FormMathsDeco.lo \
	../xforms/FormMathsDelim.lo \
	../xforms/FormMathsMatrix.lo \
	../xforms/FormMathsPanel.lo \
	../xforms/FormMathsSpace.lo 
#	../xforms/FileDialog.lo \
#	../xforms/FormFiledialog.lo \
#	../xforms/form_filedialog.lo 
#	../xforms/Timeout_pimpl.lo \
#	../xforms/Color.lo \
#	../xforms/FormFiledialog.lo \
#	../xforms/form_filedialog.lo \
#	../xforms/FormBase.lo \
#	../xforms/FormCitation.lo \
#	../xforms/form_citation.lo \
#	../xforms/FormCopyright.lo \
#	../xforms/form_copyright.lo \
#	../xforms/FormDocument.lo \
#	../xforms/form_document.lo \
#	../xforms/FormError.lo \
#	../xforms/form_error.lo \
#	../xforms/FormGraphics.lo \
#	../xforms/form_graphics.lo \
#	../xforms/FormIndex.lo \
#	../xforms/form_index.lo \
#	../xforms/FormInset.lo \
#	../xforms/FormParagraph.lo \
#	../xforms/form_paragraph.lo \
#	../xforms/FormPreferences.lo \
#	../xforms/form_preferences.lo \
#	../xforms/FormPrint.lo \
#	../xforms/form_print.lo \
#	../xforms/FormRef.lo \
#	../xforms/form_ref.lo \
#	../xforms/FormSplash.lo \
#	../xforms/form_splash.lo \
#	../xforms/FormTabular.lo \
#	../xforms/form_tabular.lo \
#	../xforms/FormTabularCreate.lo \
#	../xforms/form_tabular_create.lo \
#	../xforms/FormToc.lo \
#	../xforms/form_toc.lo \
#	../xforms/input_validators.lo \
#	../xforms/RadioButtonGroup.lo \

LIBS= 
LDFLAGS= $(libgnome_la_OBJADD)
LYXDATADIRS = 
#ETAGS_ARGS = --lang=c++
libgnome_la_SOURCES = \
	gnomeBC.C \
	gnomeBC.h \
	gnome_helpers.C \
	gnome_helpers.h \
	mainapp.C \
	mainapp.h \
	support.c \
	support.h \
	pixbutton.h \
	Dialogs.C \
	GnomeBase.C \
	GnomeBase.h \
	GUIRunTime.C \
	FileDialog.C \
	FormCopyright.C \
	FormCopyright.h \
	FormTabularCreate.C \
	FormTabularCreate.h \
	FormCredits.C \
	FormCredits.h \
	FormError.C \
	FormError.h \
	FormUrl.C \
	FormUrl.h \
	Menubar_pimpl.C \
	Menubar_pimpl.h \
	Timeout_pimpl.C \
	Timeout_pimpl.h

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
