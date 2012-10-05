# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko, kornel@lyx.org
#

project(supporttest)

macro(sources _program)
	set(_tmplist)
	foreach(_tmp ${ARGN})
		list(APPEND _tmplist "${TOP_SRC_DIR}/src/support/tests/${_tmp}")
	endforeach()
	set(${_program}_SOURCES ${_tmplist})
	add_executable(${_program} ${_tmplist})
	target_link_libraries(${_program} support ${Lyx_Boost_Libraries} ${QT_QTCORE_LIBRARY} ${ZLIB_LIBRARY})
endmacro()

file(GLOB test_sources ${TOP_SRC_DIR}/src/support/tests/${LYX_CPP_FILES})

include_directories(
	${TOP_SRC_DIR}/src/support/tests
	${QT_INCLUDES}
	${ICONV_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${LIBINTL_INCLUDE_DIR})


set(check_PROGRAMS check_convert check_filetools check_lstrings)

set(_depends)
foreach(_src ${check_PROGRAMS})
	sources(${_src} ${_src}.cpp dummy_functions.cpp boost.cpp)
	#message(STATUS "${_src}_SOURCES = " ${${_src}_SOURCES})
	string(REPLACE "check_" "" _srcx ${_src})
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/regfiles/${_src}
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/regfiles"
		COMMAND "${CMAKE_BINARY_DIR}/bin/${_src}" > ${CMAKE_CURRENT_BINARY_DIR}/regfiles/${_src}
		COMMAND ${CMAKE_COMMAND} -E compare_files "${TOP_SRC_DIR}/src/support/tests/regfiles/${_srcx}" "${CMAKE_CURRENT_BINARY_DIR}/regfiles/${_src}"
		DEPENDS	"${CMAKE_BINARY_DIR}/bin/${_src}" "${TOP_SRC_DIR}/src/support/tests/regfiles/${_srcx}"
		COMMENT "'${CMAKE_BINARY_DIR}/bin/${_src}' > '${CMAKE_CURRENT_BINARY_DIR}/regfiles/${_src}'"
	)
	list(APPEND _depends "${CMAKE_CURRENT_BINARY_DIR}/regfiles/${_src}")
endforeach()

add_custom_target(checkregfiles DEPENDS support ${_depends})

