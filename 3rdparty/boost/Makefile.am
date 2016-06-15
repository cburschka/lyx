include $(top_srcdir)/config/common.am

noinst_LIBRARIES = liblyxboost.a

EXTRA_DIST = boost \
	CMakeLists.txt \
	LICENSE_1_0.txt \
	libs/CMakeLists.txt \
	libs/regex/CMakeLists.txt \
	libs/regex/src/CMakeLists.txt \
	libs/regex/src/usinstances.cpp
# The last file above is only compiled for msvc (see cmake)

AM_CPPFLAGS += -DBOOST_USER_CONFIG="<config.h>" 
AM_CPPFLAGS +=  $(BOOST_INCLUDES)

# This version is more than three times faster than the one below

if MONOLITHIC_BOOST

liblyxboost_a_SOURCES = \
	lyxboost.cpp \
	libs/regex/src/instances.cpp \
	libs/regex/src/cpp_regex_traits.cpp \
	libs/regex/src/c_regex_traits.cpp \
	libs/regex/src/wc_regex_traits.cpp 

else

liblyxboost_a_SOURCES = \
	libs/regex/src/internals.hpp \
	libs/regex/src/c_regex_traits.cpp \
	libs/regex/src/cpp_regex_traits.cpp \
	libs/regex/src/cregex.cpp \
	libs/regex/src/fileiter.cpp \
	libs/regex/src/instances.cpp \
	libs/regex/src/posix_api.cpp \
	libs/regex/src/regex.cpp \
	libs/regex/src/regex_debug.cpp \
	libs/regex/src/regex_raw_buffer.cpp \
	libs/regex/src/regex_traits_defaults.cpp \
	libs/regex/src/w32_regex_traits.cpp \
	libs/regex/src/wc_regex_traits.cpp \
	libs/regex/src/wide_posix_api.cpp \
	libs/regex/src/winstances.cpp \
	libs/regex/src/static_mutex.cpp

endif #MONOLITHIC_BOOST

