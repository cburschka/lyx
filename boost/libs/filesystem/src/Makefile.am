include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libboostfilesystem.la

AM_CPPFLAGS = \
	$(PCH_FLAGS) \
	-DBOOST_USER_CONFIG="<config.h>" \
	$(BOOST_INCLUDES)

libboostfilesystem_la_SOURCES = \
	convenience.cpp \
	exception.cpp \
	Makefile.am \
	operations_posix_windows.cpp \
	path_posix_windows.cpp
