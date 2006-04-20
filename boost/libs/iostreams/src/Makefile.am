include $(top_srcdir)/config/common.am

EXTRA_DIST = pch.h

BUILT_SOURCES = $(PCH_FILE)

noinst_LTLIBRARIES = libboost_iostreams.la

AM_CPPFLAGS += \
       $(PCH_FLAGS) \
       -DBOOST_USER_CONFIG="<config.h>" \
       $(BOOST_INCLUDES)

libboost_iostreams_la_SOURCES = \
	bzip2.cpp \
	file_descriptor.cpp \
	mapped_file.cpp \
	zlib.cpp
