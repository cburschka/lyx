# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(boost_regex)

set(boost_regex_sources
	cpp_regex_traits.cpp
	c_regex_traits_common.cpp
	c_regex_traits.cpp
	cregex.cpp
	fileiter.cpp
	instances.cpp
	primary_transform.hpp
	regex.cpp
	w32_regex_traits.cpp
	regex_synch.cpp
)

lyx_add_path(boost_regex_sources ${TOP_SRC_DIR}/boost/libs/regex/src)

add_library(boost_regex STATIC ${boost_regex_sources})


