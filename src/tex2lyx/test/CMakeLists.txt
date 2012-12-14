# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko, <kornel@lyx.org>
#

project(test)

file(GLOB _tex_tests RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.ltx" "${CMAKE_CURRENT_SOURCE_DIR}/*.tex")
#set(_tex_tests test.ltx test-structure test-insets test-modules box-color-size-space-align CJK XeTeX-polyglossia)
list(REMOVE_ITEM _tex_tests DummyDocument.tex)

add_test(NAME tex2lyx/create_lyx
  COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target ${_lyx})

add_test(NAME tex2lyx/create_tex2lyx
  COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target ${_tex2lyx})
set_tests_properties(tex2lyx/create_tex2lyx PROPERTIES DEPENDS tex2lyx/create_lyx)

foreach(_fl ${_tex_tests})
  set(fl ${_fl})
  add_test(NAME tex2lyx/roundtrip/${_fl}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${LYX_PYTHON_EXECUTABLE} "${TOP_SRC_DIR}/src/tex2lyx/test/runtests.py"
    "${TOP_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/${_tex2lyx}"
    "${TOP_SRC_DIR}/lib/scripts"
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${fl})
  set_tests_properties(tex2lyx/roundtrip/${_fl} PROPERTIES DEPENDS tex2lyx/create_tex2lyx)
endforeach()


