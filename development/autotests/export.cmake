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
if(erg)
  message(STATUS "Exporting ${f}.lyx to ${format}")
  message(FATAL_ERROR "Export failed")
endif()
# This script invokes the keytest.py script with the simple set-up needed
# to re-run deterministic regression tests that one would like to have.
#
# AUTOTEST_ROOT   = ${LYX_ROOT}/development/autotests
# KEYTEST_INFILE  = xxx-in.txt
# KEYTEST_OUTFILE = xxx-out.txt
# BINDIR          = ${BUILD_DIR}/bin
# WORKDIR         = ${BUILD_DIR}/autotests/out-home
# LOCALE_DIR      = ${BUILD_DIR}/autotests/locale
# PO_BUILD_DIR    = ${BUILD_DIR}/po
# PACKAGE         = lyx2.1
#
# Script should be called like:
# cmake -DAUTOTEST_ROOT=xxxx \
#       -DKEYTEST_INFILE=xxxx \
#       -DKEYTEST_OUTFILE=xxx \
#       -DBINDIR=xxx \
#       -DWORKDIR=xxx \
#       -DLOCALE_DIR=xxx \
#       -DPO_BUILD_DIR=xxx \
#       -DPACKAGE=xxx \
#       -P ${AUTOTEST_ROOT}/single-test.cmake

set(KEYTEST "${AUTOTEST_ROOT}/keytest.py")

execute_process(COMMAND pidof lyx OUTPUT_VARIABLE LYX_PID RESULT_VARIABLE pidstat OUTPUT_VARIABLE pidres)
message(STATUS "pidres = ${pidres}")
if (NOT pidstat)
  # lyx already running, remove trailing '\n' from pid
  string(REGEX REPLACE "\n" "" pidres ${pidres})
  execute_process(COMMAND wmctrl -l -p OUTPUT_VARIABLE _wmco)
  string(REGEX REPLACE "[\n]+" ";" _wmc ${_wmco})
  foreach(_w ${_wmc})
    string(REGEX MATCH "${pidres}" _wr ${_w})
    if (${_wr} MATCHES ${pidres})
      # this entry contains the pid, go search for X11-window-id
      string(REGEX REPLACE " .*" "" _wr ${_w})
      set(LYX_WINDOW_NAME ${_wr})
      message(STATUS "Set LYX_WINDOW_NAME to ${_wr}")
    endif()
  endforeach()
else()
  set(pidres "")
  set(LYX_WINDOW_NAME "")
endif()
