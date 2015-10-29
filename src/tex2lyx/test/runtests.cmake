# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2013-2014 Kornel Benko <kornel@lyx.org>
#           (c) 2013-2014 Scott Kostyshak <skotysh@lyx.org>
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

set(_configure_needed FALSE)
foreach(_f lyxrc.defaults lyxmodules.lst textclass.lst packages.lst)
  if(NOT EXISTS "${LYX_TESTS_USERDIR}/${_f}")
    message(STATUS "Configure needed, because \"${LYX_TESTS_USERDIR}/${_f}\" does not exist")
    set(_configure_needed TRUE)
    break()
  endif()
  if("${SCRIPT_DIR}/../configure.py" IS_NEWER_THAN "${LYX_TESTS_USERDIR}/${_f}")
    message(STATUS "Configure needed, because \"${LYX_TESTS_USERDIR}/${_f}\" too old")
    set(_configure_needed TRUE)
    break()
  endif()
endforeach()

if(_configure_needed)
  # Determine suffix from ${TEX2LYX_EXE}
  if(TEX2LYX_EXE MATCHES ".*tex2lyx([0-9]\\.[0-9]).*")
    set(_suffix ${CMAKE_MATCH_1})
  else()
    set(_suffix "")
  endif()

  if(_suffix STREQUAL "")
    set(_with_ver "")
  else()
    set(_with_ver "--with-version-suffix=${_suffix}")
  endif()

  # Get binary directory of tex2lyx
  string(REGEX REPLACE "(.*)tex2lyx${_suffix}.*" "\\1" _bindir "${TEX2LYX_EXE}")

  message(STATUS "executing ${LYX_PYTHON_EXECUTABLE} -tt \"${SCRIPT_DIR}/../configure.py\" ${_with_ver} \"--binary-dir=${_bindir}\"")
  execute_process(COMMAND ${LYX_PYTHON_EXECUTABLE} -tt "${SCRIPT_DIR}/../configure.py"
    ${_with_ver} "--binary-dir=${_bindir}"
    WORKING_DIRECTORY "${LYX_TESTS_USERDIR}"
  )
endif()

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


