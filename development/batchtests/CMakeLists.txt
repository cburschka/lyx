# Checking whether moving beamer slides through the document doesn't break its structure.
# see also https://www.mail-archive.com/lyx-devel@lists.lyx.org/msg203281.html
add_test(NAME "outline-beamer" COMMAND ${PERL_EXECUTABLE} ${CMAKE_BINARY_DIR}/lyx_batch.pl beamer_test)
setmarkedtestlabel(outline-beamer "export")
# Checking that info inset correctly fills up VCS information
# see also bug #10835
add_test(NAME "vcs-info" COMMAND ${PERL_EXECUTABLE} ${CMAKE_BINARY_DIR}/lyx_batch.pl vcs_info_export)
