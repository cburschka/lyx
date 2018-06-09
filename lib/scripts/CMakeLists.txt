# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2008-2011 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c) 2008-2011 Kornel Benko, <Kornel.Benko@berlin.de>
#

if (UNIX)
	set(_project "scripts")
	# include(../PyCompile)
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/prefTest.pl.in" "${CMAKE_BINARY_DIR}/prefTest.pl" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/checkKeys.pl.in" "${CMAKE_BINARY_DIR}/checkKeys.pl" @ONLY)
find_program(COMPAREPDF_EXECUTABLE comparepdf)
if (COMPAREPDF_EXECUTABLE MATCHES "-NOTFOUND")
  message(STATUS "comparepdf (= \"${COMPAREPDF_EXECUTABLE}\") not found ==> no pdf comparision in test AMS-import")
else()
  message(STATUS "found COMPAREPDF_EXECUTABLE = ${COMPAREPDF_EXECUTABLE}")
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/lyx_batch.pl.in" "${CMAKE_BINARY_DIR}/lyx_batch.pl" @ONLY)


