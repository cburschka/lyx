include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libboostregex.la

INCLUDES = $(BOOST_INCLUDES)

AM_CPPFLAGS = -DBOOST_USER_CONFIG="<config.h>"
AM_CXXFLAGS = $(PCH_FLAGS)

libboostregex_la_SOURCES = \
	cpp_regex_traits.cpp \
	c_regex_traits_common.cpp \
	c_regex_traits.cpp \
	cregex.cpp \
	fileiter.cpp \
	instances.cpp \
	primary_transform.hpp \
	regex.cpp \
	w32_regex_traits.cpp \
	regex_synch.cpp

#	posix_api.cpp \
#	wide_posix_api.cpp \
#	regex_debug.cpp \
#	winstances.cpp
