include $(top_srcdir)/config/common.am
include $(srcdir)/Makefile.dialogs

SUBDIRS = ui moc

EXTRA_DIST = Makefile.dialogs

noinst_LTLIBRARIES = libqt2.la

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/images \
	$(FRONTEND_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

AM_CXXFLAGS = $(QT_CPPFLAGS)

libqt2_la_SOURCES = \
	Alert_pimpl.C \
	bulletstrings.C bulletstrings.h \
	Dialogs.C \
	Dialogs2.C \
	Dialogs_impl.h \
	FileDialog.C \
	LyXKeySymFactory.C \
	LyXScreenFactory.C \
	Menubar_pimpl.C Menubar_pimpl.h \
	Timeout_pimpl.C Timeout_pimpl.h \
	QAbout.C QAbout.h \
	QBibitem.C QBibitem.h \
	QBibtex.C QBibtex.h \
	QCharacter.C QCharacter.h \
	QCitation.C QCitation.h \
	QDocument.C QDocument.h \
	QError.C QError.h \
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
	QMinipage.C QMinipage.h \
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
	QVCLog.C QVCLog.h \
	QWorkArea.h QWorkArea.C \
	QWrap.C QWrap.h \
	Qt2BC.C	Qt2BC.h	\
	QtLyXView.h \
	WorkAreaFactory.C \
	lyx_gui.C \
	qcoloritem.h qcoloritem.C \
	qfontexample.h qfontexample.C \
	qfont_loader.h qfont_loader.C \
	qfont_metrics.C \
	qlkey.h \
	qscreen.h qscreen.C \
	qt_helpers.h qt_helpers.C \
	$(MOCFILES)

libqt2.la: $(libqt2_la_OBJECTS) $(libqt2_la_DEPENDENCIES)
