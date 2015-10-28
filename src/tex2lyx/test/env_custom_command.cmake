# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2015 Kornel Benko <kornel@lyx.org>
#
# Environ and parameter strings are separated by "'"
# Script should be called like:
#   add_custom_command(
#     COMMAND ${CMAKE_COMMAND}
#     -DENVIRON="${LYX_USERDIR_VER}=${LYX_TESTS_USERDIR}" \
#     -DCOMMAND="${LYX_PYTHON_EXECUTABLE}" \
#     -DPARAMS="${runtestsScript}'$<TARGET_FILE:${_tex2lyx}>'${scriptDir}'${CMAKE_CURRENT_SOURCE_DIR}"
#     -P ${TOP_SRC_DIR}/src/tex2lyx/test/env_custom_command.cmake
#     WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
#     DEPENDS ...
#     )

string(REGEX MATCHALL "[^']+" ENVIRON_LIST "${ENVIRON}")
foreach(_env ${ENVIRON_LIST})
  if (_env MATCHES "^\([^=]+\)=\(.*\)$")
    set(ENV{${CMAKE_MATCH_1}} ${CMAKE_MATCH_2})
    #message(STATUS "setting env ${CMAKE_MATCH_1}=${CMAKE_MATCH_2}")
  else()
    message(FATAL_ERROR "Wrong environment string \"${_env}\"")
  endif()
endforeach()

string(REGEX MATCHALL "[^']+" parameters "${PARAMS}")
execute_process(COMMAND "${COMMAND}" ${parameters}
  RESULT_VARIABLE _err
  OUTPUT_VARIABLE _out)

message(STATUS ${_out})
#message(STATUS "Error output of \"${COMMAND}\" " ${parameters} " = ${_err}")
string(COMPARE NOTEQUAL  ${_err} 0 _erg)

if(_erg)
  message(FATAL_ERROR "\"${COMMAND}\" ${parameters} failed")
endif()

