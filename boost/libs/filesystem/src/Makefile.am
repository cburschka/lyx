include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libboost_filesystem.la

AM_CPPFLAGS = \
	$(PCH_FLAGS) \
	-DBOOST_USER_CONFIG="<config.h>" \
	$(BOOST_INCLUDES)

libboost_filesystem_la_SOURCES = \
	convenience.cpp \
	exception.cpp \
	Makefile.am \
	operations_posix_windows.cpp \
	path_posix_windows.cpp
