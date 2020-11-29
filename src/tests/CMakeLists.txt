# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2013 Kornel Benko kornel@lyx.org
#

set(check_layout_SOURCES)
foreach(_f insets/InsetLayout.cpp CiteEnginesList.cpp Color.cpp Counters.cpp Floating.cpp
       FloatList.cpp FontInfo.cpp Layout.cpp LayoutFile.cpp Lexer.cpp
       ModuleList.cpp Spacing.cpp TextClass.cpp tests/check_layout.cpp
       tests/dummy_functions.cpp)
  list(APPEND check_layout_SOURCES "${TOP_SRC_DIR}/src/${_f}")
endforeach()

include_directories(${TOP_SRC_DIR}/src/support/tests)

add_executable(check_layout ${check_layout_SOURCES})

target_link_libraries(check_layout support
  ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY}
  ${ZLIB_LIBRARY} ${ICONV_LIBRARY})
if(CYGWIN)
    target_link_libraries(check_layout shlwapi)
endif()
lyx_target_link_libraries(check_layout Magic)

add_dependencies(lyx_run_tests check_layout)
set_target_properties(check_layout PROPERTIES FOLDER "tests/src")

file(GLOB layout_files RELATIVE "${TOP_SRC_DIR}/lib/layouts" "${TOP_SRC_DIR}/lib/layouts/*.layout")
file(GLOB module_files RELATIVE "${TOP_SRC_DIR}/lib/layouts" "${TOP_SRC_DIR}/lib/layouts/*.module")
file(GLOB citeengine_files RELATIVE "${TOP_SRC_DIR}/lib/citeengines" "${TOP_SRC_DIR}/lib/citeengines/*.citeengine")
list(SORT layout_files)
list(SORT module_files)
list(SORT citeengine_files)

foreach(bns ${layout_files} ${module_files} ${citeengine_files})
  string(REGEX REPLACE "\\.(layout|module|citeengine)" "" bn ${bns})
  set(_checktype ${CMAKE_MATCH_1})
  if("${_checktype}" MATCHES "module|citeengine")
    file(WRITE "${LYX_TESTS_USERDIR}/layouts/test${bn}.layout" "#% Do not delete the line below; configure depends on this\n")
    file(APPEND "${LYX_TESTS_USERDIR}/layouts/test${bn}.layout" "#  \\DeclareLaTeXClass{test${bn}}\n")
    file(APPEND "${LYX_TESTS_USERDIR}/layouts/test${bn}.layout" "Format 74\n")
    file(APPEND "${LYX_TESTS_USERDIR}/layouts/test${bn}.layout" "Style Standard\n")
    file(APPEND "${LYX_TESTS_USERDIR}/layouts/test${bn}.layout" " LabelType No_Label\n")
    file(APPEND "${LYX_TESTS_USERDIR}/layouts/test${bn}.layout" " Category MainText\n")
    file(APPEND "${LYX_TESTS_USERDIR}/layouts/test${bn}.layout" "End\n")
    if ("${_checktype}" STREQUAL "citeengine")
      file(APPEND "${LYX_TESTS_USERDIR}/layouts/test${bn}.layout" "Input \"${TOP_SRC_DIR}/lib/citeengines/${bns}\"\n")
    else()
      file(APPEND "${LYX_TESTS_USERDIR}/layouts/test${bn}.layout" "Input ${bns}\n")
    endif()
    add_test(NAME "check_layout/${bns}"
      COMMAND ${CMAKE_COMMAND}
	"-DCheckLayout=$<TARGET_FILE:check_layout>"
	"-DModule=${LYX_TESTS_USERDIR}/layouts/test${bn}"
	"-DOutput=${CMAKE_CURRENT_BINARY_DIR}/${bn}.out${_checktype}"
	-P "${TOP_SRC_DIR}/src/tests/check_layout.cmake")
    set_tests_properties("check_layout/${bns}" PROPERTIES ENVIRONMENT ${LYX_USERDIR_VER}=${LYX_TESTS_USERDIR})
  else()
    add_test(NAME "check_layout/${bns}"
      COMMAND check_layout "${TOP_SRC_DIR}/lib/layouts/${bn}" "${CMAKE_CURRENT_BINARY_DIR}/${bn}.out${_checktype}")
  endif()
  settestlabel("check_layout/${bns}" "${_checktype}")
endforeach()

set(check_ExternalTransforms_SOURCES)
foreach(_f graphics/GraphicsParams.cpp insets/ExternalTransforms.cpp
	tests/check_ExternalTransforms.cpp
	tests/dummy_functions.cpp)
    list(APPEND check_ExternalTransforms_SOURCES ${TOP_SRC_DIR}/src/${_f})
endforeach()

include_directories(${TOP_SRC_DIR}/src/tests)
add_executable(check_ExternalTransforms ${check_ExternalTransforms_SOURCES})

target_link_libraries(check_ExternalTransforms support
	${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
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

set(check_Length_SOURCES)
foreach(_f tests/check_Length.cpp tests/dummy_functions.cpp)
  list(APPEND check_Length_SOURCES ${TOP_SRC_DIR}/src/${_f})
endforeach()
add_executable(check_Length ${check_Length_SOURCES})

target_link_libraries(check_Length support
	${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
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

include_directories(${TOP_SRC_DIR}/src/tests)
set(check_ListingsCaption_SOURCES)
foreach(_f tests/check_ListingsCaption.cpp tests/dummy_functions.cpp)
  list(APPEND check_ListingsCaption_SOURCES ${TOP_SRC_DIR}/src/${_f})
endforeach()
add_executable(check_ListingsCaption ${check_ListingsCaption_SOURCES})

target_link_libraries(check_ListingsCaption support
	${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
lyx_target_link_libraries(check_ListingsCaption Magic)

add_dependencies(lyx_run_tests check_ListingsCaption)
set_target_properties(check_ListingsCaption PROPERTIES FOLDER "tests/src")
target_link_libraries(check_ListingsCaption ${ICONV_LIBRARY})

add_test(NAME "check_ListingsCaption"
  COMMAND ${CMAKE_COMMAND} -DCommand=$<TARGET_FILE:check_ListingsCaption>
	"-DInput=${TOP_SRC_DIR}/src/tests/regfiles/ListingsCaption"
	"-DOutput=${CMAKE_CURRENT_BINARY_DIR}/ListingsCaption_data"
	-P "${TOP_SRC_DIR}/src/support/tests/supporttest.cmake")
add_dependencies(lyx_run_tests check_ListingsCaption)

