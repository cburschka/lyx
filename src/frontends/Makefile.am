AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
SUBDIRS = @FRONTEND@
ETAGS_ARGS = --lang=c++
INCLUDES = ${FRONTEND_INCLUDES} -I${srcdir}/.. -I${srcdir}/@FRONTEND@ ${SIGC_CFLAGS}
LIBS =
noinst_LTLIBRARIES = libfrontends.la
libfrontends_la_SOURCES=\
	Dialogs.h \
	DialogBase.h \
	Liason.C \
	Liason.h \
	Menubar.C \
	Menubar.h \
	Toolbar.C \
	Toolbar.h

