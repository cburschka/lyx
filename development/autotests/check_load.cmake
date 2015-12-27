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
#       -Dinverted=${inverted}
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

#
# Ignore messages between "reconfiguring user directory" and "LyX: Done!"
# (Reconfigure-messages are not symptom of an error)
include(${PARAMS_DIR}/CheckLoadErrors.cmake)
if (NOT _err)
  CheckLoadErrors(lyxerr "${PARAMS_DIR}" _err)
endif()

if(inverted)
  string(COMPARE EQUAL  ${_err} 0 _erg)
else()
  string(COMPARE NOTEQUAL  ${_err} 0 _erg)
endif()
if(_erg)
  # We print here the whole error output, even the ignored part
  message(FATAL_ERROR "lyx gave warnings/errors:\n${lyxerr}")
endif()
