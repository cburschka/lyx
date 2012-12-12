# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko kornel@lyx.org
#
#
# LYX_ROOT  = ${TOP_SRC_DIR}
# lyx       = ${TOP_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/lyx
#
# Script should be called like:
# cmake -DLYX_ROOT=xxx \
#       -Dlyx=xxx \
#       -P "${TOP_SRC_DIR}/development/autotests/export.cmake"
#
file(GLOB lyx_files "${LYX_ROOT}/lib/doc/*.lyx")
message(STATUS "lyx_files = ${lyx_files}")

set(ERRORS)
foreach(format lyx16x xhtml)
  foreach(f ${lyx_files})
    message(STATUS "Executing ${lyx} -E ${format} localtest.${format} ${f}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove localtest.xhtml localtest.16.lyx)
    execute_process(
      COMMAND ${lyx} -E ${format} localtest.${format} ${f}
      RESULT_VARIABLE _err)
    string(COMPARE NOTEQUAL  ${_err} 0 _erg)
    if(_erg)
      list(APPEND ERRORS "Exporting ${f} to ${format}")
    endif()
  endforeach()
endforeach()
if(ERRORS)
  foreach(m ${ERRORS})
    message(STATUS ${m})
  endforeach()
  message(FATAL_ERROR "Export failed")
endif()
