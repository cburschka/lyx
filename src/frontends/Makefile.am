include $(top_srcdir)/config/common.am

SUBDIRS = controllers @FRONTEND@

noinst_LTLIBRARIES = libfrontends.la

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

if USE_LD_R
libfrontends.la: libfrontends.o

libfrontends.o: $(libfrontends_la_OBJECTS) $(libfrontends_la_DEPENDENCIES)
	$(CXXLINK) $(libfrontends_la_OBJECTS) $(libfrontends_la_LIBADD)
endif

EXTRA_DIST = xforms qt2 gnome

dist-hook:
	@cd $(distdir) ; \
	rm -rf `find qt2 -name \*CVS\*`; \
	rm -rf `find gnome -name \*CVS\*`
