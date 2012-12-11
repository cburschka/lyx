# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko kornel@lyx.org
#

if(UNIX)
  find_package(X11)
  if(X11_FOUND)
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
      add_test(NAME "auto_${_t}"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
	COMMAND ${CMAKE_COMMAND}
        -DAUTOTEST_ROOT=${TOP_SRC_DIR}/development/autotests
        -DKEYTEST_INFILE=${_tf}
        -DBINDIR=${TOP_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}
	-DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}
        -DKEYTEST_OUTFILE=${_t}-out.txt
        -P ${TOP_SRC_DIR}/development/autotests/single-test.cmake)
    endforeach()
    add_test(NAME lyx_export
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
      COMMAND ${CMAKE_COMMAND} -DLYX_ROOT=${TOP_SRC_DIR} -Dlyx=${TOP_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/lyx -P "${TOP_SRC_DIR}/development/autotests/export.cmake")
    set_tests_properties(lyx_export PROPERTIES DEPENDS lyx)
  endif()
endif()
