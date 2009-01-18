# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2008, 2009 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c) 2008, 2009 Kornel Benko, <Kornel.Benko@berlin.de>
#

configure_file(${TOP_SRC_DIR}/lib/lyx2lyx/lyx2lyx_version.py.in 
               ${CMAKE_CURRENT_BINARY_DIR}/lyx2lyx_version.py)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lyx2lyx_version.py DESTINATION lyx2lyx)

if (UNIX)
  set(_project "lyx2lyx")
  include(../PyCompile)
endif()

install(PROGRAMS ${TOP_SRC_DIR}/lib/lyx2lyx/lyx2lyx DESTINATION lyx2lyx)
