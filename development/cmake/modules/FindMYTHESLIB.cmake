#
#  based on FindZLIB.cmake
#  created 2009, Kornel Benko, <kornel.benko@berlin.de>
#

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

if (MYTHESLIB_INCLUDE_DIR)
  # Already in cache, be silent
  set(MYTHESLIB_FIND_QUIETLY TRUE)
endif()

set(MYTHES_H mythes.hxx)
find_path(MYTHESLIB_INCLUDE_DIR ${MYTHES_H}
 /usr/include
 /usr/local/include)

set(POTENTIAL_MYTHES_LIBS mythes-1.2)

find_library(MYTHESLIB_LIBRARY NAMES ${POTENTIAL_MYTHES_LIBS}
	PATHS ${SYSTEM_LIB_DIRS} )

# handle the QUIETLY and REQUIRED arguments and set MYTHESLIB_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MYTHESLIB DEFAULT_MSG MYTHESLIB_LIBRARY MYTHESLIB_INCLUDE_DIR)

mark_as_advanced(MYTHESLIB_LIBRARY MYTHESLIB_INCLUDE_DIR)
