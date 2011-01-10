# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2010-2011 Peter Kümmel, <syntheticpp@gmx.net>
#

project(boost)


if(LYX_USE_TR1_REGEX)
	message(STATUS "Using TR1 regex")
else()
	add_subdirectory(regex) 
endif()


add_subdirectory(signals) 

