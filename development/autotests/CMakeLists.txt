# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko kornel@lyx.org
#

set(LYX_HOME "out-home")
set(LOCALE_DIR "${CMAKE_CURRENT_BINARY_DIR}/locale")
file(MAKE_DIRECTORY "${LOCALE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}")

if(QT_USES_X11)
  set(Missing)
  set(XVFBDLIBS)
  # Make sure, the needed programs are in PATH
  find_program(PCREGREP_EXE "pcregrep")
  if (NOT PCREGREP_EXE)
    list(APPEND Missing "pcregrep")
  endif()
  find_program(WMCTRL_EXE "wmctrl")
  if(NOT WMCTRL_EXE)
    list(APPEND Missing "wmctrl")
  endif()
  # This is needed to build xvkbd
  # Programs pcregrep wmctrl and xvkbd are used in subsequent scripts
  # while testing

  # Check for needed libraries for xvkbd
  # do not add autotests, if any of them is lacking

  find_package(X11)
  if(NOT X11_FOUND)
    list(APPEND Missing "X11")
  endif()
  find_package(PkgConfig)
  if(PKG_CONFIG_FOUND)
    foreach(_lb "xaw7" "xmu" "xtst")
      pkg_check_modules(${_lb}LIB ${_lb})
      if(${_lb}LIB_LIBRARIES)
        list(APPEND XVFBDLIBS ${${_lb}LIB_LIBRARIES})
      else()
        list(APPEND Missing ${_lb})
      endif()
    endforeach()
  else()
    foreach(_lb "Xaw7" "Xmu")
      find_library(${_lb}LIB ${_lb})
      if(${_lb}LIB)
        list(APPEND XVFBDLIBS ${${_lb}LIB})
      else()
        list(APPEND Missing ${_lb})
      endif()
    endforeach()
    foreach(_lb Xt XTest X11)
      if(X11_${_lb}_LIB)
        list(APPEND XVFBDLIBS ${X11_${_lb}_LIB})
      else()
        list(APPEND Missing ${_lb})
      endif()
    endforeach()
  endif()
  if (XVFBDLIBS)
    list(REMOVE_DUPLICATES XVFBDLIBS)
  endif()

  if(Missing)
    message(STATUS "Missing Libraries or programs to create xvkbd: ${Missing}")
    message(STATUS "cmake build is therefore omitting keytests")
  endif()

  if(Missing)
    set(_runtest FALSE)
  else()
    project(keytest)

    add_subdirectory(xvkbd)

    set(_runtest TRUE)
    set(KEYTEST "${CMAKE_CURRENT_SOURCE_DIR}/keytest.py")
    file(GLOB TESTST RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*-in.txt")
    file(GLOB TESTSS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*-in.sh")
    set(_firsttxt first-time-in.txt hello-world-in.txt)
    foreach(_f ${_firsttxt})
      list(FIND TESTST ${_f} _ff)
      if(NOT _ff GREATER -1)
        set(_runtest FALSE)
        break()
      endif()
    endforeach()
    list(REMOVE_ITEM TESTST ${_firsttxt})
    list(SORT TESTST)
  endif()
  if(_runtest)
    foreach(_tf ${_firsttxt} ${TESTST})
      string(REGEX REPLACE "-in\\.(txt|sh)" "" _t ${_tf})
      add_test(NAME keytest/${_t}
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
        COMMAND ${CMAKE_COMMAND}
        -DLYX_TESTS_USERDIR=${LYX_TESTS_USERDIR}
        -DAUTOTEST_ROOT=${TOP_SRC_DIR}/development/autotests
        -DPO_BUILD_DIR=${TOP_BINARY_DIR}/po
        -DKEYTEST_INFILE=${_tf}
        -DBINDIR=$<TARGET_FILE_DIR:${_lyx}>
        -DLYX=${_lyx}
        -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}
        -DLYX_USERDIR_VER=${LYX_USERDIR_VER}
        -DKEYTEST_OUTFILE=${_t}-out.txt
        -DPACKAGE=${PACKAGE}
	-DFRONTEND=${LYX_USE_QT}
        -DLOCALE_DIR=${LOCALE_DIR}
        -P ${TOP_SRC_DIR}/development/autotests/single-test.cmake)
      settestlabel(keytest/${_t} "key")
      set_tests_properties(keytest/${_t} PROPERTIES RUN_SERIAL ON)
    endforeach()
  endif()
endif()

if (LYX_ENABLE_EXPORT_TESTS)
  include(${TOP_SRC_DIR}/development/autotests/ExportTests.cmake)
endif()
