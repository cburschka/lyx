# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2013 Kornel Benko kornel@lyx.org
#
#
# LYX2LYX  = ${TOP_SRC_DIR}/lib/lyx2lyx/lyx2lyx
# LYXFILE  = xxx
#
# Script should be called like:
# cmake -DLYX2LYX=xxx \
#       -DWORKDIR=${BUILD_DIR}/autotests/out-home \
#       -DLYXFILE=xxx \
#       -P "${TOP_SRC_DIR}/development/autotests/lyx2lyxtest.cmake"
#

message(STATUS "Executing ${LYX2LYX} -e errors -o output ${LYXFILE}")
execute_process(
  COMMAND ${LYX2LYX} -e errors -o output ${LYXFILE}
  RESULT_VARIABLE _err)

message(STATUS "Error output of lyx2lyx = ${_err}")
string(COMPARE NOTEQUAL  ${_err} 0 _erg)

# Check file "errors" being empty
file(STRINGS "errors" foundErrors)
if(foundErrors)
  set(_erg 1)
  foreach(_l ${foundErrors})
    message(STATUS "${_l}")
  endforeach()
endif()

# Check file "output" being not empty
file(STRINGS "output" createdLyx)
if(NOT createdLyx)
  set(_erg 1)
  message(STATUS "Created file empty")
endif()

if(_erg)
  message(FATAL_ERROR "lyx2lyx failed")
endif()
