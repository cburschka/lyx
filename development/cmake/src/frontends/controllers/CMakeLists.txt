# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(controllers)

file(GLOB controllers_sources ${TOP_SRC_DIR}/src/frontends/controllers/*.C)
file(GLOB controllers_headers ${TOP_SRC_DIR}/src/frontends/controllers/*.h)

include_directories(${TOP_SRC_DIR}/src/frontends/controllers)

add_library(controllers STATIC ${controllers_sources} ${controllers_headers})

target_link_libraries(controllers boost_regex boost_filesystem)

project_source_group("${GROUP_CODE}" controllers_sources controllers_headers)

