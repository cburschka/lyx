include $(top_srcdir)/config/common.am

SUBDIRS = gimages glade

EXTRA_DIST = pch.h

BUILT_SOURCES = $(PCH_FILE)

noinst_LTLIBRARIES = libgtk.la

AM_CPPFLAGS += \
	$(PCH_FLAGS) \
	-I$(top_srcdir)/images \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/controllers \
	$(BOOST_INCLUDES) @GTK_FRONTEND_CFLAGS@

libgtk_la_LIBADD = @GTK_FRONTEND_LIBS@

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
	GBibItem.C \
	GBibItem.h \
	GBibtex.C \
	GBibtex.h \
	GBox.C \
	GBox.h \
	GBranch.C \
	GBranch.h \
	GChanges.C \
	GChanges.h \
	GCharacter.C \
	GCharacter.h \
	GCitation.C \
	GCitation.h \
	GDocument.C \
	GDocument.h \
	GErrorList.C \
	GErrorList.h \
	GERT.C \
	GERT.h \
	GFloat.C \
	GFloat.h \
	GInclude.C \
	GInclude.h \
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
	GPreferences.C \
	GPreferences.h \
	GPrint.C \
	GPrint.h \
	GRef.C \
	GRef.h \
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
	GTabular.C \
	GTabular.h \
	GTexinfo.C \
	GTexinfo.h \
	GText.C \
	GText.h \
	GThesaurus.C \
	GThesaurus.h \
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
	GWrap.C \
	GWrap.h \
	GXpmBtnTbl.C \
	GXpmBtnTbl.h \
	LyXGdkImage.C \
	LyXGdkImage.h \
	GtkLengthEntry.C \
	GtkLengthEntry.h \
	GtkmmX.h \
	IdSc.C \
	IdSc.h \
	LyXGdkImage.C \
	LyXGdkImage.h \
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

