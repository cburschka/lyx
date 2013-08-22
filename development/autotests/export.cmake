# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko kornel@lyx.org
#
#
# LYX_ROOT  = ${TOP_SRC_DIR}/lib/{doc,examples,templates}
# LYX_USERDIR_VER = Name of environment variable for the user directory
# lyx       = 
# format    = lyx16x|xhtml
# extension = 16.lyx|xhtml
# file      = xxx
#
# Script should be called like:
# cmake -DLYX_ROOT=xxx \
#       -DWORKDIR=${BUILD_DIR}/autotests/out-home \
#       -DLYX_USERDIR_VER=${LYX_USERDIR_VER} \
#       -Dlyx=xxx \
#       -Dformat=xxx \
#       -Dextension=xxx \
#       -Dfile=xxx \
#       -Dreverted=[01] \
#       -P "${TOP_SRC_DIR}/development/autotests/export.cmake"
#

message(STATUS "Executing ${lyx} -userdir \"${WORKDIR}/.lyx\" -E ${format} ${file}.${extension} \"${LYX_ROOT}/${file}.lyx\"")
set(ENV{${LYX_USERDIR_VER}} "${WORKDIR}/.lyx")
execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${file}.${extension})
execute_process(
  COMMAND ${lyx} -userdir "${WORKDIR}/.lyx" -E ${format} ${file}.${extension} "${LYX_ROOT}/${file}.lyx"
  RESULT_VARIABLE _err)
if(reverted)
  string(COMPARE EQUAL  ${_err} 0 _erg)
else()
  string(COMPARE NOTEQUAL  ${_err} 0 _erg)
endif()
if(_erg)
  message(STATUS "Exporting ${f}.lyx to ${format}")
  message(FATAL_ERROR "Export failed")
endif()
