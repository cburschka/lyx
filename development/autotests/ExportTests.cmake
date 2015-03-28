#
#  Copyright (c) 2014 Kornel Benko <kornel@lyx.org>
#  Copyright (c) 2014 Scott Kostyshak <skotysh@lyx.org>
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  1. Redistributions of source code must retain the copyright
#         notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the copyright
#         notice, this list of conditions and the following disclaimer in the
#         documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote products
#         derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
#  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

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
  string(REGEX MATCH "\\/[a-z][a-z](_[A-Z][A-Z])?\\/" _v ${${testname}})
  if(_v)
    string(REGEX REPLACE "\\/" "" _v ${_v})
    set(listrevertedx ${listreverted}_${_v})
    #message(STATUS "${listreverted} ==> ${listrevertedx}")
    set(listignoredx ${listignored}_${_v})
    #message(STATUS "${listignored} ==> ${listignoredx}")
  else()
    set(listrevertedx ${listreverted})
    set(listignoredx ${listignored})
  endif()
  findexpr(mfound ${testname} ${listignoredx})
  if (NOT mfound)
    # check if test _may_ be in listreverted
    string(REGEX MATCH "_(systemF|texF|pdf3|pdf2|pdf|dvi)$" _v ${${testname}})
    if (_v)
      findexpr(mfound ${testname} ${listrevertedx})
    else()
      set(mfound OFF)
    endif()
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
      #message(STATUS "Testing ${_newl}")
      string(REGEX REPLACE "(\\/|\\||\\(|\\))" "  " _vxx ${_newl})
      string(REGEX MATCHALL " ([a-z][a-z](_[A-Z][A-Z])?) " _vx ${_vxx})
      if(_vx)
        foreach(_v ${_vx})
          string(REGEX REPLACE " " "" _v ${_v})
          #message(STATUS " ==> ${resList}_${_v}")
          list(APPEND ${resList}_${_v} "${_newl}")
        endforeach()
        if(_newl MATCHES "\\(\\|")
          #message(STATUS " ==> ${resList}")
          list(APPEND ${resList} "${_newl}")
        endif()
      else()
        #message(STATUS " ==> ${resList}")
        list(APPEND ${resList} "${_newl}")
      endif()
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
        setmarkedtestlabel(${TestName} ${reverted} "lyx2lyx")
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
      set_tests_properties(${TestName} PROPERTIES RUN_SERIAL ON)
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
