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
      elseif(_format MATCHES "pdf$")
        set(out_formats "xhtml" ${PDF_FORMATS})
      elseif(_format MATCHES "dvi$")
        set(out_formats "xhtml" ${DVI_FORMATS})
      elseif(_format MATCHES "xhtml")
        set(out_formats "xhtml")
      else()
        # Respect all other output formats
        # like "eps3"
        set(out_formats "xhtml" ${_format})
      endif()
      break()
    endif()
  endforeach()
  set(${varname} ${out_formats})
endmacro()

macro(findexpr found testname listname rsublabel)
  set(_found 0)
  set(tmpsublabel "")
  foreach(_itrx ${${listname}})
    if ("${_itrx}" MATCHES "^Sublabel:")
      set(tmpsublabel "")
      string(REGEX REPLACE "^Sublabel:[ \t]*" "" _itrlabels ${_itrx})
      string(REGEX MATCHALL "([a-zA-Z]+)" _labels ${_itrlabels})
      foreach(subl ${_labels})
        if (subl STREQUAL "RESET")
          set(tmpsublabel "")
        else()
          list(APPEND tmpsublabel ${subl})
        endif()
      endforeach()
      # remove doubles in sublabel
      list(REMOVE_DUPLICATES tmpsublabel)
    else()
      set(_itr "^${_itrx}$")
      if (${testname} MATCHES "${_itr}")
        set(_found 1)
        break()
      endif()
    endif()
  endforeach()
  if (${_found})
    if (NOT "${tmpsublabel}" STREQUAL "")
      list(APPEND ${rsublabel} ${tmpsublabel})
    endif()
  endif()
  set(${found} ${_found})
endmacro()

function(join rvalues glue routput)
  set(locallist ${${rvalues}})
  list(REMOVE_ITEM locallist "export" "lyx2lyx" "load")
  string(REGEX REPLACE "([^\\]|^);" "\\1${glue}" out "${locallist}")
  set(${routput} ${out} PARENT_SCOPE)
endfunction()

macro(maketestname testname reverted listsuspicious listignored listunreliable listlabels)
  string(REGEX MATCH "\\/[a-z][a-z](_[A-Z][A-Z])?\\/" _v ${${testname}})
  if(_v)
    string(REGEX REPLACE "\\/" "" _v ${_v})
    set(listsuspiciousx ${listsuspicious}_${_v})
    set(listignoredx ${listignored}_${_v})
    set(listunreliablex ${listunreliable}_${_v})
    set(listsuspendedx suspendedTests_${v})
  else()
    set(listsuspiciousx ${listsuspicious})
    set(listignoredx ${listignored})
    set(listunreliablex ${listunreliable})
    set(listsuspendedx suspendedTests)
  endif()
  set(sublabel "${${listlabels}}")
  findexpr(mfound ${testname} ${listignoredx} sublabel)
  if (NOT mfound)
    set(sublabel2 "")
    findexpr(foundunreliable ${testname} ${listunreliablex} sublabel2)
    if (foundunreliable)
      set(sublabel "unreliable" ${sublabel} ${sublabel2})
      list(REMOVE_ITEM sublabel "export" "reverted" "templates" "mathmacros" "manuals" "autotests")
    else()
      string(REGEX MATCH "_(systemF|texF|pdf3|pdf2|pdf|dvi|lyx16|xhtml)$" _v ${${testname}})
      # check if test _may_ be in listsuspicious
      set(sublabel2 "")
      if (_v)
	findexpr(mfound ${testname} ${listsuspiciousx} sublabel2)
      else()
	set(mfound OFF)
      endif()
      if (mfound)
	set(sublabel3 "")
	findexpr(foundsuspended ${testname} ${listsuspendedx} sublabel3)
	set(${reverted} 1)
	if (foundsuspended)
	  set(sublabel "reverted" "suspended" ${sublabel} ${sublabel2} ${sublabel3})
	  list(REMOVE_ITEM sublabel "export")
	else()
	  set(sublabel "reverted" ${sublabel} ${sublabel2} ${sublabel3})
	endif()
      else()
	set(${reverted} 0)
      endif()
    endif()
    list(REMOVE_DUPLICATES sublabel)
    if (NOT sublabel STREQUAL "")
      join(sublabel "." tmpprefixx)
      string(TOUPPER "${tmpprefixx}_" tmpprefix)
      set(${testname} "${tmpprefix}${${testname}}")
      string(TOUPPER "${sublabel}_" tmpprefix)
      set(${listlabels} ${sublabel})
    endif()
  else()
    set(${testname} "")
  endif()
