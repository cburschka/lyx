# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko kornel@lyx.org
#
#
# LYX_ROOT  = ${TOP_SRC_DIR}/lib/doc
# lyx       = 
# format    = lyx16x|xhtml
# extension = 16.lyx|xhtml
# file      = xxx
#
# Script should be called like:
# cmake -DLYX_ROOT=xxx \
#       -Dlyx=xxx \
#       -Dformat=xxx \
#       -Dextension=xxx \
#       -Dfile=xxx \
#       -P "${TOP_SRC_DIR}/development/autotests/export.cmake"
#

message(STATUS "Executing ${lyx} -E ${format} ${file}.${extension} ${LYX_ROOT}/${file}.lyx")
execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${file}.${extension})
execute_process(
  COMMAND ${lyx} -E ${format} ${file}.${extension} "${LYX_ROOT}/${file}.lyx"
  RESULT_VARIABLE _err)
string(COMPARE NOTEQUAL  ${_err} 0 _erg)
if(_erg)
  message(STATUS "Exporting ${f}.lyx to ${format}")
  message(FATAL_ERROR "Export failed")
endif()
