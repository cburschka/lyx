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
	-DLYX=${_lyx}
	-DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}
        -DKEYTEST_OUTFILE=${_t}-out.txt
        -DPACKAGE=${PACKAGE}
        -DLOCALE_DIR=${LOCALE_DIR}
        -P ${TOP_SRC_DIR}/development/autotests/single-test.cmake)
    endforeach()
  endif()
endif()

macro(getdefaultoutputformat filepath varname)
  file(STRINGS "${filepath}" lines)
  set(${varname} "pdf") # try at least this one
  foreach(_l ${lines})
    if(_l MATCHES "^\\\\default_output_format +\([^ ]+\)")
      set(found ${CMAKE_MATCH_1})
      if(found STREQUAL "default")
	set(found "pdf" "pdf2" "pdf5")
      endif()
      set(${varname} ${found})
      break()
    endif()
  endforeach()
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
  foreach(f ${nolang_lyx_files} ${lang_lyx_files})
    if(f MATCHES "MergedManuals")
      # we do not test (for now) the uncompilable file MergedManuals.lyx
    else()
      # Strip extension
      string(REGEX REPLACE "\\.lyx$" "" f ${f})
      add_test(NAME export/${libsubfolder}/${f}_lyx16
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
        COMMAND ${CMAKE_COMMAND} -DLYX_ROOT=${LIBSUB_SRC_DIR}
            -Dlyx=$<TARGET_FILE:${_lyx}>
            -Dformat=lyx16x
            -Dextension=16.lyx
            -Dfile=${f}
            -P "${TOP_SRC_DIR}/development/autotests/export.cmake")
      getdefaultoutputformat("${LIBSUB_SRC_DIR}/${f}.lyx" formatlist)
      foreach(format "xhtml" ${formatlist})
        add_test(NAME export/${libsubfolder}/${f}_${format}
          WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
          COMMAND ${CMAKE_COMMAND} -DLYX_ROOT=${LIBSUB_SRC_DIR}
              -Dlyx=$<TARGET_FILE:${_lyx}>
              -Dformat=${format}
              -Dextension=${format}
              -Dfile=${f}
              -P "${TOP_SRC_DIR}/development/autotests/export.cmake")
      endforeach()
    endif()
  endforeach()
endforeach()


