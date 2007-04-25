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
	Alert_pimpl.C \
	ColorCache.h ColorCache.C \
	DockView.h \
	Dialogs.C \
	FileDialog.C \
	GuiClipboard.h GuiClipboard.C \
	GuiFontLoader.h GuiFontLoader.C \
	GuiFontMetrics.h GuiFontMetrics.C \
	GuiSelection.h GuiSelection.C \
	LyXKeySymFactory.C \
	QLMenubar.C QLMenubar.h \
	QBox.C QBox.h \
	QBranches.C QBranches.h \
	QCharacter.C QCharacter.h \
	QCitation.C QCitation.h \
	QDialogView.C QDialogView.h \
	QFloat.C QFloat.h \
	QGraphics.C QGraphics.h \
	QInclude.C QInclude.h \
	QLImage.C QLImage.h \
	QViewSource.C QViewSource.h \
	QLPainter.C QLPainter.h \
	QLyXKeySym.C QLyXKeySym.h \
	QNomencl.C QNomencl.h \
	QNote.C QNote.h \
	QPrint.C QPrint.h \
	QRef.C QRef.h \
	QSendto.C QSendto.h \
	Qt2BC.C	Qt2BC.h	\
	checkedwidgets.C checkedwidgets.h \
	panelstack.h panelstack.C \
	qfontexample.h qfontexample.C \
	qlkey.h \
	qt_helpers.h qt_helpers.C \
	qtTimeout.C qtTimeout.h \
	UrlView.C UrlView.h \
	$(MOCFILES)
