include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libboostsignals.la

INCLUDES = $(BOOST_INCLUDES)

AM_CXXFLAGS = -DBOOST_USER_CONFIG="<config.h>"

libboostsignals_la_SOURCES = \
	connection.cpp \
	signal_base.cpp \
	slot.cpp \
	trackable.cpp
