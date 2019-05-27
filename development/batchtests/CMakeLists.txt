# Checking whether moving beamer slides through the document doesn't break its structure.
# see also https://www.mail-archive.com/lyx-devel@lists.lyx.org/msg203281.html
set(testlabel "batch")
string(TOUPPER "${testlabel}_" testprefix)

macro(add_batch_test testname testpar)
  add_test(NAME "${testprefix}${testname}" COMMAND ${PERL_EXECUTABLE} ${CMAKE_BINARY_DIR}/lyx_batch.pl ${testpar})
  setmarkedtestlabel(${testprefix}${testname} ${ARGN} "${testlabel}")
endmacro()

add_batch_test(outline-beamer beamer_test "export")
# Checking that info inset correctly fills up VCS information
# see also bug #10835
add_batch_test(vcs-info vcs_info_export)
add_batch_test(AMS-import ams-import "tex2lyx")
add_batch_test(SAVE-as save_as_test "export")

