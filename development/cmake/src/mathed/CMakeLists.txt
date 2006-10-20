# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(mathed)

file(GLOB mathed_sources ${TOP_SRC_DIR}/src/mathed/*.C)
file(GLOB mathed_headers ${TOP_SRC_DIR}/src/mathed/*.h)

list(REMOVE_ITEM mathed_sources
	${TOP_SRC_DIR}/src/mathed/InsetMathXYArrow.C
	${TOP_SRC_DIR}/src/mathed/InsetMathMBox.C
	${TOP_SRC_DIR}/src/mathed/InsetFormulaMacro.C)

include_directories(${TOP_SRC_DIR}/src/mathed)

add_library(mathed STATIC ${mathed_sources} ${mathed_headers})

project_source_group("${GROUP_CODE}" mathed_sources mathed_headers)

