include $(top_srcdir)/config/common.am

SUBDIRS = controllers $(FRONTENDS_SUBDIRS)

DIST_SUBDIRS = controllers xforms qt2 gnome gtk

noinst_LTLIBRARIES = libfrontends.la

AM_CPPFLAGS = -I$(srcdir)/..  $(BOOST_INCLUDES)

AM_CXXFLAGS = $(PCH_FLAGS)

libfrontends_la_SOURCES = \
	Alert.C \
	Alert.h \
	Alert_pimpl.h \
	Dialogs.C \
	Dialogs.h \
	FileDialog.h \
	LyXKeySym.h \
	LyXKeySymFactory.h \
	LyXScreenFactory.h \
	LyXView.C \
	LyXView.h \
	Menubar.h \
	Painter.C \
	Painter.h \
	Timeout.C \
	Timeout.h \
	Toolbars.C \
	Toolbars.h \
	WorkArea.h \
	WorkAreaFactory.h \
	font_metrics.h \
	guiapi.h \
	guiapi.C \
	key_state.h \
	lyx_gui.h \
	mouse_state.h \
	nullpainter.C \
	nullpainter.h \
	screen.C \
	screen.h
