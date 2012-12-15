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

set(LYX_EXE "${BINDIR}/lyx")
set(XVKBD_EXE "${BINDIR}/xvkbd")

# Environments needed by keytest.py
set(ENV{PACKAGE} ${PACKAGE})
set(ENV{LOCALE_DIR} ${LOCALE_DIR})
set(ENV{LYX_LOCALEDIR} "${WORKDIR}/../locale")
set(ENV{LYX_USERDIR} ${WORKDIR})
set(ENV{LYX_PID} ${pidres})
set(ENV{LYX_WINDOW_NAME} ${LYX_WINDOW_NAME})
set(ENV{LYX_EXE} ${LYX_EXE})
set(ENV{XVKBD_EXE} ${XVKBD_EXE})
set(ENV{KEYTEST_INFILE} "${AUTOTEST_ROOT}/${KEYTEST_INFILE}")
set(ENV{KEYTEST_OUTFILE} "${WORKDIR}/${KEYTEST_OUTFILE}")
set(ENV{PO_BUILD_DIR} "${PO_BUILD_DIR}")
set(ENV{MAX_DROP} 0)
set(ENV{MAX_LOOPS} 1)
file(GLOB _testfiles RELATIVE "${WORKDIR}" "test*.*" "#test*.*")
if(_testfiles)
  # remove some leftover files from previous test
  execute_process(
  COMMAND ${CMAKE_COMMAND} -E remove -f ${_testfiles} )
endif()
execute_process(
  COMMAND python ${KEYTEST}
  RESULT_VARIABLE KEYTEST_RES)
if(KEYTEST_RES)
  message(FATAL_ERROR "KEYTEST failed: KEYTEST_RES=${KEYTEST_RES}")
endif()

