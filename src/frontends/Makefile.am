include $(top_srcdir)/config/common.am

DISTCLEANFILES += -r qt2/Makefile qt2/.deps qt2/xforms/Makefile \
	qt2/xforms/.deps qt2/moc/Makefile qt2/moc/.deps qt2/ui/Makefile \
	qt2/ui/.deps qt2/ui/moc/Makefile qt2/ui/moc/.deps \
	gnome/Makefile gnome/.deps

SUBDIRS = controllers @FRONTEND@

noinst_LTLIBRARIES = libfrontends.la

LIBS += @FRONTEND_LDFLAGS@ @FRONTEND_LIBS@

INCLUDES = $(FRONTEND_INCLUDES) -I$(srcdir)/..  $(SIGC_CFLAGS) $(BOOST_INCLUDES)

libfrontends_la_LIBADD= @FRONTEND_GUILIB@ \
	controllers/*.lo

libfrontends_la_DEPENDENCIES = $(libfrontends_la_LIBADD)

libfrontends_la_SOURCES=\
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
	Timeout.C \
	Timeout.h \
	Toolbar.C \
	Toolbar.h

EXTRA_DIST = xforms qt2 gnome

dist-hook:
	@cd $(distdir) ; \
	rm -rf `find qt2 -name \*CVS\*`; \
	rm -rf `find gnome -name \*CVS\*`
