include $(top_srcdir)/config/common.am
include $(srcdir)/Makefile.dialogs

SUBDIRS = ui moc

EXTRA_DIST = Makefile.dialogs

noinst_LTLIBRARIES = libqt2.la

libqt2_la_LDFLAGS = $(QT_LDFLAGS)
libqt2_la_LIBADD = $(QT_LIB) ui/*.lo moc/*.lo ui/moc/*.lo

AM_CPPFLAGS = \
	-DQT_CLEAN_NAMESPACE \
	-DQT_GENUINE_STR \
	$(PCH_FLAGS) \
	-I$(top_srcdir)/src/ \
	-I$(top_srcdir)/src/frontends/ \
	-I$(top_srcdir)/images \
	$(QT_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

libqt2_la_SOURCES = \
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
	WorkAreaFactory.C \
	checkedwidgets.C checkedwidgets.h \
	lyx_gui.C \
	lcolorcache.h lcolorcache.C \
	panelstack.h panelstack.C \
	qcoloritem.h qcoloritem.C \
	qfontexample.h qfontexample.C \
	qfont_loader.h qfont_loader.C \
	qfont_metrics.C \
	qlkey.h \
	qscreen.h qscreen.C \
	qt_helpers.h qt_helpers.C \
	$(MOCFILES)
