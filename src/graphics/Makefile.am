AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libgraphics.la
LIBS =
ETAGS_ARGS = --lang=c++
INCLUDES = -I${srcdir}/../ $(SIGC_CFLAGS)

libgraphics_la_SOURCES = \
	Renderer.h \
	Renderer.C \
	XPM_Renderer.h \
	XPM_Renderer.C \
	GraphicsCache.h \
	GraphicsCache.C \
	GraphicsCacheItem.h \
	GraphicsCacheItem.C
