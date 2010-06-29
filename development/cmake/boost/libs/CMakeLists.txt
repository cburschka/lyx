# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(boost)

if(MSVC10)
	message(STATUS "Using TR1 regex")
else()
	add_subdirectory(regex) 
endif()

add_subdirectory(signals) 

