# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko kornel@lyx.org
#
#
# LYX_ROOT  = ${TOP_SRC_DIR}/lib/{doc,examples,templates}
# LYX_USERDIR_VER = Name of environment variable for the user directory
# lyx       =
# format    = lyx16x|lyx21x|xhtml
# extension = 16.lyx|21.lyx|xhtml
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
#       -DLYX_FORMAT_NUM=${_lyx_format_num} \
#       -Dfile=xxx \
#       -Dinverted=[01] \
#       -DTOP_SRC_DIR=${TOP_SRC_DIR}
#       -DPERL_EXECUTABLE=${PERL_EXECUTABLE}
#       -P "${TOP_SRC_DIR}/development/autotests/export.cmake"
#

set(Perl_Script "${TOP_SRC_DIR}/development/autotests/useSystemFonts.pl")
set(Structure_Script "${TOP_SRC_DIR}/development/autotests/beginEndStructureCheck.pl")
set(LanguageFile "${TOP_SRC_DIR}/lib/languages")
set(GetTempDir "${TOP_SRC_DIR}/development/autotests/getTempDir.pl")
set(_ft ${fonttype})
execute_process(COMMAND ${PERL_EXECUTABLE} "${GetTempDir}" "${WORKDIR}" OUTPUT_VARIABLE TempDir)
message(STATUS "using fonttype = ${_ft}")
if(format MATCHES "dvi|pdf")
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
  set(result_file_name ${file}_${_ft}.${extension})
else()
  message(STATUS "Not converting")
  set(LYX_SOURCE "${LYX_ROOT}/${file}.lyx")
  if(extension MATCHES "\\.lyx$")
    # Font-type not relevant for lyx16/lyx21 exports
    set(result_file_base ${file})
  else()
    set(result_file_name ${file}.${extension})
  endif()
endif()

set(ENV{${LYX_USERDIR_VER}} "${LYX_TESTS_USERDIR}")
if (extension MATCHES "\\.lyx$")
  set(ENV{${LYX_USERDIR_VER}} "${LYX_TESTS_USERDIR}")
  set(ENV{LANG} "en") # to get all error-messages in english

  include(${TOP_SRC_DIR}/development/autotests/CheckLoadErrors.cmake)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E md5sum "${LYX_SOURCE}"
    OUTPUT_VARIABLE source_md5sum_x
    RESULT_VARIABLE _err
    ERROR_VARIABLE lyxerr)
  string(REGEX REPLACE " .*" "" source_md5sum ${source_md5sum_x})
  message(STATUS "MD5SUM of \"${LYX_SOURCE}\" is ${source_md5sum}")
  foreach(_lv RANGE 1 5)
    set(result_file_base "${result_file_base}.${LYX_FORMAT_NUM}")
    set(result_file_name "${result_file_base}.lyx")
    file(REMOVE "${result_file_name}" "${result_file_name}.emergency" )
    message(STATUS "Executing ${lyx} -userdir \"${LYX_TESTS_USERDIR}\" -E ${format} ${result_file_name} \"${LYX_SOURCE}\"")
    execute_process(
      COMMAND ${lyx} -userdir "${LYX_TESTS_USERDIR}" -E ${format} ${result_file_name} "${LYX_SOURCE}"
      RESULT_VARIABLE _err
      ERROR_VARIABLE lyxerr)
    if(_err)
      break()
    else()
      if (NOT EXISTS "${result_file_name}")
        message(STATUS "Expected result file \"${result_file_name}\" does not exist")
        set(_err -1)
        break()
      else()
        message(STATUS "Expected result file \"${result_file_name}\" exists")
        checkLoadErrors(lyxerr "${TOP_SRC_DIR}/development/autotests" _err)
        if(_err)
          break()
        endif()
      endif()
    endif()
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E md5sum ${result_file_name}
      OUTPUT_VARIABLE result_md5sum_x
      RESULT_VARIABLE _err
      ERROR_VARIABLE lyxerr)
    string(REGEX REPLACE " .*" "" result_md5sum ${result_md5sum_x})
    message(STATUS "MD5SUM of \"${result_file_name}\" is ${result_md5sum}")
    if(_err)
      break()
    endif()
    message(STATUS "check structures of ${result_file_name}")
    execute_process(
      COMMAND ${PERL_EXECUTABLE} ${Structure_Script} "${WORKDIR}/${result_file_name}"
      RESULT_VARIABLE _err
      ERROR_VARIABLE lyxerr)
    if(_err)
      break()
    endif()
    message(STATUS "check load of ${result_file_name}")
    execute_process(
      COMMAND ${lyx} -batch -userdir "${LYX_TESTS_USERDIR}" ${result_file_name}
      RESULT_VARIABLE _err
      ERROR_VARIABLE lyxerr)
    if(_err)
      break()
    endif()
    checkLoadErrors(lyxerr "${TOP_SRC_DIR}/development/autotests" _err)
    if(_err)
      break()
    endif()
    # Check if result file identical to source file
    if(result_md5sum STREQUAL ${source_md5sum})
      message(STATUS "Source(${LYX_SOURCE}) and dest(${result_file_name}) are equal")
      break()
    endif()
    set(source_md5sum ${result_md5sum})
    set(LYX_SOURCE ${result_file_name})
  endforeach()
else()
  message(STATUS "Executing ${lyx} -userdir \"${LYX_TESTS_USERDIR}\" -E ${format} ${result_file_name} \"${LYX_SOURCE}\"")
  file(REMOVE ${result_file_name})
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
endif()

if(inverted)
  string(COMPARE EQUAL  ${_err} 0 _erg)
else()
  string(COMPARE NOTEQUAL  ${_err} 0 _erg)
endif()
execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory "${TempDir}")
if(_erg)
  message(STATUS "Exporting ${file}.lyx to ${format}")
  message(FATAL_ERROR "Export failed")
endif()
