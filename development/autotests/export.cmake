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
#       -DLYX_TESTS_USERDIR=${LYX_TESTS_USERDIR} \
#       -DWORKDIR=${BUILD_DIR}/autotests/out-home \
#       -DLYX_USERDIR_VER=${LYX_USERDIR_VER} \
#       -Dlyx=xxx \
#       -Dformat=xxx \
#       -Dextension=xxx \
#       -Dfile=xxx \
#       -Dreverted=[01] \
#       -DTOP_SRC_DIR=${TOP_SRC_DIR}
#       -DPERL_EXECUTABLE=${PERL_EXECUTABLE}
#       -P "${TOP_SRC_DIR}/development/autotests/export.cmake"
#

set(Perl_Script "${TOP_SRC_DIR}/development/autotests/useSystemFonts.pl")
if(format MATCHES "dvi3|pdf4|pdf5")
  message(STATUS "LYX_TESTS_USERDIR = ${LYX_TESTS_USERDIR}")
  message(STATUS "Converting with perl ${Perl_Script}")
  set(LYX_SOURCE "${WORKDIR}/${file}_${format}.lyx")
  message(STATUS "Using source \"${LYX_ROOT}/${file}.lyx\"")
  message(STATUS "Using dest \"${LYX_SOURCE}\"")
  execute_process(COMMAND ${PERL_EXECUTABLE} "${Perl_Script}" "${LYX_ROOT}/${file}.lyx" "${LYX_SOURCE}" ${format}
    RESULT_VARIABLE _err)
  string(COMPARE EQUAL  ${_err} 0 _erg)
  if(NOT _erg)
    message(FATAL_ERROR "Export failed while converting")
  endif()
else()
  message(STATUS "Not converting")
  set(LYX_SOURCE "${LYX_ROOT}/${file}.lyx")
endif()

message(STATUS "Executing ${lyx} -userdir \"${LYX_TESTS_USERDIR}\" -E ${format} ${file}.${extension} \"${LYX_SOURCE}\"")
set(ENV{${LYX_USERDIR_VER}} "${LYX_TESTS_USERDIR}")
execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${file}.${extension})
execute_process(
  COMMAND ${lyx} -userdir "${LYX_TESTS_USERDIR}" -E ${format} ${file}.${extension} "${LYX_SOURCE}"
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
