include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libboostfilesystem.la

INCLUDES = $(BOOST_INCLUDES)

AM_CPPFLAGS = -DBOOST_USER_CONFIG="<config.h>"
AM_CXXFLAGS = $(PCH_FLAGS)

libboostfilesystem_la_SOURCES = \
	convenience.cpp \
	exception.cpp \
	Makefile.am \
	operations_posix_windows.cpp \
	path_posix_windows.cpp
