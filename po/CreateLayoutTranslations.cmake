# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2016 Kornel Benko kornel@lyx.org
#
# Script should be called like:
# cmake -DTOP_SRC_DIR=xxx \
#       -DDestFile=xxx \
#       -DLYX_PYTHON_EXECUTABLE=xxx \
#       -DParType=xxx \
#       -DSources=xxx \
#       -P "${TOP_SRC_DIR}/po/CreateLayoutTranslations.cmake"

set(_dst ${DestFile})
set(type ${ParType})
set(_src_files ${Sources})

macro(die_if _err text)
  string(COMPARE EQUAL  ${${_err}} 0 _erg)
  if(NOT _erg)
    message(FATAL_ERROR "CreateLayoutTranslations: ${text}")
  endif()
endmacro()

message(STATUS "Starting CreateLayoutTranslations")
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TOP_SRC_DIR}/lib/layouttranslations" "${_dst}"
  RESULT_VARIABLE _err)
die_if(_err "Copy of layouttranslations failed")

execute_process(COMMAND ${LYX_PYTHON_EXECUTABLE}
  "${TOP_SRC_DIR}/po/lyx_pot.py" -b "${TOP_SRC_DIR}" -o "${_dst}" -t ${type}
  "--src_file=${_src_files}"
  RESULT_VARIABLE _err)
die_if(_err "Calling lyx_pot.py failed")

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_dst}" "${TOP_SRC_DIR}/lib/layouttranslations"
  RESULT_VARIABLE _err)
die_if(_err "Copy of layouttranslations to source failed")

