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

project(dt2dv C)

if (WIN32)
  set_source_files_properties(dt2dv.c dv2dt.c PROPERTIES LANGUAGE C COMPILE_FLAGS -D_CRT_SECURE_NO_WARNINGS=1)
endif()
add_executable(dt2dv dt2dv.c)
add_executable(dv2dt dv2dt.c)
install(TARGETS dv2dt dt2dv DESTINATION ${LYX_UTILITIES_INSTALL_PATH})
