# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko kornel@lyx.org
#

if(Q_WS_X11)
  # Make sure, the needed programs are in PATH
  find_program(PCREGREP_EXE "pcregrep")
  find_program(WMCTRL_EXE "wmctrl")
  # This is needed to build xvkbd
  # Programs pcregrep wmctrl and xvkbd are used in subsequent scripts
  # while testing
  find_package(X11)
  if(X11_FOUND AND PCREGREP_EXE AND WMCTRL_EXE)
    #message(STATUS "PCREGREP_EXE and WMCTRL_EXE found")
    project(autotests)

    add_subdirectory(xvkbd)

    set(KEYTEST "${CMAKE_CURRENT_SOURCE_DIR}/keytest.py")
    set(LYX_HOME "out-home")
    set(LYX_USERDIR "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}/.lyx")
    set(LOCALE_DIR "${CMAKE_CURRENT_BINARY_DIR}/locale") 
    file(GLOB TESTST RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*-in.txt")
    file(GLOB TESTSS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*-in.sh")
    list(REMOVE_ITEM TESTST hello-world-in.txt first-time-in.txt)
    list(SORT TESTST)
    file(MAKE_DIRECTORY "${LYX_USERDIR}" "${LOCALE_DIR}")

    foreach(_tf first-time-in.txt hello-world-in.txt ${TESTST})
      string(REGEX REPLACE "-in\\.(txt|sh)" "" _t ${_tf})
      add_test(NAME autotests/${_t}
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
	COMMAND ${CMAKE_COMMAND}
        -DAUTOTEST_ROOT=${TOP_SRC_DIR}/development/autotests
        -DPO_BUILD_DIR=${TOP_BINARY_DIR}/po
        -DKEYTEST_INFILE=${_tf}
        -DBINDIR=$<TARGET_FILE_DIR:${_lyx}>
	-DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}
        -DKEYTEST_OUTFILE=${_t}-out.txt
        -DPACKAGE=${PACKAGE}
        -DLOCALE_DIR=${LOCALE_DIR}
        -P ${TOP_SRC_DIR}/development/autotests/single-test.cmake)
    endforeach()
  endif()
endif()

file(GLOB lyx_files RELATIVE "${TOP_SRC_DIR}/lib/doc" "${TOP_SRC_DIR}/lib/doc/*.lyx")
foreach(f ${lyx_files})
  # Strip extension
  string(REGEX REPLACE "\\.lyx$" "" f ${f})
  add_test(NAME autotests/export/${f}_lyx16
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
    COMMAND ${CMAKE_COMMAND} -DLYX_ROOT=${TOP_SRC_DIR}/lib/doc
            -Dlyx=$<TARGET_FILE:${_lyx}>
	    -Dformat=lyx16x
	    -Dextension=16.lyx
	    -Dfile=${f}
	    -P "${TOP_SRC_DIR}/development/autotests/export.cmake")
  add_test(NAME autotests/export/${f}_xhtml
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
    COMMAND ${CMAKE_COMMAND} -DLYX_ROOT=${TOP_SRC_DIR}/lib/doc
            -Dlyx=$<TARGET_FILE:${_lyx}>
	    -Dformat=xhtml
	    -Dextension=xhtml
	    -Dfile=${f}
	    -P "${TOP_SRC_DIR}/development/autotests/export.cmake")
endforeach()


