include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libgraphics.la

INCLUDES = -I$(srcdir)/.. $(BOOST_INCLUDES)

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
	GraphicsLoader.h \
	GraphicsLoader.C \
	$(GRAPHICSIMAGEXPM) GraphicsParams.C \
	GraphicsParams.h \
	GraphicsSupport.h \
	GraphicsSupport.C \
	GraphicsTypes.h \
	GraphicsTypes.C \
	PreviewImage.h \
	PreviewImage.C \
	PreviewLoader.h \
	PreviewLoader.C \
	Previews.h \
	Previews.C \
	PreviewedInset.h \
	PreviewedInset.C
