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
#       -DLYX_TESTS_USERDIR=${LYX_TESTS_USERDIR} \
#       -DLYXFILE=xxx \
#       -DLYX_USERDIR_VER=${LYX_USERDIR_VER} \
#       -DPARAMS_DIR="${TOP_SRC_DIR}/development/autotests" \
#       -P "${TOP_SRC_DIR}/development/autotests/check_load.cmake"
#

message(STATUS "Executing ${lyx} -batch -userdir \"${LYX_TESTS_USERDIR}\" ${LYXFILE}")
set(ENV{${LYX_USERDIR_VER}} "${LYX_TESTS_USERDIR}")
set(ENV{LANG} "en") # to get all error-messages in english

execute_process(
  COMMAND ${lyx} -batch -userdir "${LYX_TESTS_USERDIR}" ${LYXFILE}
  RESULT_VARIABLE _err
  ERROR_VARIABLE lyxerr)

message(STATUS "Error code of lyx = ${_err}")
string(COMPARE NOTEQUAL  ${_err} 0 _erg)

#
# Ignore messages between "reconfiguring user directory" and "LyX: Done!"
# (Reconfigure-messages are not symptom of an error)
if(lyxerr)
  set(ConfigureOutput 0)
  file(STRINGS "${PARAMS_DIR}/filterCheckWarnings" ignoreRegexp)
  # Split lyxerr into lines
  string(REGEX REPLACE "[\n]+" ";" foundErrors ${lyxerr})
  foreach(_l ${foundErrors})
    if(ConfigureOutput)
      if(_l MATCHES "LyX: Done!")
        set(ConfigureOutput 0)
      endif()
    else()
      if(_l MATCHES "reconfiguring user directory")
        set(ConfigureOutput 1)
      endif()
    endif()
    if(NOT ConfigureOutput)
      set(found 0)
      foreach(_r ${ignoreRegexp})
        if(_l MATCHES "${_r}")
          set(found 1)
          break()
        endif()
      endforeach()
      if(NOT found)
        message(STATUS "Error line = ${_l}")
        # It is error, because the error-line does not match
        # any ignoring expression
        set(_erg 1)
        break()
      endif()
    endif()
  endforeach()
endif()

if(_erg)
  # We print here the whole error output, even the ignored part
  message(FATAL_ERROR "lyx gave warnings/errors:\n${lyxerr}")
endif()
