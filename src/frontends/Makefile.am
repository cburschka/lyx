include $(top_srcdir)/config/common.am

SUBDIRS = controllers $(FRONTENDS_SUBDIRS)

DIST_SUBDIRS = controllers qt4

EXTRA_DIST = pch.h

BUILT_SOURCES = $(PCH_FILE)

noinst_LTLIBRARIES = libfrontends.la

AM_CPPFLAGS += $(PCH_FLAGS) -I$(srcdir)/..  $(BOOST_INCLUDES)

libfrontends_la_SOURCES = \
	Alert.cpp \
	Alert.h \
	Alert_pimpl.h \
	Application.cpp \
	Application.h \
	NoGuiFontLoader.h \
	NoGuiFontMetrics.h \
	Dialogs.cpp \
	Dialogs.h \
	FileDialog.h \
	FontLoader.h \
	FontMetrics.h \
	KeySymbol.h \
	KeySymbolFactory.h \
	LyXView.cpp \
	LyXView.h \
	Menubar.h \
	Painter.cpp \
	Painter.h \
	Timeout.cpp \
	Timeout.h \
	Toolbars.cpp \
	Toolbars.h \
	Clipboard.h \
	Gui.h \
	Selection.h \
	WorkArea.cpp \
	WorkArea.h \
	key_state.h \
	mouse_state.h
