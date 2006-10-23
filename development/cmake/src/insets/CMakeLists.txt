# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(insets)

file(GLOB insets_sources ${TOP_SRC_DIR}/src/insets/*.C)
file(GLOB insets_headers ${TOP_SRC_DIR}/src/insets/*.h)

list(REMOVE_ITEM insets_sources ${TOP_SRC_DIR}/src/insets/insettheorem.C)

lyx_add_msvc_pch(insets)

include_directories(${TOP_SRC_DIR}/src/insets)

add_library(insets STATIC ${insets_sources} ${insets_headers})

project_source_group("${GROUP_CODE}" insets_sources insets_headers)

