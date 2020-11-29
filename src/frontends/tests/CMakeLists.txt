# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2013 Kornel Benko, <kornel@lyx.org>

project(testFrontends)

set(TESTS "test_biblio")

add_executable(biblio biblio.cpp)
#target_link_libraries(biblio)
add_dependencies(lyx_run_tests biblio)
set_target_properties(biblio PROPERTIES FOLDER "tests/frontends")

foreach(tst ${TESTS})
  add_test(NAME "frontends/${tst}"
    COMMAND ${CMAKE_COMMAND}
      -Dbiblio=$<TARGET_FILE:biblio>
      -Dsaved_data=${TOP_SRC_DIR}/src/frontends/tests/regfiles/biblio
      -P ${TOP_SRC_DIR}/src/frontends/tests/${tst}.cmake)
endforeach()
