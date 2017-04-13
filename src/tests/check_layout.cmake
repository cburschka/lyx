# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2017 Kornel Benko kornel@lyx.org

# Should be called like
# cmake -DCheckLayout=$<TARGET_FILE:check_layout>
#       -DModule="${LYX_TESTS_USERDIR}/layouts/test${bn}"
#       -DOutput="${CMAKE_CURRENT_BINARY_DIR}/${bn}.out${_checktype}"
#       -P "${TOP_SRC_DIR}/src/tests/check_layout.cmake"

execute_process(COMMAND ${CheckLayout} ${Module} ${Output}
                   OUTPUT_VARIABLE _out
                   ERROR_VARIABLE _err
                   RESULT_VARIABLE _erg)
#message(STATUS "_out = \"${_out}\"")
string(COMPARE EQUAL ${_erg} 0 _erg1)
if(NOT _erg1)
  message(STATUS "_erg = \"${_erg}\"")
  message(FATAL_ERROR "${CheckLayout} failed")
endif()
if (_err MATCHES "Error reading")
  message(STATUS "_err = \"${_err}\"")
  message(FATAL_ERROR "Errors found")
endif()

