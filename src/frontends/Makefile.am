include $(top_srcdir)/config/common.am

SUBDIRS = controllers @FRONTEND@

noinst_LTLIBRARIES = libfrontends.la

LIBS += @FRONTEND_LDFLAGS@ @FRONTEND_LIBS@

INCLUDES = $(FRONTEND_INCLUDES) -I$(srcdir)/..  $(SIGC_CFLAGS) $(BOOST_INCLUDES)

EXTRA_DIST = xforms qt2 gnome

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
	Timeout.C \
	Timeout.h \
	Toolbar.C \
	Toolbar.h

dist-hook:
	@cd $(distdir) ; \
	rm -rf `find qt2 -name \*CVS\*`; \
	rm -rf `find gnome -name \*CVS\*`

distclean-local:
	for dir in $(EXTRA_DIST) ; do \
	if test -f $$dir/Makefile ; then \
		make -C $$dir distclean ; \
	fi ; \
	done
