include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libboostsignals.la

AM_CPPFLAGS = \
	$(PCH_FLAGS) \
	-DBOOST_USER_CONFIG="<config.h>" \
	$(BOOST_INCLUDES)

libboostsignals_la_SOURCES = \
	connection.cpp \
	named_slot_map.cpp \
	signal_base.cpp \
	slot.cpp \
	trackable.cpp
