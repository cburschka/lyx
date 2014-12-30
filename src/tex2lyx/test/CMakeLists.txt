# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko, <kornel@lyx.org>
#

project(test)

#file(GLOB _tex_tests RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.ltx" "${CMAKE_CURRENT_SOURCE_DIR}/*.tex")
#
#list(REMOVE_ITEM _tex_tests DummyDocument.tex)

set(_tex_tests test.ltx
               box-color-size-space-align.tex
               CJK.tex
               CJKutf8.tex
               test-insets.tex
               test-memoir.tex
               test-modules.tex
               test-refstyle-theorems.tex
               test-scr.tex
               test-structure.tex
               verbatim.tex
               XeTeX-polyglossia.tex
               algo2e.tex)

foreach(_fl ${_tex_tests})
  set(fl ${_fl})
  add_test(NAME tex2lyx/roundtrip/${_fl}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND}
      -DLYX_TESTS_USERDIR=${LYX_TESTS_USERDIR}
      -DLYX_USERDIR_VER=${LYX_USERDIR_VER}
      -DLYX_PYTHON_EXECUTABLE=${LYX_PYTHON_EXECUTABLE}
      -DPY_SCRIPT=${TOP_SRC_DIR}/src/tex2lyx/test/runtests.py
      -DTEX2LYX_EXE=$<TARGET_FILE:${_tex2lyx}>
      -DSCRIPT_DIR=${TOP_SRC_DIR}/lib/scripts
      -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}
      -DTESTFILE=${fl}
      -P ${TOP_SRC_DIR}/src/tex2lyx/test/runtests.cmake)
  set_tests_properties(tex2lyx/roundtrip/${_fl} PROPERTIES RESOURCE_LOCK "runtests.lock" LABELS "roundtrip")
  add_test(NAME tex2lyx/cmplyx/${_fl}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND}
      -DLYX_TESTS_USERDIR=${LYX_TESTS_USERDIR}
      -DLYX_USERDIR_VER=${LYX_USERDIR_VER}
      -DLYX_PYTHON_EXECUTABLE=${LYX_PYTHON_EXECUTABLE}
      -DPY_SCRIPT=${TOP_SRC_DIR}/src/tex2lyx/test/runtests.py
      -DFIRST_PARAM=uselyx2lyx
      -DTEX2LYX_EXE=$<TARGET_FILE:${_tex2lyx}>
      -DSCRIPT_DIR=${TOP_SRC_DIR}/lib/scripts
      -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}
      -DTESTFILE=${fl}
      -P ${TOP_SRC_DIR}/src/tex2lyx/test/runtests.cmake)
  set_tests_properties(tex2lyx/cmplyx/${_fl} PROPERTIES RESOURCE_LOCK "runtests.lock" LABELS "cmplyx")
endforeach()

add_dependencies(lyx_run_tests ${_tex2lyx} ${_lyx})

