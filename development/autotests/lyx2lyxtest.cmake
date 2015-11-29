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
# cmake -DLYX_PYTHON_EXECUTABLE=xxx \
#       -DLYX2LYX=xxx \
#       -DLYX_TESTS_USERDIR=${LYX_TESTS_USERDIR} \
#       -DWORKDIR=${BUILD_DIR}/autotests/out-home \
#       -DLYXFILE=xxx \
#       -P "${TOP_SRC_DIR}/development/autotests/lyx2lyxtest.cmake"
#

string(RANDOM LENGTH 5 errorfile)
string(RANDOM LENGTH 6 outputfile)
message(STATUS "Executing ${LYX_PYTHON_EXECUTABLE} ${LYX2LYX} -e ${errorfile} -o ${outputfile} ${LYXFILE}")
execute_process(
  COMMAND ${LYX_PYTHON_EXECUTABLE} ${LYX2LYX} -e ${errorfile} -o ${outputfile} ${LYXFILE}
  RESULT_VARIABLE _err)

message(STATUS "Error output of lyx2lyx = ${_err}")
string(COMPARE NOTEQUAL  ${_err} 0 _erg)

# Check file "errors" being empty
file(STRINGS ${errorfile} foundErrors)
if(foundErrors)
  foreach(_l ${foundErrors})
    if (NOT _l MATCHES "Warning: No conversion needed:")
      set(_erg 1)
      message(STATUS "${_l}")
  endif()
  endforeach()
endif()

# Check the output-file being not empty
file(STRINGS ${outputfile} createdLyx)
if(NOT createdLyx)
  set(_erg 1)
  message(STATUS "Created file empty")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${errorfile} ${outputfile})
if(_erg)
  message(FATAL_ERROR "lyx2lyx failed")
endif()
