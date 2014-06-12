# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2008-2011 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c) 2008-2011 Kornel Benko, <Kornel.Benko@berlin.de>
#

configure_file(${TOP_SRC_DIR}/lib/lyx2lyx/lyx2lyx_version.py.in 
               ${CMAKE_CURRENT_BINARY_DIR}/lyx2lyx_version.py)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lyx2lyx_version.py DESTINATION ${LYX_DATA_SUBDIR}lyx2lyx)

if (UNIX)
	set(_project "lyx2lyx")
	# include(../PyCompile)
endif()

set(_testname "lyx2lyx/parser_tools")
add_test(NAME ${_testname}
  COMMAND ${LYX_PYTHON_EXECUTABLE} "${TOP_SRC_DIR}/lib/lyx2lyx/test_parser_tools.py"
  )
settestlabel(${_testname} "lyx2lyx")

install(PROGRAMS ${TOP_SRC_DIR}/lib/lyx2lyx/lyx2lyx DESTINATION ${LYX_DATA_SUBDIR}lyx2lyx)
