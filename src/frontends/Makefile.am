AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
# xforms has to be fixed here as we still need object files in this directory
# for the compilation of other frontends, so until that we have to let this
# here.
SUBDIRS = support controllers xforms @FRONTEND@
ETAGS_ARGS = --lang=c++
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = ${FRONTEND_INCLUDES} -I${srcdir}/.. -I${srcdir}/xforms ${SIGC_CFLAGS} $(BOOST_INCLUDES)
LIBS =
noinst_LTLIBRARIES = libfrontends.o

libfrontends_o_LIBADD= `cat libxforms.objects` \
	`cat libcontrollers.objects` \
	support/libfrontendsupport.o

libfrontends_o_DEPENDENCIES = \
	libxforms.objects \
	libcontrollers.objects \
	support/libfrontendsupport.o

libfrontends_o_SOURCES=\
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

EXTRA_DIST = qt2 gnome

dist-hook:
	@cd $(distdir) ; \
	rm -rf `find qt2 -name \*CVS\*`; \
	rm -rf `find gnome -name \*CVS\*`
