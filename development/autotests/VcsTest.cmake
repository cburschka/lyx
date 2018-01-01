# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2018 Kornel Benko kornel@lyx.org
#
#
# LYX_ROOT  = ${TOP_SRC_DIR}/lib/{doc,examples,templates}
# file      = xxx
#
set(ENV{${LYX_USERDIR_VER}} "${LYX_TESTS_USERDIR}")
set(ENV{LANG} "en_US.UTF-8") # to get all error-messages in english

set(result_file_name ${file}.tex)
set(check_file_name "${LYX_ROOT}/${file}.tex")

message(STATUS "Check for ${LYX_GITVERSION}")
if(LYX_GITVERSION)
  execute_process(
    WORKING_DIRECTORY "${LYX_ROOT}"
    COMMAND git status ${file}.lyx
    RESULT_VARIABLE _err
    ERROR_VARIABLE lyxerr)
  if (_err)
    message(STATUS "Not a git controlled file")
  else()
    message(STATUS "Executing ${lyx} -E ${ExportFormat} ${result_file_name} \"${LYX_ROOT}/${file}.lyx\"")
    execute_process(
      COMMAND ${lyx} -E ${ExportFormat} ${result_file_name} "${LYX_ROOT}/${file}.lyx"
      RESULT_VARIABLE _err
      ERROR_VARIABLE lyxerr)
      if(NOT _err)
	if(NOT EXISTS "${result_file_name}")
	  message(STATUS "Expected result file \"${result_file_name}\" does not exist")
	  set(_err -1)
	else()
	  message(STATUS "Expected result file \"${result_file_name}\" exists")
	endif()
	# Check if result file identical to check file
	execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files
	  "${result_file_name}" "${LYX_ROOT}/${file}.tex"
	  RESULT_VARIABLE _err
	  ERROR_VARIABLE lyxerr)
	if (NOT _err)
	  message(STATUS "Compare successful")
	endif()
      endif()

    string(COMPARE NOTEQUAL  ${_err} 0 _erg)
    if(_erg)
      message(STATUS "Exporting ${file}.lyx to ${ExportFormat}")
      message(FATAL_ERROR "Export failed")
    endif()
  endif()
endif()
