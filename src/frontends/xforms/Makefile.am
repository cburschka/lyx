include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libxforms.la

INCLUDES = -I$(top_srcdir)/images -I$(top_srcdir)/src/ \
	-I$(top_srcdir)/src/frontends/ \
	-I$(top_srcdir)/src/frontends/controllers \
	$(SIGC_CFLAGS) $(BOOST_INCLUDES)

SUBDIRS = forms

EXTRA_DIST = xformsImage.C xformsImage.h

if USE_BASIC_IMAGE_LOADER
else
XFORMSIMAGE = xformsImage.C xformsImage.h
endif

# Alphabetical order please.  It makes it easier to figure out what's missing.
libxforms_la_SOURCES = \
	forms_fwd.h \
	forms_gettext.C \
	forms_gettext.h \
	bmtable.c \
	bmtable.h \
	combox.C \
	combox.h \
	input_validators.C \
	input_validators.h \
	lyx_gui.C \
	lyxlookup.C \
	lyxlookup.h \
	xfont_loader.C \
	xfont_loader.h \
	xfont_metrics.C \
	xfont_metrics.h \
	xforms_helpers.C \
	xforms_helpers.h \
	xforms_resize.C \
	xforms_resize.h \
	$(XFORMSIMAGE) xformsBC.C \
	xformsBC.h \
	xscreen.C \
	xscreen.h \
	Alert_pimpl.C \
	Color.C \
	Color.h \
	ColorHandler.C \
	ColorHandler.h \
	Dialogs.C \
	DropDown.h \
	DropDown.C \
	FeedbackController.C \
	FeedbackController.h \
	FileDialog.C \
	FontInfo.C \
	FontInfo.h \
	FormFiledialog.h \
	FormFiledialog.C \
	FormAboutlyx.C \
	FormAboutlyx.h \
	FormBase.C \
	FormBase.h \
	FormBaseDeprecated.C \
	FormBaseDeprecated.h \
	FormBibitem.C \
	FormBibitem.h \
	FormBibtex.C \
	FormBibtex.h \
	FormBrowser.C \
	FormBrowser.h \
	FormCharacter.C \
	FormCharacter.h \
	FormCitation.C \
	FormCitation.h \
	FormDocument.C \
	FormDocument.h \
	FormError.C \
	FormError.h \
	FormERT.C \
	FormERT.h \
	FormExternal.C \
	FormExternal.h \
	FormFloat.C \
	FormFloat.h \
	FormForks.C \
	FormForks.h \
	FormGraphics.C \
	FormGraphics.h \
	FormInclude.C \
	FormInclude.h \
	FormIndex.C \
	FormIndex.h \
	FormInset.C \
	FormInset.h \
	FormLog.C \
	FormLog.h \
	FormMathsBitmap.C \
	FormMathsBitmap.h \
	FormMathsDeco.C \
	FormMathsDeco.h \
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
	FormMinipage.C \
	FormMinipage.h \
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
	FormThesaurus.C \
	FormThesaurus.h \
	FormToc.C \
	FormToc.h \
	FormUrl.C \
	FormUrl.h \
	FormVCLog.C \
	FormVCLog.h \
	LyXKeySymFactory.C \
	LyXScreenFactory.C \
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
