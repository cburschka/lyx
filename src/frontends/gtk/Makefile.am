include $(top_srcdir)/config/common.am

SUBDIRS = gimages glade

noinst_LTLIBRARIES = libgtk.la

AM_CPPFLAGS = \
	-I$(top_srcdir)/images \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/controllers \
	-I../xforms \
	-I$(top_srcdir)/src/frontends/xforms \
	$(BOOST_INCLUDES) @GTK_FRONTEND_CFLAGS@

libgtk_la_LIBADD = $(xforms_objects) ../xforms/forms/*.lo @GTK_FRONTEND_LIBS@ @XFORMS_LIBS@

# Alphabetical order please.  It makes it easier to figure out what's missing.
libgtk_la_SOURCES = \
	Alert_pimpl.C \
	Dialogs.C \
	FileDialog.C \
	FileDialogPrivate.C \
	FileDialogPrivate.h \
	GAboutlyx.C \
	GAboutlyx.h \
	GBC.C \
	GBC.h \
	GBox.C \
	GBox.h \
	GChanges.C \
	GChanges.h \
	GCharacter.C \
	GCharacter.h \
	GErrorList.C \
	GErrorList.h \
	GERT.C \
	GERT.h \
	GFloat.C \
	GFloat.h \
	GGraphics.C \
	GGraphics.h \
	GLog.C \
	GLog.h \
	GLyXKeySym.C \
	GLyXKeySym.h \
	GMathDelim.C \
	GMathDelim.h \
	GMathPanel.C \
	GMathPanel.h \
	GMathsMatrix.C \
	GMathsMatrix.h \
	GMenubar.C \
	GMenubar.h \
	GMiniBuffer.C \
	GMiniBuffer.h \
	GNote.C \
	GNote.h \
	GPainter.C \
	GPainter.h \
	GParagraph.C \
	GParagraph.h \
	GPrint.C \
	GPrint.h \
	GScreen.C \
	GScreen.h \
	GSearch.C \
	GSearch.h \
	GSendto.C \
	GSendto.h \
	GShowFile.C \
	GShowFile.h \
	GSpellchecker.C \
	GSpellchecker.h \
	GTableCreate.C \
	GTableCreate.h \
	GTexinfo.C \
	GTexinfo.h \
	GText.C \
	GText.h \
	GTimeout.C \
	GTimeout.h \
	GToc.C \
	GToc.h \
	GToolbar.C \
	GToolbar.h \
	GUrl.C \
	GUrl.h \
	GView.C \
	GView.h \
	GViewBase.C \
	GViewBase.h \
	GVSpace.C \
	GVSpace.h \
	GWorkArea.C \
	GWorkArea.h \
	GXpmBtnTbl.C \
	GXpmBtnTbl.h \
	GtkmmX.h \
	IdSc.C \
	IdSc.h \
	LyXKeySymFactory.C \
	LyXScreenFactory.C \
	WorkAreaFactory.C \
	codeConvert.h \
	ghelpers.C \
	ghelpers.h \
	io_callback.C \
	io_callback.h \
	lyx_gui.C \
	xftFontLoader.C \
	xftFontLoader.h \
	xftFontMetrics.C


xforms_objects = \
	../xforms/bmtable.lo \
	../xforms/checkedwidgets.lo \
	../xforms/ColorHandler.lo \
	../xforms/Color.lo \
	../xforms/combox.lo \
	../xforms/fdesign_base.lo \
	../xforms/FormBibitem.lo \
	../xforms/FormBibtex.lo \
	../xforms/FormBranch.lo \
	../xforms/FormBrowser.lo \
	../xforms/FormCitation.lo \
	../xforms/FormColorpicker.lo \
	../xforms/FormDialogView.lo \
	../xforms/FormDocument.lo \
	../xforms/FormExternal.lo \
	../xforms/FormInclude.lo \
	../xforms/FormMathsBitmap.lo \
	../xforms/FormMathsDelim.lo \
	../xforms/FormMathsSpace.lo \
	../xforms/FormMathsStyle.lo \
	../xforms/FormPreamble.lo \
	../xforms/FormPreferences.lo \
	../xforms/FormRef.lo \
	../xforms/forms_gettext.lo \
	../xforms/FormTabular.lo \
	../xforms/FormText.lo \
	../xforms/FormThesaurus.lo \
	../xforms/FormWrap.lo \
	../xforms/freebrowser.lo \
	../xforms/input_validators.lo \
	../xforms/RadioButtonGroup.lo \
	../xforms/Tooltips.lo \
	../xforms/xformsBC.lo \
	../xforms/xforms_helpers.lo \
	../xforms/xformsImage.lo \
	../xforms/xforms_resize.lo
