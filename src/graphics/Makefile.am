AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libgraphics.la
LIBS =
ETAGS_ARGS = --lang=c++
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${srcdir}/../ $(SIGC_CFLAGS) $(BOOST_INCLUDES)

EXTRA_DIST = GraphicsImageXPM.C GraphicsImageXPM.h

if USE_BASIC_IMAGE_LOADER
GRAPHICSIMAGEXPM = GraphicsImageXPM.C GraphicsImageXPM.h
endif

libgraphics_la_SOURCES = \
	GraphicsCache.h \
	GraphicsCache.C \
	GraphicsCacheItem.h \
	GraphicsCacheItem.C \
	GraphicsConverter.h \
	GraphicsConverter.C \
	GraphicsImage.h \
	GraphicsImage.C \
	$(GRAPHICSIMAGEXPM) GraphicsParams.C \
	GraphicsParams.h \
	GraphicsTypes.h

libgraphics.la: libgraphics.o

libgraphics.o: $(libgraphics_la_OBJECTS) $(libgraphics_la_DEPENDENCIES)
	$(CXXLINK) $(libgraphics_la_OBJECTS)
