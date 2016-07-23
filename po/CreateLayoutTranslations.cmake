# -DTOP_SRC_DIR=xxx \
# -DDestFile=xxx \
# -DLYX_PYTHON_EXECUTABLE=xxx \
# -DParType=xxx \
# -DSources=xxx \

set(_dst ${DestFile})
set(type ${ParType})
set(_src_files ${Sources})

message(STATUS "Starting CreateLayoutTranslations")
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TOP_SRC_DIR}/lib/layouttranslations" "${_dst}"
  RESULT_VARIABLE _err)
string(COMPARE EQUAL  ${_err} 0 _erg)
if(NOT _erg)
  message(FATAL_ERROR "CreateLayoutTranslations: Copy of layouttranslations failed")
endif()

execute_process(COMMAND ${LYX_PYTHON_EXECUTABLE}
  "${TOP_SRC_DIR}/po/lyx_pot.py" -b "${TOP_SRC_DIR}" -o "${_dst}" -t ${type}
  "--src_file=${_src_files}"
  RESULT_VARIABLE _err)

string(COMPARE EQUAL  ${_err} 0 _erg)
if(NOT _erg)
  message(FATAL_ERROR "CreateLayoutTranslations: Calling python wit errors")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_dst}" "${TOP_SRC_DIR}/lib/layouttranslations"
  RESULT_VARIABLE _err)
if(NOT _erg)
  message(FATAL_ERROR "CreateLayoutTranslations: Copy of layouttranslations to source failed")
endif()

