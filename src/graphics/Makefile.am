include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libgraphics.la

INCLUDES = -I$(srcdir)/.. $(BOOST_INCLUDES)

AM_CXXFLAGS = $(PCH_FLAGS)

libgraphics_la_SOURCES = \
	GraphicsCache.h \
	GraphicsCache.C \
	GraphicsCacheItem.h \
	GraphicsCacheItem.C \
	GraphicsConverter.h \
	GraphicsConverter.C \
	GraphicsImage.h \
	GraphicsImage.C \
	GraphicsLoader.h \
	GraphicsLoader.C \
	GraphicsParams.C \
	GraphicsParams.h \
	LoaderQueue.h \
	LoaderQueue.C \
	GraphicsTypes.h \
	GraphicsTypes.C \
	PreviewImage.h \
	PreviewImage.C \
	PreviewLoader.h \
	PreviewLoader.C \
	Previews.h \
	Previews.C
