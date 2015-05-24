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
               algo2e.tex
               box-color-size-space-align.tex
               CJK.tex
               CJKutf8.tex
               test-insets-basic.tex
               test-insets.tex
               test-memoir.tex
               test-modules.tex
               test-refstyle-theorems.tex
               test-scr.tex
               test-structure.tex
               XeTeX-polyglossia.tex)

set(runtestsScript "${TOP_SRC_DIR}/src/tex2lyx/test/runtests.py")
set(scriptDir "${TOP_SRC_DIR}/lib/scripts")

foreach(_fl ${_tex_tests})
  set(fl ${_fl})
  add_test(NAME tex2lyx/roundtrip/${_fl}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND}
      -DLYX_TESTS_USERDIR=${LYX_TESTS_USERDIR}
      -DLYX_USERDIR_VER=${LYX_USERDIR_VER}
      -DLYX_PYTHON_EXECUTABLE=${LYX_PYTHON_EXECUTABLE}
      -DPY_SCRIPT=${runtestsScript}
      -DTEX2LYX_EXE=$<TARGET_FILE:${_tex2lyx}>
      -DSCRIPT_DIR=${scriptDir}
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
      -DPY_SCRIPT=${runtestsScript}
      -DFIRST_PARAM=uselyx2lyx
      -DTEX2LYX_EXE=$<TARGET_FILE:${_tex2lyx}>
      -DSCRIPT_DIR=${scriptDir}
      -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}
      -DTESTFILE=${fl}
      -P ${TOP_SRC_DIR}/src/tex2lyx/test/runtests.cmake)
  set_tests_properties(tex2lyx/cmplyx/${_fl} PROPERTIES RESOURCE_LOCK "runtests.lock" LABELS "cmplyx")
endforeach()

add_dependencies(lyx_run_tests ${_tex2lyx} ${_lyx})

add_custom_command(
  OUTPUT LyxTestFiles
  COMMAND ${CMAKE_COMMAND} -E touch LyxTestFiles
  COMMAND env LYX_USERDIR_VER=${LYX_USERDIR_VER} 
    ${LYX_PYTHON_EXECUTABLE} "${runtestsScript}" $<TARGET_FILE:${_tex2lyx}> "${scriptDir}" "${CMAKE_CURRENT_SOURCE_DIR}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS ${_tex2lyx} ${_lyx}
)

add_custom_target(updatetex2lyxtests DEPENDS LyxTestFiles)
