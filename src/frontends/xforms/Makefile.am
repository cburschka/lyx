AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libxforms.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ \
	-I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS} $(BOOST_INCLUDES)
LIBS=
LDFLAGS=
LYXDATADIRS = forms
ETAGS_ARGS = --lang=c++
# Alphabetical order please.  It makes it easier to figure out what's missing. 
libxforms_la_SOURCES = \
	ButtonController.C \
	ButtonController.h \
	Color.C \
	Color.h \
	Dialogs.C \
	FileDialog.C \
	FormFiledialog.h \
	FormFiledialog.C \
	form_filedialog.C \
	form_filedialog.h \
	GUIRunTime.C \
	FormBase.C \
	FormBase.h \
	FormBibitem.C \
	FormBibitem.h \
	form_bibitem.C \
	form_bibitem.h \
	FormBibtex.C \
	FormBibtex.h \
	form_bibtex.C \
	form_bibtex.h \
	FormBrowser.C \
	FormBrowser.h \
	form_browser.C \
	form_browser.h \
	FormCharacter.C \
	FormCharacter.h \
	form_character.C \
	form_character.h \
	FormCitation.C \
	FormCitation.h \
	form_citation.C \
	form_citation.h \
	FormCopyright.C \
	FormCopyright.h \
	form_copyright.C \
	form_copyright.h \
	FormCredits.C \
	FormCredits.h \
	form_credits.C \
	form_credits.h \
	FormDocument.C \
	FormDocument.h \
	form_document.C \
	form_document.h \
	FormError.C \
	FormError.h \
	form_error.C \
	form_error.h \
	FormExternal.C \
	FormExternal.h \
	form_external.C \
	form_external.h \
	FormGraphics.C \
	FormGraphics.h \
	form_graphics.C \
	form_graphics.h \
	FormInclude.C \
	FormInclude.h \
	form_include.C \
	form_include.h \
	FormIndex.C \
	FormIndex.h \
	form_index.C \
	form_index.h \
	FormInset.C \
	FormInset.h \
	FormLog.C \
	FormLog.h \
	FormParagraph.C \
	FormParagraph.h \
	form_paragraph.C \
	form_paragraph.h \
	FormPreamble.C \
	FormPreamble.h \
	form_preamble.C \
	form_preamble.h \
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
	FormSearch.C \
	FormSearch.h \
	form_search.C \
	form_search.h \
	FormSplash.C \
	FormSplash.h \
	form_splash.C \
	form_splash.h \
	FormTabular.C \
	FormTabular.h \
	form_tabular.C \
	form_tabular.h \
	FormTabularCreate.C \
	FormTabularCreate.h \
	form_tabular_create.C \
	form_tabular_create.h \
	FormToc.C \
	FormToc.h \
	form_toc.C \
	form_toc.h \
	FormUrl.C \
	FormUrl.h \
	form_url.C \
	form_url.h \
	FormVCLog.C \
	FormVCLog.h \
	input_validators.h \
	input_validators.C \
	Menubar_pimpl.C \
	Menubar_pimpl.h \
	RadioButtonGroup.C \
	RadioButtonGroup.h \
	Timeout_pimpl.C \
	Timeout_pimpl.h \
	Toolbar_pimpl.C \
	Toolbar_pimpl.h \
	xforms_helpers.C \
	xforms_helpers.h

# These still have to be added. Sooner or later. ARRae-20000411
#	GUI_defaults.C \
#	GUI_initialize.C \
#	GUI_postlyxrc.C \
#	GUI_runtime.C \
#	GUI_applymenu.C

# for convenience only
updatesrc:
	$(MAKE) -C forms updatesrc
 
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
