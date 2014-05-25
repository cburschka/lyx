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

  if(NOT Missing)
    project(keytest)

    add_subdirectory(xvkbd)

    set(KEYTEST "${CMAKE_CURRENT_SOURCE_DIR}/keytest.py")
    file(GLOB TESTST RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*-in.txt")
    file(GLOB TESTSS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*-in.sh")
    list(REMOVE_ITEM TESTST hello-world-in.txt first-time-in.txt)
    list(SORT TESTST)

    foreach(_tf first-time-in.txt hello-world-in.txt ${TESTST})
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
        -DLOCALE_DIR=${LOCALE_DIR}
        -P ${TOP_SRC_DIR}/development/autotests/single-test.cmake)
      settestlabel(keytest/${_t} "key")
      set_tests_properties(keytest/${_t} PROPERTIES RUN_SERIAL ON)
    endforeach()
  endif()
endif()

find_package(Perl)

if(PERL_FOUND)
  set(DVI_FORMATS "dvi" "dvi3")
  set(PDF_FORMATS "pdf" "pdf2" "pdf3" "pdf4" "pdf5")
else()
  set(DVI_FORMATS "dvi")
  set(PDF_FORMATS "pdf" "pdf2" "pdf3")
endif()

macro(getoutputformats filepath varname)
  file(STRINGS "${filepath}" lines)
  # What should we test, if default_output_format is not defined?
  # For now we test everything ...
  set(out_formats "xhtml" ${DVI_FORMATS} ${PDF_FORMATS})
  foreach(_l ${lines})
    if(_l MATCHES "^\\\\default_output_format +\([^ ]+\)")
      set(_format ${CMAKE_MATCH_1})
      if(_format STREQUAL "default")
        set(out_formats "xhtml" ${DVI_FORMATS} ${PDF_FORMATS})
      elseif(_format MATCHES "pdf")
        set(found "xhtml" ${PDF_FORMATS})
      elseif(_format MATCHES "dvi")
        set(out_formats "xhtml" ${DVI_FORMATS})
      else()
        # Respect all other output formats
        # like "eps3", "xhtml"
        set(out_formats ${_format})
      endif()
      break()
    endif()
  endforeach()
  set(${varname} ${out_formats})
endmacro()

macro(findexpr found testname listname)
  set(_found 0)
  foreach(_itrx ${${listname}})
    set(_itr "^${_itrx}$")
    if(${testname} MATCHES ${_itr})
      set(_found 1)
      break()
    endif()
  endforeach()
  set(${found} ${_found})
endmacro()
  
macro(maketestname testname reverted listreverted listignored)
  findexpr(mfound ${testname} ${listignored})
  if (NOT mfound)
    findexpr(mfound ${testname} ${listreverted})
    list(FIND ${listreverted} ${${testname}} _idx)
    if (NOT mfound)
      set(${reverted} 0)
    else()
      set(${reverted} 1)
      set(${testname} "INVERTED_SEE-README.ctest_${${testname}}")
    endif()
  else()
    set(${testname} "")
  endif()
endmacro()

macro(loadTestList filename resList)
  # Create list of strings from a file without comments
  file(STRINGS ${filename} tempList)
  set(${resList})
  foreach(_l ${tempList})
    string(REGEX REPLACE "[ \t]*#.*" "" _newl "${_l}")
    if(_newl)
      list(APPEND ${resList} "${_newl}")
    endif()
  endforeach()
endmacro()

loadTestList(revertedTests revertedTests)
loadTestList(ignoredTests ignoredTests)

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
    # Strip extension
    string(REGEX REPLACE "\\.lyx$" "" f ${f})
    set(TestName "export/${libsubfolder}/${f}_lyx16")
    maketestname(TestName reverted revertedTests ignoredTests)
    if(TestName)
      add_test(NAME ${TestName}
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
        COMMAND ${CMAKE_COMMAND} -DLYX_ROOT=${LIBSUB_SRC_DIR}
            -DLYX_TESTS_USERDIR=${LYX_TESTS_USERDIR}
            -Dlyx=$<TARGET_FILE:${_lyx}>
            -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}
            -DLYX_USERDIR_VER=${LYX_USERDIR_VER}
            -Dformat=lyx16x
            -Dextension=16.lyx
            -Dfile=${f}
            -Dreverted=${reverted}
            -DTOP_SRC_DIR=${TOP_SRC_DIR}
            -DPERL_EXECUTABLE=${PERL_EXECUTABLE}
            -P "${TOP_SRC_DIR}/development/autotests/export.cmake")
      setmarkedtestlabel(${TestName} ${reverted} "export")
    endif()
    if(LYX_PYTHON_EXECUTABLE)
      # For use of lyx2lyx we need the python executable
      set(TestName "lyx2lyx/${libsubfolder}/${f}")
      maketestname(TestName reverted revertedTests ignoredTests)
      if(TestName)
        add_test(NAME ${TestName}
          WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
          COMMAND ${CMAKE_COMMAND}
            "-DLYX_PYTHON_EXECUTABLE=${LYX_PYTHON_EXECUTABLE}"
            "-DLYX2LYX=${TOP_SRC_DIR}/lib/lyx2lyx/lyx2lyx"
            "-DLYX_TESTS_USERDIR=${LYX_TESTS_USERDIR}"
            "-DLYXFILE=${LIBSUB_SRC_DIR}/${f}.lyx"
            -P "${TOP_SRC_DIR}/development/autotests/lyx2lyxtest.cmake")
        setmarkedtestlabel(${TestName} ${reverted} "export")
      endif()
    endif()
    set(TestName "check_load/${libsubfolder}/${f}")
    maketestname(TestName reverted revertedTests ignoredTests)
    if(TestName)
      add_test(NAME ${TestName}
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
        COMMAND ${CMAKE_COMMAND} -DLYXFILE=${LIBSUB_SRC_DIR}/${f}.lyx
          -DLYX_TESTS_USERDIR=${LYX_TESTS_USERDIR}
          -Dlyx=$<TARGET_FILE:${_lyx}>
          -DPARAMS_DIR=${TOP_SRC_DIR}/development/autotests
          -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}
          -P "${TOP_SRC_DIR}/development/autotests/check_load.cmake")
      setmarkedtestlabel(${TestName} ${reverted} "load")
    endif()
    getoutputformats("${LIBSUB_SRC_DIR}/${f}.lyx" formatlist)
    foreach(format ${formatlist})
      if(format MATCHES "dvi3|pdf4|pdf5")
        set(fonttypes "texF" "systemF")
      else()
        set(fonttypes "defaultF")
      endif()
      foreach(fonttype ${fonttypes})
        if(fonttype MATCHES "defaultF")
          set(TestName "export/${libsubfolder}/${f}_${format}")
        else()
          set(TestName "export/${libsubfolder}/${f}_${format}_${fonttype}")
        endif()
        maketestname(TestName reverted revertedTests ignoredTests)
        if(TestName)
          add_test(NAME ${TestName}
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
            COMMAND ${CMAKE_COMMAND} -DLYX_ROOT=${LIBSUB_SRC_DIR}
            -DLYX_TESTS_USERDIR=${LYX_TESTS_USERDIR}
            -Dlyx=$<TARGET_FILE:${_lyx}>
            -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}
            -Dformat=${format}
            -Dfonttype=${fonttype}
            -Dextension=${format}
            -Dfile=${f}
            -Dreverted=${reverted}
            -DTOP_SRC_DIR=${TOP_SRC_DIR}
            -DPERL_EXECUTABLE=${PERL_EXECUTABLE}
            -P "${TOP_SRC_DIR}/development/autotests/export.cmake")
          setmarkedtestlabel(${TestName} ${reverted} "export")
        endif()
      endforeach()
    endforeach()
  endforeach()
endforeach()

