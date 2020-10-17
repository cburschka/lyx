# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2011 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c) 2011 Kornel Benko, <Kornel.Benko@berlin.de>
#

project(doxydoc)

find_program(DOXYGEN_EXECUTABLE doxygen)
if(DOXYGEN_EXECUTABLE MATCHES "-NOTFOUND")
  message(STATUS "doxygen not found ==> no doxygen creation")
else()
  set(VERSION ${PACKAGE_VERSION})
  set(top_srcdir "${TOP_SRC_DIR}")
  configure_file("${TOP_SRC_DIR}/sourcedoc/Doxyfile.in" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)
  add_custom_command(
    OUTPUT doxy_created
    COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    COMMAND ${CMAKE_COMMAND} -E touch doxy_created
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
  )
  add_custom_target(doxydoc DEPENDS doxy_created)
endif()

