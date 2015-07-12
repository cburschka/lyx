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
  ${ZLIB_LIBRARY} ${ICONV_LIBRARY})
if(CYGWIN)
    target_link_libraries(check_layout shlwapi)
endif()
lyx_target_link_libraries(check_layout Magic)

add_dependencies(lyx_run_tests check_layout)
set_target_properties(check_layout PROPERTIES FOLDER "tests/src")

file(GLOB layout_files RELATIVE "${TOP_SRC_DIR}/lib/layouts" "${TOP_SRC_DIR}/lib/layouts/*.layout")
file(GLOB module_files RELATIVE "${TOP_SRC_DIR}/lib/layouts" "${TOP_SRC_DIR}/lib/layouts/*.module")
list(SORT layout_files)
list(SORT module_files)

foreach(bns ${layout_files} ${module_files})
  string(REGEX REPLACE "\\.(layout|module)" "" bn ${bns})
  set(_checktype ${CMAKE_MATCH_1})
  if("${_checktype}" STREQUAL "module")
    file(WRITE "${LYX_TESTS_USERDIR}/layouts/test${bn}.layout" "#% Do not delete the line below; configure depends on this\n")
    file(APPEND "${LYX_TESTS_USERDIR}/layouts/test${bn}.layout" "#  \\DeclareLaTeXClass{test${bn}}\n")

    file(APPEND "${LYX_TESTS_USERDIR}/layouts/test${bn}.layout" "Format 5\n")
    file(APPEND "${LYX_TESTS_USERDIR}/layouts/test${bn}.layout" "Input ${bns}\n")
    add_test(NAME "check_layout/${bns}"
      COMMAND env LYX_USERDIR_22x=${LYX_TESTS_USERDIR} $<TARGET_FILE:check_layout> "${LYX_TESTS_USERDIR}/layouts/test${bn}" "${CMAKE_CURRENT_BINARY_DIR}/${bn}.out${_checktype}")
  else()
    add_test(NAME "check_layout/${bns}"
      COMMAND check_layout "${TOP_SRC_DIR}/lib/layouts/${bn}" "${CMAKE_CURRENT_BINARY_DIR}/${bn}.out${_checktype}")
  endif()
  settestlabel("check_layout/${bns}" "${_checktype}")
endforeach()

set(check_ExternalTransforms_SOURCES)
foreach(_f graphics/GraphicsParams.cpp insets/ExternalTransforms.cpp
	Length.cpp lengthcommon.cpp tests/check_ExternalTransforms.cpp
	tests/boost.cpp tests/dummy_functions.cpp)
    list(APPEND check_ExternalTransforms_SOURCES ${TOP_SRC_DIR}/src/${_f})
endforeach()

include_directories(${TOP_SRC_DIR}/src/tests)
add_executable(check_ExternalTransforms ${check_ExternalTransforms_SOURCES})

target_link_libraries(check_ExternalTransforms support
	${Lyx_Boost_Libraries} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
lyx_target_link_libraries(check_ExternalTransforms Magic)

add_dependencies(lyx_run_tests check_ExternalTransforms)
set_target_properties(check_ExternalTransforms PROPERTIES FOLDER "tests/src")
target_link_libraries(check_ExternalTransforms ${ICONV_LIBRARY})

add_test(NAME "check_ExternalTransforms"
  COMMAND ${CMAKE_COMMAND} -DCommand=$<TARGET_FILE:check_ExternalTransforms>
	"-DInput=${TOP_SRC_DIR}/src/tests/regfiles/ExternalTransforms"
	"-DOutput=${CMAKE_CURRENT_BINARY_DIR}/ExternalTransforms_data"
	-P "${TOP_SRC_DIR}/src/support/tests/supporttest.cmake")
add_dependencies(lyx_run_tests check_ExternalTransforms)

set(check_ExternalTransforms_SOURCES)
foreach(_f graphics/GraphicsParams.cpp insets/ExternalTransforms.cpp
	Length.cpp lengthcommon.cpp tests/check_ExternalTransforms.cpp
	tests/boost.cpp tests/dummy_functions.cpp)
    list(APPEND check_ExternalTransforms_SOURCES ${TOP_SRC_DIR}/src/${_f})
endforeach()

include_directories(${TOP_SRC_DIR}/src/tests)
add_executable(check_Length ${check_Length_SOURCES})

target_link_libraries(check_Length support
	${Lyx_Boost_Libraries} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
lyx_target_link_libraries(check_Length Magic)

add_dependencies(lyx_run_tests check_Length)
set_target_properties(check_Length PROPERTIES FOLDER "tests/src")
target_link_libraries(check_Length ${ICONV_LIBRARY})

add_test(NAME "check_Length"
  COMMAND ${CMAKE_COMMAND} -DCommand=$<TARGET_FILE:check_Length>
	"-DInput=${TOP_SRC_DIR}/src/tests/regfiles/Length"
	"-DOutput=${CMAKE_CURRENT_BINARY_DIR}/Length_data"
	-P "${TOP_SRC_DIR}/src/support/tests/supporttest.cmake")
add_dependencies(lyx_run_tests check_Length)

