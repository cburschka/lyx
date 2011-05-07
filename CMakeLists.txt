# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c) 2008-2011 Kornel Benko, <Kornel.Benko@berlin.de>

cmake_minimum_required(VERSION 2.6.4)

set(LYX_PROJECT lyx)
project(${LYX_PROJECT})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/development/cmake/modules")

get_filename_component(lyx_dir_readme ${CMAKE_SOURCE_DIR}/README ABSOLUTE)
get_filename_component(TOP_SRC_DIR ${lyx_dir_readme} PATH)

set(CMAKE_PROJECT_NAME ${LYX_PROJECT})

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR}) 
	message(STATUS "Building in-source")
	set(TOP_BINARY_DIR "${CMAKE_BINARY_DIR}/build")
else()
	message(STATUS "Building out-of-source")
	set(TOP_BINARY_DIR "${CMAKE_BINARY_DIR}")
	cmake_policy(SET CMP0013 OLD)
endif()

add_subdirectory(development/cmake "${TOP_BINARY_DIR}")


