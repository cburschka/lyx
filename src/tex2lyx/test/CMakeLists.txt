# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko, <kornel@lyx.org>
#

project(test)

#file(GLOB _tex_tests RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.ltx" "${CMAKE_CURRENT_SOURCE_DIR}/*.tex")
#
#list(REMOVE_ITEM _tex_tests DummyDocument.tex)

set(_tex_tests test.ltx test-structure.tex test-insets.tex
               test-modules.tex box-color-size-space-align.tex
               CJK.tex XeTeX-polyglossia.tex)

foreach(_fl ${_tex_tests})
  set(fl ${_fl})
  add_test(NAME tex2lyx/roundtrip/${_fl}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${LYX_PYTHON_EXECUTABLE} "${TOP_SRC_DIR}/src/tex2lyx/test/runtests.py"
    "$<TARGET_FILE:${_tex2lyx}>"
    "${TOP_SRC_DIR}/lib/scripts"
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${fl})
  add_test(NAME tex2lyx2lyx/roundtrip/${_fl}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${LYX_PYTHON_EXECUTABLE} "${TOP_SRC_DIR}/src/tex2lyx/test/runtests.py" "uselyx2lyx"
    "$<TARGET_FILE:${_tex2lyx}>"
    "${TOP_SRC_DIR}/lib/scripts"
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${fl})
endforeach()

add_dependencies(lyx_run_tests ${_tex2lyx} ${_lyx})

