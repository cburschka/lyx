# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko kornel@lyx.org
#

if(Q_WS_X11)
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
    message(STATUS "cmake build is therefore omitting autotests")
  endif()

  if(NOT Missing)
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
        -DLYX=${_lyx}
        -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}
        -DKEYTEST_OUTFILE=${_t}-out.txt
        -DPACKAGE=${PACKAGE}
        -DLOCALE_DIR=${LOCALE_DIR}
        -P ${TOP_SRC_DIR}/development/autotests/single-test.cmake)
    endforeach()
  endif()
endif()

macro(getoutputformats filepath varname)
  file(STRINGS "${filepath}" lines)
  set(out_formats "xhtml" "pdf" "pdf2" "pdf5")
  set(${varname} ${out_formats})
  foreach(_l ${lines})
    if(_l MATCHES "^\\\\default_output_format +\([^ ]+\)")
      if(CMAKE_MATCH_1 STREQUAL "default")
        set(found ${out_formats})
      elseif(CMAKE_MATCH_1 STREQUAL "xhtml")
        set(found "xhtml")
      else()
        set(found "xhtml" ${CMAKE_MATCH_1})
      endif()
      set(${varname} ${found})
      break()
    endif()
  endforeach()
endmacro()

macro(getreverted testname reverted listreverted)
  list(FIND ${listreverted} ${${testname}} _idx)
  if (${_idx} LESS 0)
    set(${reverted} 0)
  else()
    set(${reverted} 1)
    set(${testname} "INVERTED_SEE-README.ctest_${${testname}}")
  endif()
endmacro()

foreach(libsubfolder doc examples templates)
  set(LIBSUB_SRC_DIR "${TOP_SRC_DIR}/lib/${libsubfolder}")
  file(GLOB_RECURSE lyx_files RELATIVE "${LIBSUB_SRC_DIR}" "${LIBSUB_SRC_DIR}/*.lyx")
  list(SORT lyx_files)
  # Now create 2 lists. One for files in a language dir, one without
  set(lang_lyx_files)
  set(nolang_lyx_files)
  foreach(f ${lyx_files})
    string(REGEX MATCHALL "^[a-z][a-z](_[A-Z][A-Z])?\\/" _v ${f})
    if(_v)
      list(APPEND lang_lyx_files ${f})
    else()
      list(APPEND nolang_lyx_files ${f})
    endif()
  endforeach()
  file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/revertedTests" revertedTests)
  foreach(f ${nolang_lyx_files} ${lang_lyx_files})
    # Strip extension
    string(REGEX REPLACE "\\.lyx$" "" f ${f})
    set(TestName "export/${libsubfolder}/${f}_lyx16")
    getreverted(TestName reverted revertedTests)
    add_test(NAME ${TestName}
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
      COMMAND ${CMAKE_COMMAND} -DLYX_ROOT=${LIBSUB_SRC_DIR}
          -Dlyx=$<TARGET_FILE:${_lyx}>
          -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}
          -Dformat=lyx16x
          -Dextension=16.lyx
          -Dfile=${f}
          -Dreverted=${reverted}
          -P "${TOP_SRC_DIR}/development/autotests/export.cmake")
    getoutputformats("${LIBSUB_SRC_DIR}/${f}.lyx" formatlist)
    foreach(format ${formatlist})
      set(TestName "export/${libsubfolder}/${f}_${format}")
      getreverted(TestName reverted revertedTests)
      add_test(NAME ${TestName}
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
        COMMAND ${CMAKE_COMMAND} -DLYX_ROOT=${LIBSUB_SRC_DIR}
            -Dlyx=$<TARGET_FILE:${_lyx}>
            -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}
            -Dformat=${format}
            -Dextension=${format}
            -Dfile=${f}
            -Dreverted=${reverted}
            -P "${TOP_SRC_DIR}/development/autotests/export.cmake")
    endforeach()
  endforeach()
endforeach()

