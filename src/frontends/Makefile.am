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
	DialogBase.h \
	FileDialog.h \
	GUIRunTime.h \
	Liason.C \
	Liason.h \
	Menubar.C \
	Menubar.h \
	Painter.C \
	Painter.h \
	Timeout.C \
	Timeout.h \
	Toolbar.C \
	Toolbar.h \
	LyXView.C \
	LyXView.h \
	WorkArea.C \
	WorkArea.h \
	font_metrics.h \
	screen.C \
	screen.h
