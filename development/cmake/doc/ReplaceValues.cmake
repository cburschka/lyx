set(ENV{PYTHONIOENCODING} "utf-8")

# check if all needed variables are set
foreach(_v LYX_USERDIR_VER LYX_DIR_VER REL_DIR SOURCE CREATED LYX_PYTHON_EXECUTABLE)
  if (NOT DEFINED ${_v})
    message(FATAL_ERROR "${_v} not set")
  else()
    #message(STATUS "${_v}=${${_v}}")
  endif()
endforeach()

set(originstring "origin\ unavailable=origin\ /systemlyxdir/doc/${REL_DIR}")
#message(STATUS "originstring=${originstring}")
execute_process(COMMAND ${LYX_PYTHON_EXECUTABLE} "${TOP_CMAKE_PATH}/doc/ReplaceValues.py"
  "LYX_USERDIR_VER=${LYX_USERDIR_VER}"
  "LYX_DIR_VER=${LYX_DIR_VER}"
  ${originstring}
  "${TOP_SRC_DIR}/lib/doc/${SOURCE}"
  OUTPUT_FILE "${CREATED}"
  RESULT_VARIABLE _err
  ERROR_VARIABLE lyxerr)

if(_err)
  message(STATUS "_err = ${_err}")
  message(STATUS "lyxerr = ${lyxerr}")
  message(FATAL_ERROR "Conversion of ${SOURCE} failed")
endif()