endmacro()

# This labels should not be used in .*Tests files
set(known_labels "export" "key" "layout" "load" "lyx2lyx"
  "examples" "manuals" "mathmacros" "reverted" "templates" "unreliable" "suspended" 
  "module" "roundtrip" "url")

macro(loadTestList filename resList)
  # Create list of strings from a file without comments
  # ENCODING parameter is a new feature in cmake 3.1
  if (CMAKE_VERSION VERSION_GREATER "3.1")
    file(STRINGS ${filename} tempList ENCODING "UTF-8")
  else()
    file(STRINGS ${filename} tempList)
  endif()
  set(${resList})
  set(sublabel)
  set(mylabels "")
  set(languages "")
  message(STATUS "Reading list ${filename}")
  foreach(_l ${tempList})
    set(_newl "${_l}")
    string(REGEX REPLACE "[ \t]+$" "" _newl "${_l}")
    string(REGEX REPLACE "[ \t]*#.*$" "" _newl "${_l}")
    if(_newl)
      list(APPEND ${resList} "${_newl}")
      if (_newl MATCHES "^Sublabel:")
        string(REGEX REPLACE "^Sublabel:[ \t]*" "" _newlabels ${_newl})
        string(REGEX MATCHALL "([a-zA-Z]+)" _labels ${_newlabels})
        foreach(labname ${_labels})
          if (NOT labname STREQUAL "RESET")
            list(APPEND mylabels ${labname})
          endif()
        endforeach()
        list(REMOVE_DUPLICATES mylabels)
        set(sublabel ${_newl})
      else()
        string(REGEX REPLACE "(\\/|\\||\\(|\\))" "  " _vxx ${_newl})
        string(REGEX MATCHALL " ([a-z][a-z](_[A-Z][A-Z])?) " _vx ${_vxx})
        if(_vx)
          foreach(_v ${_vx})
            string(REGEX REPLACE " " "" _v ${_v})
            #message(STATUS " ==> ${resList}_${_v}")
            #message(STATUS "sublabel = ${sublabel}, sublabel_${_v} = ${sublabel_${_v}}")
            if (NOT sublabel STREQUAL "${sublabel_${_v}}")
              list(APPEND ${resList}_${_v} "${sublabel}")
              set(sublabel_${_v} "${sublabel}")
              #message(STATUS "Setting variable sublabel_${_v} with \"${sublabel}\"")
            endif()
            list(APPEND ${resList}_${_v} "${_newl}")
            list(APPEND languages ${_v})
          endforeach()
          list(REMOVE_DUPLICATES languages)
          #message(STATUS "languages = ${languages}")
        endif()
      endif()
    endif()
  endforeach()
  foreach(_l ${mylabels})
    list(FIND known_labels ${_l} _ff)
    if (_ff GREATER -1)
      message(STATUS "Label \"${_l}\" already in use. Reused in ${filename}")
    else()
      list(APPEND known_labels ${_l})
    endif()
  endforeach()
  foreach(_lg ${languages})
    # reset label for each used language string at end of file
    #message(STATUS "Resetting variable sublabel_${_lg}, previously set to ${sublabel_${_lg}}")
    set(sublabel_${_lg} "")
  endforeach()
endmacro()

