AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libfrontendsupport.la
LIBS =
ETAGS_ARGS = --lang=c++
INCLUDES = -I${srcdir}/../../ $(SIGC_CFLAGS)

libfrontendsupport_la_SOURCES = \
	LyXImage.h \
	LyXImage.C

libfrontendsupport.la: libfrontendsupport.o

libfrontendsupport.o: $(libfrontendsupport_la_OBJECTS) $(libfrontendsupport_la_DEPENDENCIES)
	$(CXXLINK) $(libfrontendsupport_la_OBJECTS)

EXTRA_DIST = \
	LyXImage_X.h \
	LyXImage_X.C
