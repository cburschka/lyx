include $(top_srcdir)/config/common.am
include $(srcdir)/Makefile.dialogs

SUBDIRS = ui

EXTRA_DIST = pch.h Makefile.dialogs

libqt3_la_DEPENDENCIES = $(MOCEDFILES)

MOCEDFILES = $(MOCFILES:.C=_moc.cpp)

%_moc.cpp: %.h
	$(MOC) -o $@ $<

BUILT_SOURCES = $(PCH_FILE) $(MOCEDFILES)

noinst_LTLIBRARIES = libqt3.la

libqt3_la_LDFLAGS = $(QT_LDFLAGS)
libqt3_la_LIBADD = $(QT_LIB) ui/*.lo ui/moc/*.lo

AM_CPPFLAGS += \
	$(QT_CPPFLAGS) \
	$(PCH_FLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/images \
	$(QT_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

libqt3_la_SOURCES = \
	QDialogView.C \
	QDialogView.h \
	Alert_pimpl.C \
	Dialogs.C \
	FileDialog.C \
	GuiApplication.C GuiApplication.h \
	GuiClipboard.C GuiClipboard.h \
	GuiFontMetrics.C GuiFontMetrics.h \
	GuiImplementation.h \
	GuiSelection.C GuiSelection.h \
	GuiWorkArea.h \
	LyXKeySymFactory.C \
	QLMenubar.C QLMenubar.h \
	qtTimeout.C qtTimeout.h \
	QAbout.C QAbout.h \
	QBibitem.C QBibitem.h \
	QBibtex.C QBibtex.h \
	QBox.C QBox.h \
	QBranch.C QBranch.h \
	QChanges.C QChanges.h \
	QCharacter.C QCharacter.h \
	QCitation.C QCitation.h \
	QDocument.C QDocument.h \
	QErrorList.C QErrorList.h \
	QERT.C QERT.h \
	QExternal.C QExternal.h \
	QFloat.C QFloat.h \
	QGraphics.C QGraphics.h \
	QInclude.C QInclude.h \
	QIndex.C QIndex.h \
	QLImage.C QLImage.h \
	QLog.C QLog.h \
	QViewSource.C QViewSource.h \
	QLPainter.C QLPainter.h \
	QLyXKeySym.C QLyXKeySym.h \
	QMath.C QMath.h \
	QNote.C QNote.h \
	QParagraph.C QParagraph.h \
	QPrefs.C QPrefs.h \
	QPrint.C QPrint.h \
	QRef.C QRef.h \
	QSearch.C QSearch.h \
	QSendto.C QSendto.h \
	QShowFile.C QShowFile.h \
	QSpellchecker.C QSpellchecker.h \
	QTabular.C QTabular.h \
	QTabularCreate.C QTabularCreate.h \
	QTexinfo.C QTexinfo.h \
	QThesaurus.C QThesaurus.h \
	QToc.C QToc.h \
	QURL.C QURL.h \
	QVSpace.C QVSpace.h \
	QWorkArea.h QWorkArea.C \
	QWrap.C QWrap.h \
	Qt2BC.C	Qt2BC.h	\
	QtLyXView.h \
	checkedwidgets.C checkedwidgets.h \
	lcolorcache.h lcolorcache.C \
	panelstack.h panelstack.C \
	qcoloritem.h qcoloritem.C \
	qfontexample.h qfontexample.C \
	qfont_loader.h qfont_loader.C \
	qlkey.h \
	qscreen.h qscreen.C \
	qt_helpers.h qt_helpers.C \
	$(MOCFILES)
