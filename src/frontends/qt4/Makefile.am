include $(top_srcdir)/config/common.am
include $(srcdir)/Makefile.dialogs

SUBDIRS = ui

EXTRA_DIST = pch.h Makefile.dialogs

DISTCLEANFILES += *_moc.cpp

libqt4_la_DEPENDENCIES = $(MOCEDFILES)

MOCEDFILES = $(MOCFILES:.cpp=_moc.cpp)

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
	alert_pimpl.cpp \
	ColorCache.h ColorCache.cpp \
	DockView.h \
	Dialogs.cpp \
	FileDialog.cpp \
	GuiFontLoader.h GuiFontLoader.cpp \
	GuiFontMetrics.h GuiFontMetrics.cpp \
	KeySymbol.cpp \
	QLMenubar.cpp QLMenubar.h \
	QBox.cpp QBox.h \
	QBranches.cpp QBranches.h \
	QCitation.cpp QCitation.h \
	QDialogView.cpp QDialogView.h \
	QFloat.cpp QFloat.h \
	QGraphics.cpp QGraphics.h \
	QListings.cpp QListings.h \
	QLImage.cpp QLImage.h \
	QViewSource.cpp QViewSource.h \
	QLPainter.cpp QLPainter.h \
	QKeySymbol.cpp QKeySymbol.h \
	QPrint.cpp QPrint.h \
	Qt2BC.cpp	Qt2BC.h	\
	CheckedLineEdit.cpp CheckedLineEdit.h \
	PanelStack.h PanelStack.cpp \
	QFontExample.h QFontExample.cpp \
	qlkey.h \
	qt_helpers.h qt_helpers.cpp \
	qtTimeout.cpp qtTimeout.h \
	UrlView.cpp UrlView.h \
	$(MOCFILES)
