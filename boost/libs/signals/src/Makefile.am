include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libboostsignals.la

AM_CPPFLAGS = \
	-DBOOST_USER_CONFIG="<config.h>" \
	$(BOOST_INCLUDES)

AM_CXXFLAGS = $(PCH_FLAGS)

libboostsignals_la_SOURCES = \
	connection.cpp \
	named_slot_map.cpp \
	signal_base.cpp \
	slot.cpp \
	trackable.cpp
