# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko kornel@lyx.org
#
project(xvkbd)

find_package(PkgConfig)

add_definitions(-DUSE_XTEST -DUSE_I18N)

add_executable(xvkbd xvkbd.c findwidget.c)

set(XVFBDLIBS)

if(PKG_CONFIG_FOUND)
  pkg_check_modules(XAWLIB REQUIRED xaw7)
  list(APPEND XVFBDLIBS ${XAWLIB_LIBRARIES})
  pkg_check_modules(XTEST REQUIRED xtst)
  list(APPEND XVFBDLIBS ${XTEST_LIBRARIES})
else()
  list(APPEND XVFBDLIBS -lXaw7)
  foreach(_lb Xt XTest X11)
    if(X11_${_lb}_LIB)
      list(APPEND XVFBDLIBS ${X11_${_lb}_LIB})
    else()
      message(FATAL_ERROR "Library for ${_lb} not found")
    endif()
  endforeach()
endif()

target_link_libraries(xvkbd ${XVFBDLIBS})
add_dependencies(lyx_run_tests xvkbd)
