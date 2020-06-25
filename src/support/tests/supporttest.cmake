# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko kornel@lyx.org
#
#
# Script should be called like:
# cmake -DCommand=xxx \
#       -DInput=xxx \
#       -DOutput=xxx \
#       -P xxxx/supporttest.cmake
#
# 
# Command: The executable, whose output is to be examined
# Input: The file name of expected data
# Output: The file name for data caught from output of the executable

execute_process(COMMAND "${Command}"
  OUTPUT_FILE "${Output}"
  RESULT_VARIABLE _testres)

if(_testres)
  message(FATAL_ERROR "${Command} not working")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files "${Input}" "${Output}"
                RESULT_VARIABLE _testres)
if(_testres)
  message(FATAL_ERROR "Files are not eqal")
endif()
