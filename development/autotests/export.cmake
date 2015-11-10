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
#       -Dfonttype=xxx \
#       -Dextension=xxx \
#       -Dfile=xxx \
#       -Dreverted=[01] \
#       -DTOP_SRC_DIR=${TOP_SRC_DIR}
#       -DPERL_EXECUTABLE=${PERL_EXECUTABLE}
#       -P "${TOP_SRC_DIR}/development/autotests/export.cmake"
#

set(Perl_Script "${TOP_SRC_DIR}/development/autotests/useSystemFonts.pl")
set(LanguageFile "${TOP_SRC_DIR}/lib/languages")
set(GetTempDir "${TOP_SRC_DIR}/development/autotests/getTempDir.pl")
set(_ft ${fonttype})
execute_process(COMMAND ${PERL_EXECUTABLE} "${GetTempDir}" "${WORKDIR}" OUTPUT_VARIABLE TempDir)
message(STATUS "using fonttype = ${_ft}")
if(format MATCHES "dvi3|pdf4|pdf5")
  message(STATUS "LYX_TESTS_USERDIR = ${LYX_TESTS_USERDIR}")
  message(STATUS "Converting with perl ${Perl_Script}")
  set(LYX_SOURCE "${TempDir}/${file}_${format}_${_ft}.lyx")
  message(STATUS "Using source \"${LYX_ROOT}/${file}.lyx\"")
  message(STATUS "Using dest \"${LYX_SOURCE}\"")
  execute_process(COMMAND ${PERL_EXECUTABLE} "${Perl_Script}" "${LYX_ROOT}/${file}.lyx" "${LYX_SOURCE}" ${format} ${_ft} ${LanguageFile}
    RESULT_VARIABLE _err)
  string(COMPARE EQUAL  ${_err} 0 _erg)
  if(NOT _erg)
    message(FATAL_ERROR "Export failed while converting")
  endif()
else()
  message(STATUS "Not converting")
  set(LYX_SOURCE "${LYX_ROOT}/${file}.lyx")
endif()

set(result_file_name ${file}_${_ft}.${extension})
message(STATUS "Executing ${lyx} -userdir \"${LYX_TESTS_USERDIR}\" -E ${format} ${result_file_name} \"${LYX_SOURCE}\"")
set(ENV{${LYX_USERDIR_VER}} "${LYX_TESTS_USERDIR}")
execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${result_file_name})
execute_process(
  COMMAND ${lyx} -userdir "${LYX_TESTS_USERDIR}" -E ${format} ${result_file_name} "${LYX_SOURCE}"
  RESULT_VARIABLE _err)

#check if result file created
if (NOT _err)
  if (NOT EXISTS "${result_file_name}")
    message(STATUS "Expected result file \"${result_file_name}\" does not exist")
    set(_err -1)
  else()
    message(STATUS "Expected result file \"${result_file_name}\" exists")
  endif()
endif()

if(reverted)
  string(COMPARE EQUAL  ${_err} 0 _erg)
else()
  string(COMPARE NOTEQUAL  ${_err} 0 _erg)
endif()
execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory "${TempDir}")
if(_erg)
  message(STATUS "Exporting ${f}.lyx to ${format}")
  message(FATAL_ERROR "Export failed")
endif()
