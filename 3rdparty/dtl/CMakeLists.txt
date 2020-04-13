cmake_minimum_required(VERSION 3.1)

set(LYX_IPO_SUPPORTED FALSE)
if (POLICY CMP0069)
  cmake_policy(SET CMP0069 NEW)
  if (LYX_USE_IPO MATCHES "ON")
    set(LYX_IPO_SUPPORTED YES)
  endif()
endif()
if (POLICY CMP0075)
  cmake_policy(SET CMP0075 NEW)
endif()

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(dtl C)
set(targets dt2dv dv2dt)

set(_manuals)
macro(add_man _mansrc)
	get_filename_component(_mandest ${_mansrc} NAME_WE)
	SET(_created_manual "${CMAKE_CURRENT_BINARY_DIR}/${_mandest}.1")
	configure_file("${TOP_SRC_DIR}/3rdparty/dtl/${_mansrc}" ${_created_manual} COPYONLY)
	list(APPEND _manuals ${_created_manual})
endmacro(add_man)

foreach(_t ${targets})
  if (WIN32)
    set_source_files_properties(${_t}.c PROPERTIES LANGUAGE C COMPILE_FLAGS -D_CRT_SECURE_NO_WARNINGS=1)
  endif()
  add_executable(${_t} ${_t}.c)
  add_man(${_t}.man)
endforeach()

# Install dtl tools in this directory to omit possible packaging conflicts
# if installing multiple lyx versions, each with own dtl executables
get_data_destination(dest_data)
install(TARGETS ${targets} DESTINATION "${dest_data}/extratools")
install(FILES ${_manuals} DESTINATION ${LYX_MAN_DIR}/man1)
