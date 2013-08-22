# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2013 Kornel Benko kornel@lyx.org
# Copyright (c) 2013 Scott Kostyshak skotysh@lyx.org
#
#
# LYXFILE  = xxx
# lyx      =
#
# Script should be called like:
# cmake -DWORKDIR=${BUILD_DIR}/autotests/out-home \
#       -DLYXFILE=xxx \
#       -DLYX_USERDIR_VER=${LYX_USERDIR_VER} \
#       -DPARAMS_DIR="${TOP_SRC_DIR}/development/autotests" \
#       -P "${TOP_SRC_DIR}/development/autotests/check_load.cmake"
#

message(STATUS "Executing ${lyx} -batch ${LYXFILE}")
set(ENV{${LYX_USERDIR_VER}} "${WORKDIR}/.lyx")
set(ENV{LANG} "en") # to get all error-messages in english

execute_process(
  COMMAND ${lyx} -batch -userdir ${WORKDIR}/.lyx ${LYXFILE}
  RESULT_VARIABLE _err
  ERROR_VARIABLE lyxerr)

message(STATUS "Error code of lyx = ${_err}")
string(COMPARE NOTEQUAL  ${_err} 0 _erg)

if(lyxerr)
  file(STRINGS "${PARAMS_DIR}/filterCheckWarnings" ignoreRegexp)
  # Split lyxerr into lines
  string(REGEX REPLACE "[\n]+" ";" foundErrors ${lyxerr})
  foreach(_l ${foundErrors})
    message(STATUS "Checking line: \"${_l}\"")
    set(found 0)
    foreach(_r ${ignoreRegexp})
      string(REGEX MATCHALL "${_r}" _match ${_l})
      if(_match)
        set(found 1)
        break()
      endif()
    endforeach()
    if(NOT found)
      # It is error, because the error-line does not match
      # any ignoring expression
      set(_erg 1)
    endif()
  endforeach()
endif()

if(_erg)
  # We print here the whole error output, even the ignored part
  message(FATAL_ERROR "lyx gave warnings/errors:\n${lyxerr}")
endif()
