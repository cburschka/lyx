include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libxforms.la

INCLUDES = -I$(top_srcdir)/images -I$(top_srcdir)/src/ \
	-I$(top_srcdir)/src/frontends/ \
	-I$(top_srcdir)/src/frontends/controllers \
	$(SIGC_CFLAGS) $(BOOST_INCLUDES)

SUBDIRS = forms

EXTRA_DIST = xformsGImage.C xformsGImage.h

if USE_BASIC_IMAGE_LOADER
else
XFORMSGIMAGE = xformsGImage.C xformsGImage.h
endif

# Alphabetical order please.  It makes it easier to figure out what's missing.
libxforms_la_SOURCES = \
	Alert_pimpl.C \
	bmtable.c \
	bmtable.h \
	Color.C \
	Color.h \
	combox.C \
	combox.h \
	Dialogs.C \
	DropDown.h \
	DropDown.C \
	FeedbackController.C \
	FeedbackController.h \
	FileDialog.C \
	FormFiledialog.h \
	FormFiledialog.C \
	form_filedialog.C \
	form_filedialog.h \
	FormAboutlyx.C \
	FormAboutlyx.h \
	form_aboutlyx.C \
	form_aboutlyx.h \
	FormBase.C \
	FormBase.h \
	FormBaseDeprecated.C \
	FormBaseDeprecated.h \
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
	FormDocument.C \
	FormDocument.h \
	form_document.C \
	form_document.h \
	FormError.C \
	FormError.h \
	form_error.C \
	form_error.h \
	FormERT.C \
	FormERT.h \
	form_ert.C \
	form_ert.h \
	FormExternal.C \
	FormExternal.h \
	form_external.C \
	form_external.h \
	FormFloat.C \
	FormFloat.h \
	form_float.C \
	form_float.h \
	FormForks.C \
	FormForks.h \
	form_forks.C \
	form_forks.h \
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
	FormMathsBitmap.C \
	FormMathsBitmap.h \
	FormMathsDeco.C \
	FormMathsDeco.h \
	form_maths_deco.C \
	form_maths_deco.h \
	FormMathsDelim.C \
	FormMathsDelim.h \
	form_maths_delim.C \
	form_maths_delim.h \
	FormMathsMatrix.C \
	FormMathsMatrix.h \
	form_maths_matrix.C \
	form_maths_matrix.h \
	FormMathsPanel.C \
	FormMathsPanel.h \
	form_maths_panel.C \
	form_maths_panel.h \
	FormMathsSpace.C \
	FormMathsSpace.h \
	form_maths_space.C \
	form_maths_space.h \
	FormMathsStyle.C \
	FormMathsStyle.h \
	form_maths_style.C \
	form_maths_style.h \
	FormMinipage.C \
	FormMinipage.h \
	form_minipage.C \
	form_minipage.h \
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
	FormSendto.C \
	FormSendto.h \
	form_sendto.C \
	form_sendto.h \
	FormShowFile.C \
	FormShowFile.h \
	FormSpellchecker.C \
	FormSpellchecker.h \
	form_spellchecker.C \
	form_spellchecker.h \
	FormTabular.C \
	FormTabular.h \
	form_tabular.C \
	form_tabular.h \
	FormTabularCreate.C \
	FormTabularCreate.h \
	form_tabular_create.C \
	form_tabular_create.h \
	FormTexinfo.C \
	FormTexinfo.h \
	form_texinfo.C \
	form_texinfo.h \
	FormThesaurus.C \
	FormThesaurus.h \
	form_thesaurus.C \
	form_thesaurus.h \
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
	GUIRunTime.C \
	input_validators.C \
	input_validators.h \
	MathsSymbols.C \
	MathsSymbols.h \
	Menubar_pimpl.C \
	Menubar_pimpl.h \
	RadioButtonGroup.C \
	RadioButtonGroup.h \
	Timeout_pimpl.C \
	Timeout_pimpl.h \
	Toolbar_pimpl.C \
	Toolbar_pimpl.h \
	Tooltips.C \
	Tooltips.h \
	xforms_helpers.C \
	xforms_helpers.h \
	xforms_resize.C \
	xforms_resize.h \
	$(XFORMSGIMAGE) xformsBC.C \
	xformsBC.h

# These still have to be added. Sooner or later. ARRae-20000411
#	GUI_defaults.C \
#	GUI_initialize.C \
#	GUI_postlyxrc.C \
#	GUI_runtime.C \
#	GUI_applymenu.C

libxforms.la: $(libxforms_la_OBJECTS) $(libxforms_la_DEPENDENCIES)

# for convenience only
updatesrc:
	$(MAKE) -C forms updatesrc

# just copied from old lyx repository
#dist-hook:
#	for subdir in $(LYXDATADIRS) ; do \
#	test -d $(distdir)/$$subdir \
#	|| mkdir $(distdir)/$$subdir \
#	|| exit 1; \
#	chmod 777 $(distdir)/$$subdir; \
#	list=`(cd $(srcdir)/$$subdir && ls -1 | grep -v CVS)`; \
#	echo $$list ; \
#	for fil in $$list ; do \
#	cp -p $(srcdir)/$$subdir/$$fil $(distdir)/$$subdir ; \
#	done ; \
#	done
