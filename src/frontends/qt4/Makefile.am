include $(top_srcdir)/config/common.am
include $(srcdir)/Makefile.dialogs

SUBDIRS = ui

EXTRA_DIST = pch.h Makefile.dialogs

DISTCLEANFILES += *_moc.cpp

libqt4_la_DEPENDENCIES = $(MOCEDFILES)

MOCEDFILES = $(MOCFILES:.C=_moc.cpp)

%_moc.cpp: %.h
	$(MOC4) -o $@ $<

BUILT_SOURCES = $(PCH_FILE) $(MOCEDFILES)

noinst_LTLIBRARIES = libqt4.la

libqt4_la_LDFLAGS = $(QT4_LDFLAGS)
libqt4_la_LIBADD = $(QT4_LIB) 
AM_CPPFLAGS += \
	$(QT4_CPPFLAGS) \
	$(PCH_FLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/images \
	$(QT4_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

libqt4_la_SOURCES = \
	QDialogView.C \
	QDialogView.h \
	Alert_pimpl.C \
	Dialogs.C \
	FileDialog.C \
	LyXKeySymFactory.C \
	LyXScreenFactory.C \
	QLMenubar.C QLMenubar.h \
	qtTimeout.C qtTimeout.h \
	QAbout.C QAbout.h \
	QBibitem.C QBibitem.h \
	QBibtex.C QBibtex.h \
	QBox.C QBox.h \
	QBranch.C QBranch.h \
	QBranches.C QBranches.h \
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
	QLAction.C QLAction.h \
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
	QWorkArea.C QWorkArea.h \
	QWrap.C QWrap.h \
	Qt2BC.C	Qt2BC.h	\
	QtLyXView.h \
	WorkAreaFactory.C \
	checkedwidgets.C checkedwidgets.h \
	lyx_gui.C \
	lcolorcache.h lcolorcache.C \
	panelstack.h panelstack.C \
	qfontexample.h qfontexample.C \
	qfont_loader.h qfont_loader.C \
	qfont_metrics.C \
	qlkey.h \
	qt_helpers.h qt_helpers.C \
	$(MOCFILES)
