# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2013 Kornel Benko <kornel@lyx.org>
#           (c) 2013 Scott Kostyshak <skotysh@lyx.org>
#
# Script should be called like:
# COMMAND ${CMAKE_COMMAND} \
#     -DLYX_TESTS_USERDIR=${LYX_TESTS_USERDIR} \
#     -DLYX_USERDIR_VER=${LYX_USERDIR_VER} \
#     -DLYX_PYTHON_EXECUTABLE=${LYX_PYTHON_EXECUTABLE} \
#     -DPY_SCRIPT=${TOP_SRC_DIR}/src/tex2lyx/test/runtests.py \
#     -DFIRST_PARAM=
#     -DTEX2LYX_EXE=$<TARGET_FILE:${_tex2lyx}> \
#     -DSCRIPT_DIR=${TOP_SRC_DIR}/lib/scripts \
#     -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR} \
#     -DTESTFILE=${fl} \
#     -P ${TOP_SRC_DIR}/src/tex2lyx/test/runtests.cmake

set(ENV{${LYX_USERDIR_VER}} ${LYX_TESTS_USERDIR})
message(STATUS "SCRIPT_DIR = ${SCRIPT_DIR}")

execute_process(COMMAND ${LYX_PYTHON_EXECUTABLE} ${PY_SCRIPT} ${FIRST_PARAM}
  ${TEX2LYX_EXE} ${SCRIPT_DIR} ${WORKDIR} ${TESTFILE}
  RESULT_VARIABLE _err
  OUTPUT_VARIABLE _out)

message(STATUS ${_out})
message(STATUS "Error output of ${PY_SCRIPT} = ${_err}")
string(COMPARE NOTEQUAL  ${_err} 0 _erg)

if(_erg)
  message(FATAL_ERROR "${PY_SCRIPT} failed")
endif()


