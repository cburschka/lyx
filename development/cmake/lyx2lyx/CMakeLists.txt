# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2008, 2009 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c) 2008, 2009 Kornel Benko, <Kornel.Benko@berlin.de>
#

project(lyx2lyx)

set(_project "scripts")
#include(../PyCompile)

include(FindPythonInterp)

file(GLOB _lyx2lyx_py_files ${TOP_SRC_DIR}/lib/lyx2lyx/*.py)

set(py_compile ${TOP_SRC_DIR}/config/py-compile)

set(_generated)
set(ENV{PYTHON} ${PYTHON_EXECUTABLE})

foreach(_orig_py ${_lyx2lyx_py_files})
  get_filename_component(_base_we_py ${_orig_py} NAME_WE)
  set(_compiled_py_ "${CMAKE_CURRENT_BINARY_DIR}/${_base_we_py}.pyc" "${CMAKE_CURRENT_BINARY_DIR}/${_base_we_py}.pyo")
  set(_copied_py "${CMAKE_CURRENT_BINARY_DIR}/${_base_we_py}.py")
# we need that file really local first, due to the "py_compile"-script
# because the creation will be in the directory of .py-file
  add_custom_command(
    OUTPUT ${_copied_py}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${_orig_py}" ${_copied_py}
    DEPENDS ${_orig_py}
    )
  add_custom_command(
    OUTPUT ${_compiled_py_}
    PRE_BUILD
    COMMAND ${py_compile} ${_copied_py}
    DEPENDS ${_copied_py}
    )
  SET_SOURCE_FILES_PROPERTIES(${_copied_py} ${_created_py_} GENERATED)
  install(FILES ${_compiled_py_} DESTINATION lyx2lyx)
  LIST(APPEND _generated ${_compiled_py_})
endforeach(_orig_py)

ADD_CUSTOM_TARGET(lyx2lyx ALL DEPENDS ${_generated})
