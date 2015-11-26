# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2013 Kornel Benko <kornel@lyx.org>
#           (c) 2013 Scott Kostyshak <skotysh@lyx.org>
#
# Needed, because of perl scripts here
find_package(Perl REQUIRED)

# create file the lyx-files-list
set(TOP_SEARCH_PATH "${TOP_SRC_DIR}")
set(LYXFILES_FILE "${CMAKE_CURRENT_BINARY_DIR}/filesToScan")
file(WRITE "${LYXFILES_FILE}")
file(GLOB_RECURSE lyx_files RELATIVE "${TOP_SEARCH_PATH}" "${TOP_SEARCH_PATH}/*.lyx")

foreach(_f ${lyx_files})
  file(APPEND "${LYXFILES_FILE}" "${_f}\n")
endforeach()

# Define the perl-script running the actual test
set(SEARCH_URL_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/search_url.pl")
set(GET_TRANSLATORS_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/getTranslators.pl")

# Test used and as accessible expected urls
add_test(NAME "check_accessible_urls"
  WORKING_DIRECTORY "${TOP_SEARCH_PATH}"
  COMMAND ${PERL_EXECUTABLE} "${SEARCH_URL_SCRIPT}"
    "filesToScan=${LYXFILES_FILE}"
    "ignoredURLS=${CMAKE_CURRENT_SOURCE_DIR}/inaccessibleURLS"
    "ignoredURLS=${CMAKE_CURRENT_SOURCE_DIR}/knownInvalidURLS"
    "summaryFile=${TOP_BINARY_DIR}/Testing/Temporary/LastFailedAccessibleURLS.log")

# Test inaccessible, but revert the error marker (failed <=> passed)
# if this fails, then some url is accessible and does not belong
# to file inaccessibleURLS
add_test(NAME "check_inaccessible_urls"
  WORKING_DIRECTORY "${TOP_SEARCH_PATH}"
  COMMAND ${PERL_EXECUTABLE} "${SEARCH_URL_SCRIPT}"
    "filesToScan=${LYXFILES_FILE}"
    "selectedURLS=${CMAKE_CURRENT_SOURCE_DIR}/inaccessibleURLS"
    "revertedURLS=${CMAKE_CURRENT_SOURCE_DIR}/inaccessibleURLS"
    "summaryFile=${TOP_BINARY_DIR}/Testing/Temporary/LastFailedInaccessibleURLS.log")

#
# Test our own bad urls
# and urls used in examples which should not match valid urls
# if this test fails, then our testtool contains errors
add_test(NAME "check_invalid_urls"
  WORKING_DIRECTORY "${TOP_SEARCH_PATH}"
  COMMAND ${PERL_EXECUTABLE} "${SEARCH_URL_SCRIPT}"
    "extraURLS=${CMAKE_CURRENT_SOURCE_DIR}/knownInvalidURLS"
    "revertedURLS=${CMAKE_CURRENT_SOURCE_DIR}/knownInvalidURLS"
    "summaryFile=${TOP_BINARY_DIR}/Testing/Temporary/LastFailedKnownInvalidURLS.log")

set(URL_TEST_NAMES "check_accessible_urls" "check_inaccessible_urls" "check_invalid_urls")
add_test(NAME "check_translators"
  WORKING_DIRECTORY "${TOP_BINARY_DIR}"
  COMMAND ${PERL_EXECUTABLE} "${GET_TRANSLATORS_SCRIPT}")

set_tests_properties(${URL_TEST_NAMES}
        PROPERTIES RESOURCE_LOCK "url.lock" LABELS "url")
set_tests_properties("check_translators"
        PROPERTIES RESOURCE_LOCK "url.lock")

settestlabel("check_inaccessible_urls" "reverted")
settestlabel("check_invalid_urls" "reverted")
