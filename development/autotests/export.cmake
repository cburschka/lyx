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
#       -DXMLLINT_EXECUTABLE=${XMLLINT_EXECUTABLE}
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

function(get_md5sum msource mresult mreserr)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E md5sum ${${msource}}
    OUTPUT_VARIABLE msource_md5sum_x
    RESULT_VARIABLE mres_err)
  if (NOT mres_err)
    string(REGEX REPLACE " .*" "" msource_md5sum ${msource_md5sum_x})
    set(${mresult} ${msource_md5sum} PARENT_SCOPE)
    message(STATUS "MD5SUM of \"${${msource}}\" is ${msource_md5sum}")
  else()
    set(${mresult} "xx" PARENT_SCOPE)
    message(STATUS "Error getting MD5SUM of \"${${msource}}\"")
  endif()
  set(${mreserr} ${mres_err} PARENT_SCOPE)
endfunction()

set(ENV{${LYX_USERDIR_VER}} "${LYX_TESTS_USERDIR}")
set(ENV{LANG} "en") # to get all error-messages in english
set(ENV{LC_ALL} "C")
if (extension MATCHES "\\.lyx$")
  include(${TOP_SRC_DIR}/development/autotests/CheckLoadErrors.cmake)
  get_md5sum(LYX_SOURCE source_md5sum _err)
  foreach(_lv RANGE 1 5)
    set(result_file_base "${result_file_base}.${LYX_FORMAT_NUM}")
    set(result_file_name "${result_file_base}.lyx")
    message(STATUS "check structures of ${LYX_SOURCE}")
    execute_process(
      COMMAND ${PERL_EXECUTABLE} ${Structure_Script} "${WORKDIR}/${result_file_name}"
      RESULT_VARIABLE _err)
    if(_err)
      break()
    endif()
    file(REMOVE "${result_file_name}" "${result_file_name}.emergency" )
    message(STATUS "Executing ${lyx} -userdir \"${LYX_TESTS_USERDIR}\" -E ${format} ${result_file_name} \"${LYX_SOURCE}\"")
    message(STATUS "This implicitly checks load of ${LYX_SOURCE}")
    execute_process(
      COMMAND ${lyx} -userdir "${LYX_TESTS_USERDIR}" -E ${format} ${result_file_name} "${LYX_SOURCE}"
      RESULT_VARIABLE _err
      ERROR_VARIABLE lyxerr)
    if(_err)
      break()
    elseif(NOT EXISTS "${result_file_name}")
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
    get_md5sum(result_file_name result_md5sum _err)
    if(_err)
      # Somehow the created file is not readable?
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
      if (format MATCHES "xhtml")
        if (XMLLINT_EXECUTABLE)
          # check the created xhtml file
          execute_process(
            COMMAND ${XMLLINT_EXECUTABLE} --sax --html --valid  "${result_file_name}"
            OUTPUT_VARIABLE xmlout
            ERROR_VARIABLE xmlerr
            RESULT_VARIABLE _err)
          file(WRITE "${result_file_name}.sax_out" ${xmlout})
          if (NOT _err)
            # check if sax-parser output contains error messages
            execute_process(
              COMMAND ${PERL_EXECUTABLE} "${TOP_SRC_DIR}/development/autotests/examineXmllintOutput.pl" "${result_file_name}.sax_out"
              OUTPUT_VARIABLE xmlout
              RESULT_VARIABLE _err)
          endif()
          if (NOT _err)
            if (NOT "${xmlout}" STREQUAL "")
              message(STATUS "${xmlout}")
              set(_err -1)
            endif()
          endif()
        endif()
      endif()
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
