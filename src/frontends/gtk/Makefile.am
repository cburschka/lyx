include $(top_srcdir)/config/common.am

SUBDIRS = gimages glade

noinst_LTLIBRARIES = libgtk.la

INCLUDES = -I$(top_srcdir)/images -I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/controllers \
	-I../xforms \
	-I$(top_srcdir)/src/frontends/xforms \
	$(BOOST_INCLUDES) @GTK_FRONTEND_CFLAGS@

libgtk_la_LIBADD = xforms.lo @GTK_FRONTEND_LIBS@ @XFORMS_LIBS@

# Alphabetical order please.  It makes it easier to figure out what's missing.
libgtk_la_SOURCES = \
	lyx_gui.C \
	GtkmmX.h \
	xftFontLoader.C \
	xftFontLoader.h \
	codeConvert.h \
	xftFontMetrics.C \
	GScreen.C \
	GScreen.h \
	LyXScreenFactory.C \
	GMenubar.C \
	GMenubar.h \
	GTimeout.C \
	GTimeout.h \
	GToolbar.C \
	GToolbar.h \
	WorkAreaFactory.C \
	GMiniBuffer.C \
	GMiniBuffer.h \
	GPainter.C \
	GPainter.h \
	GWorkArea.h \
	GWorkArea.C \
	GLyXKeySym.h \
	GLyXKeySym.C \
	LyXKeySymFactory.C \
	Alert_pimpl.C \
	GView.h \
	GView.C \
	IdSc.h \
	IdSc.C \
	Dialogs.C \
	GAboutlyx.h \
	GAboutlyx.C \
	GViewBase.h \
	GViewBase.C \
	GBC.h \
	GBC.C \
	FileDialogPrivate.h \
	FileDialogPrivate.C \
	FileDialog.C \
	GText.h \
	GText.C \
	GUrl.h \
	GUrl.C \
	GTableCreate.h \
	GTableCreate.C \
	GMathPanel.h \
	GMathPanel.C \
	GXpmBtnTbl.h \
	GXpmBtnTbl.C \
	GMathDelim.h \
	GMathDelim.C

#	GPrint.h
#	GPrint.C

xforms_objects = \
	../xforms/bmtable.lo \
	../xforms/checkedwidgets.lo \
	../xforms/ColorHandler.lo \
	../xforms/Color.lo \
	../xforms/combox.lo \
	../xforms/fdesign_base.lo \
	../xforms/FormBibitem.lo \
	../xforms/FormBibtex.lo \
	../xforms/FormBox.lo \
	../xforms/FormBranch.lo \
	../xforms/FormBrowser.lo \
	../xforms/FormChanges.lo \
	../xforms/FormCharacter.lo \
	../xforms/FormCitation.lo \
	../xforms/FormColorpicker.lo \
	../xforms/FormDialogView.lo \
	../xforms/FormDocument.lo \
	../xforms/FormErrorList.lo \
	../xforms/FormERT.lo \
	../xforms/FormExternal.lo \
	../xforms/FormFloat.lo \
	../xforms/FormGraphics.lo \
	../xforms/FormInclude.lo \
	../xforms/FormLog.lo \
	../xforms/FormMathsBitmap.lo \
	../xforms/FormMathsDelim.lo \
	../xforms/FormMathsMatrix.lo \
	../xforms/FormMathsSpace.lo \
	../xforms/FormMathsStyle.lo \
	../xforms/FormNote.lo \
	../xforms/FormParagraph.lo \
	../xforms/FormPreamble.lo \
	../xforms/FormPreferences.lo \
	../xforms/FormPrint.lo \
	../xforms/FormRef.lo \
	../xforms/FormSearch.lo \
	../xforms/FormSendto.lo \
	../xforms/forms_gettext.lo \
	../xforms/FormShowFile.lo \
	../xforms/FormSpellchecker.lo \
	../xforms/FormTabular.lo \
	../xforms/FormTexinfo.lo \
	../xforms/FormText.lo \
	../xforms/FormThesaurus.lo \
	../xforms/FormToc.lo \
	../xforms/FormVSpace.lo \
	../xforms/FormWrap.lo \
	../xforms/freebrowser.lo \
	../xforms/input_validators.lo \
	../xforms/RadioButtonGroup.lo \
	../xforms/Tooltips.lo \
	../xforms/xformsBC.lo \
	../xforms/xforms_helpers.lo \
	../xforms/xformsImage.lo \
	../xforms/xforms_resize.lo

#	../xforms/Dialogs.lo
#	../xforms/FormFiledialog.lo
#	../xforms/FileDialog.lo
#	../xforms/FormAboutlyx.lo
#	../xforms/FormUrl.lo
#	../xforms/FormTabularCreate.lo
#	../xforms/FormMathsPanel.lo

xforms.lo: $(xforms_objects) ../xforms/forms/*.lo
	$(CXXLINK) $(xforms_objects) ../xforms/forms/*.lo
