include $(top_srcdir)/config/common.am

EXTRA_DIST = pch.h

BUILT_SOURCES = $(PCH_FILE)

noinst_LTLIBRARIES = libboost_filesystem.la

AM_CPPFLAGS += \
	$(PCH_FLAGS) \
	-DBOOST_USER_CONFIG="<config.h>" \
	$(BOOST_INCLUDES)

libboost_filesystem_la_SOURCES = \
	exception.cpp \
	operations.cpp \
	path.cpp \
	portability.cpp
