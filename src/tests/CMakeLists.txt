# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2013 Kornel Benko kornel@lyx.org
#

set(check_layout_SOURCES)
foreach(_f insets/InsetLayout.cpp Color.cpp Counters.cpp Floating.cpp
       FloatList.cpp FontInfo.cpp Layout.cpp LayoutFile.cpp Lexer.cpp
       ModuleList.cpp Spacing.cpp TextClass.cpp tests/check_layout.cpp
       tests/boost.cpp tests/dummy_functions.cpp)
  list(APPEND check_layout_SOURCES "${TOP_SRC_DIR}/src/${_f}")
endforeach()

include_directories(${TOP_SRC_DIR}/src/support/tests)

add_executable(check_layout ${check_layout_SOURCES})

target_link_libraries(check_layout support
  ${Lyx_Boost_Libraries} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY}
  ${ZLIB_LIBRARY} ${ICONV_LIBRARY} ${Magic_LIBRARY})

add_dependencies(lyx_run_tests check_layout)
set_target_properties(check_layout PROPERTIES FOLDER "tests/src")

file(GLOB layout_files RELATIVE "${TOP_SRC_DIR}/lib/layouts" "${TOP_SRC_DIR}/lib/layouts/*.layout")
list(SORT layout_files)
foreach(bns ${layout_files}) 
  string(REPLACE ".layout" "" bn ${bns})

  add_test(NAME "check_layout/${bns}"
    COMMAND check_layout "${TOP_SRC_DIR}/lib/layouts/${bn}")
  settestlabel("check_layout/${bns}" "layout")
endforeach()
