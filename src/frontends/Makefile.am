include $(top_srcdir)/config/common.am

SUBDIRS = controllers @FRONTEND@

DIST_SUBDIRS = controllers xforms qt2 gnome

noinst_LTLIBRARIES = libfrontends.la

LIBS += @FRONTEND_LDFLAGS@ @FRONTEND_LIBS@

INCLUDES = $(FRONTEND_INCLUDES) -I$(srcdir)/..  $(SIGC_CFLAGS) $(BOOST_INCLUDES)
libfrontends_la_LIBADD = @FRONTEND_GUILIB@ controllers/*.lo

libfrontends_la_DEPENDENCIES = $(libfrontends_la_LIBADD)

libfrontends_la_SOURCES = \
	Alert.C \
	Alert.h \
	Alert_pimpl.h \
	Dialogs.C \
	Dialogs.h \
	FileDialog.h \
	Liason.C \
	Liason.h \
	LyXKeySym.h \
	LyXKeySymFactory.h \
	LyXScreenFactory.h \
	LyXView.C \
	LyXView.h \
	Menubar.C \
	Menubar.h \
	Painter.C \
	Painter.h \
	Timeout.C \
	Timeout.h \
	Toolbar.C \
	Toolbar.h \
	WorkArea.C \
	WorkArea.h \
	WorkAreaFactory.h \
	font_metrics.h \
	guiapi.h \
	guiapi.C \
	key_state.h \
	lyx_gui.h \
	mouse_state.h \
	screen.C \
	screen.h
