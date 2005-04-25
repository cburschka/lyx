include $(top_srcdir)/config/common.am

SUBDIRS = forms
CLEANFILES += stamp-xpm stamp-forms
DISTCLEANFILES += lyx_forms.h lyx_forms.h-tmp lyx_xpm.h lyx_xpm.h-tmp
BUILT_SOURCES = lyx_forms.h lyx_xpm.h

EXTRA_DIST = lyx_forms.h.in lyx_xpm.h.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/images \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/controllers \
	$(BOOST_INCLUDES)

# This cannot be put in AM_CPPFLAGS since that is used both for C and C++
# And the precompiled header is setup only for C++
AM_CXXFLAGS = $(PCH_FLAGS)

noinst_LTLIBRARIES = libxforms.la

libxforms_la_LIBADD = @XFORMS_LIBS@ forms/*.lo

# Alphabetical order please.  It makes it easier to figure out what's missing.
libxforms_la_SOURCES = \
	forms_fwd.h \
	forms_gettext.C \
	forms_gettext.h \
	bmtable.c \
	bmtable.h \
	checkedwidgets.C \
	checkedwidgets.h \
	combox.c \
	combox.h \
	freebrowser.c \
	freebrowser.h \
	fdesign_base.C \
	fdesign_base.h \
	globbing.C \
	globbing.h \
	input_validators.C \
	input_validators.h \
	lyx_gui.C \
	xfont_loader.C \
	xfont_loader.h \
	xfont_metrics.C \
	xfont_metrics.h \
	xforms_helpers.C \
	xforms_helpers.h \
	xforms_resize.C \
	xforms_resize.h \
	xformsBC.C \
	xformsBC.h \
	xformsImage.C \
	xformsImage.h \
	xformsTimeout.C \
	xformsTimeout.h \
	xscreen.C \
	xscreen.h \
	Alert_pimpl.C \
	Color.C \
	Color.h \
	ColorHandler.C \
	ColorHandler.h \
	Dialogs.C \
	FileDialog.C \
	FontInfo.C \
	FontInfo.h \
	FormFiledialog.h \
	FormFiledialog.C \
	FormAboutlyx.C \
	FormAboutlyx.h \
	FormDialogView.C \
	FormDialogView.h \
	FormBibitem.C \
	FormBibitem.h \
	FormBibtex.C \
	FormBibtex.h \
	FormBox.C \
	FormBox.h \
	FormBrowser.C \
	FormBrowser.h \
	FormBranch.C \
	FormBranch.h \
	FormChanges.C \
	FormChanges.h \
	FormCharacter.C \
	FormCharacter.h \
	FormCitation.C \
	FormCitation.h \
	FormColorpicker.C \
	FormColorpicker.h \
	FormDocument.C \
	FormDocument.h \
	FormErrorList.C \
	FormErrorList.h \
	FormERT.C \
	FormERT.h \
	FormExternal.C \
	FormExternal.h \
	FormFloat.C \
	FormFloat.h \
	FormGraphics.C \
	FormGraphics.h \
	FormInclude.C \
	FormInclude.h \
	FormLog.C \
	FormLog.h \
	FormMathsBitmap.C \
	FormMathsBitmap.h \
	FormMathsDelim.C \
	FormMathsDelim.h \
	FormMathsMatrix.C \
	FormMathsMatrix.h \
	FormMathsPanel.C \
	FormMathsPanel.h \
	FormMathsSpace.C \
	FormMathsSpace.h \
	FormMathsStyle.C \
	FormMathsStyle.h \
	FormNote.C \
	FormNote.h \
	FormParagraph.C \
	FormParagraph.h \
	FormPreamble.C \
	FormPreamble.h \
	FormPreferences.C \
	FormPreferences.h \
	FormPrint.C \
	FormPrint.h \
	FormRef.C \
	FormRef.h \
	FormSearch.C \
	FormSearch.h \
	FormSendto.C \
	FormSendto.h \
	FormShowFile.C \
	FormShowFile.h \
	FormSpellchecker.C \
	FormSpellchecker.h \
	FormTabular.C \
	FormTabular.h \
	FormTabularCreate.C \
	FormTabularCreate.h \
	FormTexinfo.C \
	FormTexinfo.h \
	FormText.C \
	FormText.h \
	FormThesaurus.C \
	FormThesaurus.h \
	FormToc.C \
	FormToc.h \
	FormUrl.C \
	FormUrl.h \
	FormVSpace.C \
	FormVSpace.h \
	FormWrap.C \
	FormWrap.h \
	LayoutEngine.C \
	LayoutEngine.h \
	LyXKeySymFactory.C \
	LyXScreenFactory.C \
	XFormsMenubar.C \
	XFormsMenubar.h \
	RadioButtonGroup.C \
	RadioButtonGroup.h \
	XFormsToolbar.C \
	XFormsToolbar.h \
	Tooltips.C \
	Tooltips.h \
	WorkAreaFactory.C \
	XFormsView.C \
	XFormsView.h \
	XLyXKeySym.C \
	XLyXKeySym.h \
	XMiniBuffer.C \
	XMiniBuffer.h \
	XPainter.C \
	XPainter.h \
	XWorkArea.h \
	XWorkArea.C

lyx_forms.h: stamp-forms
	@:

stamp-forms: lyx_forms.h-tmp
	cmp -s $< lyx_forms.h || {\
		rm -f lyx_forms.h ;\
		cp $< lyx_forms.h ;\
	}
	echo timestamp > $@

lyx_xpm.h: stamp-xpm
	@:

stamp-xpm: lyx_xpm.h-tmp
	cmp -s $< lyx_xpm.h || {\
		rm -f lyx_xpm.h ;\
		cp $< lyx_xpm.h ;\
	}
	echo timestamp > $@