loadTestList(suspiciousTests suspiciousTests)
loadTestList(ignoredTests ignoredTests)
loadTestList(suspendedTests suspendedTests)
loadTestList(unreliableTests unreliableTests)

# preparing to add e.g. development/mathmacros to the foreach() loop
foreach(libsubfolderx autotests/export lib/doc lib/examples lib/templates development/mathmacros)
  set(testlabel "export")
  if (libsubfolderx MATCHES "lib/doc")
    list(APPEND testlabel "manuals")
  elseif (libsubfolderx MATCHES "lib/examples")
    list(APPEND testlabel "examples")
  elseif (libsubfolderx MATCHES "lib/templates")
    list(APPEND testlabel "templates")
  elseif (libsubfolderx MATCHES "development/mathmacros")
    list(APPEND testlabel "mathmacros")
  elseif (libsubfolderx MATCHES "autotests/.+")
    list(APPEND testlabel "autotests")
  endif()
  set(LIBSUB_SRC_DIR "${TOP_SRC_DIR}/${libsubfolderx}")
  string(REGEX REPLACE "^(lib|development|autotests)/" "" libsubfolder "${libsubfolderx}")
  set(LIBSUB_SRC_DIR "${TOP_SRC_DIR}/${libsubfolderx}")
  message(STATUS "Handling export dir ${LIBSUB_SRC_DIR}")
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
    set(mytestlabel ${testlabel})
    maketestname(TestName reverted suspiciousTests ignoredTests unreliableTests mytestlabel)
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
      setmarkedtestlabel(${TestName} ${reverted} ${mytestlabel})
    endif()
    if(LYX_PYTHON_EXECUTABLE)
      set(lyx2lyxtestlabel "lyx2lyx")
      # For use of lyx2lyx we need the python executable
      set(mytestlabel ${lyx2lyxtestlabel})
      set(TestName "lyx2lyx/${libsubfolder}/${f}")
      maketestname(TestName reverted suspiciousTests ignoredTests unreliableTests mytestlabel)
      if(TestName)
        add_test(NAME ${TestName}
          WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
          COMMAND ${CMAKE_COMMAND}
          "-DLYX_PYTHON_EXECUTABLE=${LYX_PYTHON_EXECUTABLE}"
          "-DLYX2LYX=${TOP_SRC_DIR}/lib/lyx2lyx/lyx2lyx"
          "-DLYX_TESTS_USERDIR=${LYX_TESTS_USERDIR}"
          "-DLYXFILE=${LIBSUB_SRC_DIR}/${f}.lyx"
          -P "${TOP_SRC_DIR}/development/autotests/lyx2lyxtest.cmake")
        setmarkedtestlabel(${TestName} ${reverted} ${mytestlabel})
      endif()
    endif()
    set(loadtestlabel "load")
    set(mytestlabel ${loadtestlabel})
    set(TestName "check_load/${libsubfolder}/${f}")
    maketestname(TestName reverted suspiciousTests ignoredTests unreliableTests mytestlabel)
    if(TestName)
      add_test(NAME ${TestName}
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}"
        COMMAND ${CMAKE_COMMAND} -DLYXFILE=${LIBSUB_SRC_DIR}/${f}.lyx
        -DLYX_TESTS_USERDIR=${LYX_TESTS_USERDIR}
        -Dlyx=$<TARGET_FILE:${_lyx}>
        -DPARAMS_DIR=${TOP_SRC_DIR}/development/autotests
        -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}/${LYX_HOME}
        -P "${TOP_SRC_DIR}/development/autotests/check_load.cmake")
      setmarkedtestlabel(${TestName} ${reverted} ${mytestlabel})
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
        set(mytestlabel ${testlabel})
        maketestname(TestName reverted suspiciousTests ignoredTests unreliableTests mytestlabel)
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
          setmarkedtestlabel(${TestName} ${reverted} ${mytestlabel}) # check for suspended pdf/dvi exports
        endif()
      endforeach()
    endforeach()
  endforeach()
endforeach()
