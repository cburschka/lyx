include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libboost_signals.la

AM_CPPFLAGS += \
	$(PCH_FLAGS) \
	-DBOOST_USER_CONFIG="<config.h>" \
	$(BOOST_INCLUDES)

libboost_signals_la_SOURCES = \
	connection.cpp \
	named_slot_map.cpp \
	signal_base.cpp \
	slot.cpp \
	trackable.cpp
