include $(top_srcdir)/config/common.am

EXTRA_DIST = boost \
	CMakeLists.txt \
	LICENSE_1_0.txt \
	libs/CMakeLists.txt \
	libs/regex/CMakeLists.txt \
	libs/regex/src/CMakeLists.txt

AM_CPPFLAGS += -DBOOST_USER_CONFIG="<config.h>" 
AM_CPPFLAGS +=  $(BOOST_INCLUDES)

