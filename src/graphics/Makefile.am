AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libgraphics.la
LIBS =
ETAGS_ARGS = --lang=c++
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${srcdir}/../ $(SIGC_CFLAGS) $(BOOST_INCLUDES)

libgraphics_la_SOURCES = \
	GraphicsCache.h \
	GraphicsCache.C \
	GraphicsCacheItem.h \
	GraphicsCacheItem.C \
	ImageLoaderXPM.h \
	ImageLoaderXPM.C \
	ImageLoader.h \
	ImageLoader.C 
