
if(WIN32)
  find_library(HUNSPELL_LIBRARY "libhunspell")
else()
  #find_library(HUNSPELL_LIBRARY NAME "hunspell" PATH "/usr/local/lib" "/usr/lib" NO_DEFAULT_PATH)
  find_library(HUNSPELL_LIBRARY NAMES "hunspell" "hunspell-1.2" PATHS "/usr/local/lib" "/usr/lib" "/usr/lib64")
endif()

FIND_PATH(HUNSPELL_INCLUDE_DIR "hunspell/hunspell.hxx")


# handle the QUIETLY and REQUIRED arguments and 
# set HUNSPELL_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HUNSPELL DEFAULT_MSG HUNSPELL_LIBRARY HUNSPELL_INCLUDE_DIR)

if(HUNSPELL_FIND_REQUIRED AND NOT HUNSPELL_FOUND)
	message(FATAL_ERROR "Could not find Hunspell library")
endif()

mark_as_advanced(HUNSPELL_LIBRARY HUNSPELL_INCLUDE_DIR)
