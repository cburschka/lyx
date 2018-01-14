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
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/lyx_batch.pl.in" "${CMAKE_BINARY_DIR}/lyx_batch.pl" @ONLY)


