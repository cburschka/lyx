# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(boost_iostreams)

set(boost_iostreams_sources
	file_descriptor.cpp
	mapped_file.cpp
	zlib.cpp)

lyx_add_path(boost_iostreams_sources ${TOP_SRC_DIR}/boost/libs/iostreams/src)

include_directories(${ZLIB_INCLUDE_DIR})

if(NOT MERGE_FILES)
	add_library(boost_iostreams STATIC ${boost_iostreams_sources})
else()
	lyx_const_touched_files(_allinone  boost_iostreams_sources)
	add_library(boost_iostreams STATIC ${_allinone_files})
endif()

target_link_libraries(boost_iostreams ${ZLIB_LIBRARY})

