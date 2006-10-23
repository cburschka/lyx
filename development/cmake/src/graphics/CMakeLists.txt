# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(graphics)

file(GLOB graphics_sources ${TOP_SRC_DIR}/src/graphics/*.C)
file(GLOB graphics_headers ${TOP_SRC_DIR}/src/graphics/*.h)

lyx_add_msvc_pch(graphics_sources)

include_directories(${TOP_SRC_DIR}/src/graphics)

add_library(graphics STATIC ${graphics_sources} ${graphics_headers})

add_dependencies(graphics ${pchlibname})

project_source_group("${GROUP_CODE}" graphics_sources graphics_headers)

