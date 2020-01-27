
if(WIN32)
  find_library(HUNSPELL_LIBRARY "libhunspell")
else()
  find_library(HUNSPELL_LIBRARY NAMES "hunspell" "hunspell-1.2" PATHS "/usr/local/lib" ${SYSTEM_LIB_DIRS} "/usr/lib64")
endif()

FIND_PATH(HUNSPELL_INCLUDE_DIR "hunspell.hxx")
if (NOT HUNSPELL_INCLUDE_DIR)
  FIND_PATH(HUNSPELL_INCLUDE_DIR "hunspell/hunspell.hxx")
  if (HUNSPELL_INCLUDE_DIR)
    set(HUNSPELL_INCLUDE_DIR "${HUNSPELL_INCLUDE_DIR}/hunspell")
  endif()
endif()

# handle the QUIETLY and REQUIRED arguments and
# set HUNSPELL_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HUNSPELL DEFAULT_MSG HUNSPELL_LIBRARY HUNSPELL_INCLUDE_DIR)

mark_as_advanced(HUNSPELL_LIBRARY HUNSPELL_INCLUDE_DIR)
