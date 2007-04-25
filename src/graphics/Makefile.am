include $(top_srcdir)/config/common.am

EXTRA_DIST = pch.h

BUILT_SOURCES = $(PCH_FILE)

noinst_LTLIBRARIES = libgraphics.la

AM_CPPFLAGS += $(PCH_FLAGS) -I$(srcdir)/.. $(BOOST_INCLUDES)

libgraphics_la_SOURCES = \
	GraphicsCache.h \
	GraphicsCache.cpp \
	GraphicsCacheItem.h \
	GraphicsCacheItem.cpp \
	GraphicsConverter.h \
	GraphicsConverter.cpp \
	GraphicsImage.h \
	GraphicsImage.cpp \
	GraphicsLoader.h \
	GraphicsLoader.cpp \
	GraphicsParams.cpp \
	GraphicsParams.h \
	LoaderQueue.h \
	LoaderQueue.cpp \
	GraphicsTypes.h \
	GraphicsTypes.cpp \
	PreviewImage.h \
	PreviewImage.cpp \
	PreviewLoader.h \
	PreviewLoader.cpp \
	Previews.h \
	Previews.cpp
