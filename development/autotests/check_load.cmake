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
include(${PARAMS_DIR}/CheckLoadErrors.cmake)
CheckLoadErrors(lyxerr "${PARAMS_DIR}" _erg)

if(_erg)
  # We print here the whole error output, even the ignored part
  message(FATAL_ERROR "lyx gave warnings/errors:\n${lyxerr}")
endif()
