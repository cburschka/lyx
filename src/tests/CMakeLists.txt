# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2013 Kornel Benko kornel@lyx.org
#

set(check_Layout_SOURCES)
foreach(_f insets/InsetLayout.cpp Color.cpp Counters.cpp Floating.cpp
       FloatList.cpp FontInfo.cpp Layout.cpp LayoutFile.cpp Lexer.cpp
       ModuleList.cpp Spacing.cpp TextClass.cpp tests/check_Layout.cpp
       support/tests/boost.cpp support/tests/dummy_functions.cpp)
  list(APPEND check_Layout_SOURCES "${TOP_SRC_DIR}/src/${_f}")
endforeach()

add_definitions(-DNO_LAYOUT_CSS)
add_executable(check_Layout ${check_Layout_SOURCES})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-multiple-definition")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /FORCE:MULTIPLE")
else()
  message(STATUS "Allow multiple definitions for cxx-compiler ${CMAKE_CXX_COMPILER_ID} not handled")
endif()

target_link_libraries(check_Layout support ${QT_QTMAIN_LIBRARY})
add_dependencies(lyx_run_tests check_Layout)

file(GLOB layout_files RELATIVE "${TOP_SRC_DIR}/lib/layouts" "${TOP_SRC_DIR}/lib/layouts/*.layout")
list(SORT layout_files)
foreach(bns ${layout_files}) 
  string(REPLACE ".layout" "" bn ${bns})

  add_test(NAME "check_Layout/${bns}"
    COMMAND check_Layout "${TOP_SRC_DIR}/lib/layouts/${bn}")
endforeach()
