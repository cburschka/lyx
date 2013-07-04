# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko kornel@lyx.org
#
# Variable "XVFBDLIBS" created in development/autotests/CMakeLists.txt
#
project(xvkbd)

add_definitions(-DUSE_XTEST -DUSE_I18N)

add_executable(xvkbd xvkbd.c findwidget.c)

target_link_libraries(xvkbd ${XVFBDLIBS})
add_dependencies(lyx_run_tests xvkbd)
