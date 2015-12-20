# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
#

project(boost_signals)

set(boost_signals_sources
	connection.cpp 
	named_slot_map.cpp 
	signal_base.cpp 
	slot.cpp 
	trackable.cpp
)

lyx_add_path(boost_signals_sources ${TOP_SRC_DIR}/3rdparty/boost/libs/signals/src)

if(NOT LYX_MERGE_FILES)
	add_library(boost_signals STATIC ${boost_signals_sources})
else()
	lyx_const_touched_files(_allinone  boost_signals_sources)
	add_library(boost_signals STATIC ${_allinone_files})
endif()

set_target_properties(boost_signals PROPERTIES FOLDER "3rd_party")
