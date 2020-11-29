# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
#

project(frontends)

include_directories(${TOP_SRC_DIR}/src/frontends)

add_subdirectory(qt)

file(GLOB frontends_sources ${TOP_SRC_DIR}/src/frontends/${LYX_CPP_FILES})
file(GLOB frontends_headers ${TOP_SRC_DIR}/src/frontends/${LYX_HPP_FILES})


if(NOT LYX_MERGE_FILES)
	add_library(frontends ${library_type} ${frontends_sources} ${frontends_headers})
else()
	lyx_const_touched_files(_allinone  frontends_sources)
	add_library(frontends ${library_type} ${_allinone_files})
endif()
set_target_properties(frontends PROPERTIES FOLDER "applications/LyX")

#target_link_libraries(frontends)

project_source_group("${GROUP_CODE}" frontends_sources frontends_headers)

add_subdirectory(tests)
