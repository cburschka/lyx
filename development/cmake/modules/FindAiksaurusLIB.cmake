#
#  based on FindZLIB.cmake
#  created 2009, Kornel Benko, <kornel.benko@berlin.de>
#

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

if (AIKSAURUSLIB_INCLUDE_DIR)
  # Already in cache, be silent
  set(AIKSAURUSLIB_FIND_QUIETLY TRUE)
endif()

find_path(AIKSAURUSLIB_INCLUDE_DIR NAMES Aiksaurus.h
  PATH_SUFFIXES "" "Aiksaurus")
if(AIKSAURUSLIB_INCLUDE_DIR)
  if(AIKSAURUSLIB_INCLUDE_DIR MATCHES "Aiksaurus")
    set(AIKSAURUSLIB_H Aiksaurus/Aiksaurus.h)
  else()
    set(AIKSAURUSLIB_H Aiksaurus.h)
  endif()
endif()

set(POTENTIAL_AIKSAURUS_LIBS Aiksaurus)

find_library(AIKSAURUSLIB_LIBRARY NAMES ${POTENTIAL_AIKSAURUS_LIBS}
	PATHS
	${SYSTEM_LIB_DIRS})

# handle the QUIETLY and REQUIRED arguments and set AIKSAURUSLIB_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AIKSAURUSLIB DEFAULT_MSG AIKSAURUSLIB_LIBRARY AIKSAURUSLIB_INCLUDE_DIR)

set(AIKSAURUSLIB_H ${AIKSAURUSLIB_H} CACHE STRING "Aiksaurus header" FORCE)

mark_as_advanced(AIKSAURUSLIB_LIBRARY AIKSAURUSLIB_INCLUDE_DIR AIKSAURUSLIB_H)
