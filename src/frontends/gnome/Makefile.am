include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libgnome.la

INCLUDES = $(FRONTEND_INCLUDES) -I$(top_srcdir)/src/ \
	-I$(top_srcdir)/src/frontends/ \
	-I$(top_srcdir)/src/frontends/xforms \
	-I$(top_srcdir)/src/frontends/controllers \
	$(BOOST_INCLUDES)

# These are the XForms GUI classes we still need because they
# haven't been implemented in Gnome.
# alphabetised
xforms_objects = \
	../xforms/Alert_pimpl.lo \
	../xforms/bmtable.lo \
	../xforms/checkedwidgets.lo \
	../xforms/combox.lo \
	../xforms/Color.lo \
	../xforms/ColorHandler.lo \
	../xforms/DropDown.lo \
	../xforms/fdesign_base.lo \
	../xforms/FontInfo.lo \
	../xforms/FormAboutlyx.lo \
	../xforms/FormBase.lo \
	../xforms/FormDialogView.lo \
	../xforms/FormBibitem.lo \
	../xforms/FormBibtex.lo \
	../xforms/FormBrowser.lo \
	../xforms/FormCharacter.lo \
	../xforms/FormCitation.lo \
	../xforms/FormDocument.lo \
	../xforms/FormExternal.lo \
	../xforms/FormForks.lo \
	../xforms/FormGraphics.lo \
	../xforms/FormInclude.lo \
	../xforms/FormIndex.lo \
	../xforms/FormMathsBitmap.lo \
	../xforms/FormMathsDelim.lo \
	../xforms/FormMathsMatrix.lo \
	../xforms/FormMathsPanel.lo \
	../xforms/FormMathsSpace.lo \
	../xforms/FormMathsStyle.lo \
	../xforms/FormParagraph.lo \
	../xforms/FormPreferences.lo \
	../xforms/FormPrint.lo \
	../xforms/FormRef.lo \
	../xforms/forms_gettext.lo \
	../xforms/FormSearch.lo \
	../xforms/FormSendto.lo \
	../xforms/FormShowFile.lo \
	../xforms/FormSpellchecker.lo \
	../xforms/FormTabular.lo \
	../xforms/FormTexinfo.lo \
	../xforms/FormThesaurus.lo \
	../xforms/FormToc.lo \
	../xforms/FormVCLog.lo \
	../xforms/FormWrap.lo \
	../xforms/input_validators.lo \
	../xforms/LyXKeySymFactory.lo \
	../xforms/LyXScreenFactory.lo \
	../xforms/Menubar_pimpl.lo \
	../xforms/RadioButtonGroup.lo \
	../xforms/Toolbar_pimpl.lo \
	../xforms/Tooltips.lo \
	../xforms/WorkAreaFactory.lo \
	../xforms/xfont_loader.lo \
	../xforms/xfont_metrics.lo \
	../xforms/xforms_helpers.lo \
	../xforms/xforms_resize.lo \
	../xforms/xformsBC.lo \
	../xforms/xformsImage.lo \
	../xforms/XFormsView.lo \
	../xforms/XLyXKeySym.lo \
	../xforms/XMiniBuffer.lo \
	../xforms/XPainter.lo \
	../xforms/xscreen.lo \
	../xforms/XWorkArea.lo \
	../xforms/forms/form_aboutlyx.lo \
	../xforms/forms/form_bibitem.lo \
	../xforms/forms/form_bibtex.lo \
	../xforms/forms/form_browser.lo \
	../xforms/forms/form_character.lo \
	../xforms/forms/form_citation.lo \
	../xforms/forms/form_document.lo \
	../xforms/forms/form_error.lo \
	../xforms/forms/form_ert.lo \
	../xforms/forms/form_external.lo \
	../xforms/forms/form_filedialog.lo \
	../xforms/forms/form_float.lo \
	../xforms/forms/form_forks.lo \
	../xforms/forms/form_graphics.lo \
	../xforms/forms/form_include.lo \
	../xforms/forms/form_index.lo \
	../xforms/forms/form_maths_delim.lo \
	../xforms/forms/form_maths_matrix.lo \
	../xforms/forms/form_maths_panel.lo \
	../xforms/forms/form_maths_space.lo \
	../xforms/forms/form_maths_style.lo \
	../xforms/forms/form_paragraph.lo \
	../xforms/forms/form_preamble.lo \
	../xforms/forms/form_preferences.lo \
	../xforms/forms/form_print.lo \
	../xforms/forms/form_ref.lo \
	../xforms/forms/form_search.lo \
	../xforms/forms/form_sendto.lo \
	../xforms/forms/form_spellchecker.lo \
	../xforms/forms/form_tabular_create.lo \
	../xforms/forms/form_tabular.lo \
	../xforms/forms/form_texinfo.lo \
	../xforms/forms/form_thesaurus.lo \
	../xforms/forms/form_toc.lo \
	../xforms/forms/form_url.lo \
	../xforms/forms/form_wrap.lo


xforms.lo: $(xforms_objects)
	$(CXXLINK) $(xforms_objects)

###
# KOZ 20020315 Large commented out removed.
###

libgnome_la_SOURCES = \
	Dialogs.C \
	Dialogs2.C \
	Dialogs_impl.h \
	FileDialog.C \
	GAbout.C \
	GAbout.h \
	GChanges.C \
	GChanges.h \
	GError.C \
	GError.h \
	GERT.C \
	GERT.h \
	GFloat.C \
	GFloat.h \
	GLog.C \
	GLog.h \
	GPreamble.C \
	GPreamble.h \
	GTabularCreate.C \
	GTabularCreate.h \
	GUrl.C \
	GUrl.h \
	gnome_helpers.C \
	gnome_helpers.h \
	GnomeBase.C \
	GnomeBase.h \
	GView.C \
	GView.h \
	gnomeBC.C \
	gnomeBC.h \
	lyx_gui.C \
	gnomeTimeout.C \
	gnomeTimeout.h \
	pixbutton.h

libgnome.la: xforms.lo $(libgnome_la_OBJECTS) $(libgnome_la_DEPENDENCIES)
