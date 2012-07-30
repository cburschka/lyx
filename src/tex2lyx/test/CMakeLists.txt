# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko, <kornel@lyx.org>
#

project(test)

set(_test_depend "${TOP_SRC_DIR}/src/tex2lyx/test/test.ltx")
set(_test_output "${TOP_SRC_DIR}/src/tex2lyx/test/test.lyx.tex")
foreach(_arg runtests.py DummyDocument.tex test.ltx foo.eps foo.png)
  configure_file("${TOP_SRC_DIR}/src/tex2lyx/test/${_arg}" "${CMAKE_CURRENT_BINARY_DIR}/${_arg}" COPYONLY)
endforeach(_arg)

foreach(_arg test-structure test-insets box-color-size-space-align CJK XeTeX-polyglossia)
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_arg}.tex"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TOP_SRC_DIR}/src/tex2lyx/test/${_arg}.tex" "${CMAKE_CURRENT_BINARY_DIR}/${_arg}.tex"
    DEPENDS "${TOP_SRC_DIR}/src/tex2lyx/test/${_arg}.tex"
    )
  list(APPEND _test_depend "${CMAKE_CURRENT_BINARY_DIR}/${_arg}.tex")
  list(APPEND _test_output "${CMAKE_CURRENT_BINARY_DIR}/${_arg}.lyx.tex")
  list(APPEND _test_output "${CMAKE_CURRENT_BINARY_DIR}/${_arg}.lyx.lyx")
endforeach(_arg)

ADD_CUSTOM_COMMAND(
  OUTPUT ${_test_output}
  COMMAND ${LYX_PYTHON_EXECUTABLE}
  ARGS "${CMAKE_CURRENT_BINARY_DIR}/runtests.py" "${TOP_BINARY_DIR}/bin/${_tex2lyx}" "${TOP_SRC_DIR}/lib/scripts"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS ${_tex2lyx} ${_test_depend}
)

ADD_CUSTOM_TARGET(test DEPENDS ${_tex2lyx} ${_test_output})

