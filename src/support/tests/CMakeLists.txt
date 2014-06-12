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
	target_link_libraries(${_program} support
		${Lyx_Boost_Libraries} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY}
		${ZLIB_LIBRARY} ${ICONV_LIBRARY} ${Magic_LIBRARY})
endmacro()

file(GLOB test_sources ${TOP_SRC_DIR}/src/support/tests/${LYX_CPP_FILES})

include_directories(
	${TOP_SRC_DIR}/src/support/tests
	${QT_INCLUDES}
	${ICONV_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR})


set(check_PROGRAMS check_convert check_filetools check_lstrings)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/regfiles")

set(_depends)
foreach(_src ${check_PROGRAMS})
	sources(${_src} ${_src}.cpp dummy_functions.cpp boost.cpp)
	string(REPLACE "check_" "" _srcx ${_src})
	set(src_data_file "${TOP_SRC_DIR}/src/support/tests/regfiles/${_srcx}")
	if (MSVC AND EXISTS "${src_data_file}.MSVC")
		set(src_data_file "${src_data_file}.MSVC")
	endif()
	set(created_data_file "${CMAKE_CURRENT_BINARY_DIR}/regfiles/${_src}_data")
        add_test(NAME support/${_srcx}
          COMMAND ${CMAKE_COMMAND} -DCommand=$<TARGET_FILE:${_src}>
                                   -DInput=${src_data_file}
                                   -DOutput=${created_data_file}
                                   -P "${TOP_SRC_DIR}/src/support/tests/supporttest.cmake")

	set_target_properties(${_src} PROPERTIES FOLDER "tests/support")
	add_dependencies(lyx_run_tests ${_src})
endforeach()


