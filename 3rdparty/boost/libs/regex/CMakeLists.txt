# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
#

project(boost_regex)

set(boost_regex_sources
	cpp_regex_traits.cpp
	c_regex_traits.cpp
	cregex.cpp
	fileiter.cpp
	instances.cpp
	regex.cpp
	regex_raw_buffer.cpp
	regex_traits_defaults.cpp
	static_mutex.cpp
	w32_regex_traits.cpp)

lyx_add_path(boost_regex_sources ${TOP_SRC_DIR}/3rdparty/boost/libs/regex/src)

add_library(boost_regex STATIC ${boost_regex_sources})

set_target_properties(boost_regex PROPERTIES FOLDER "3rd_party")

