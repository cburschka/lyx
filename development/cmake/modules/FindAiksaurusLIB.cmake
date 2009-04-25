#
#  based on FindZLIB.cmake
#  created 2009, Kornel Benko, <kornel.benko@berlin.de>
#

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

if (AIKSAURUS_INCLUDE_DIR)
  # Already in cache, be silent
  set(AIKSAURUS_FIND_QUIETLY TRUE)
endif()

find_path(AIKSAURUS_INCLUDE_DIR NAMES Aiksaurus.h
  PATH_SUFFIXES "" "Aiksaurus")
if(AIKSAURUS_INCLUDE_DIR)
  if(AIKSAURUS_INCLUDE_DIR MATCHES "Aiksaurus")
    set(AIKSAURUS_H Aiksaurus/Aiksaurus.h)
  else()
    set(AIKSAURUS_H Aiksaurus.h)
  endif()
endif()

set(POTENTIAL_AIKSAURUS_LIBS Aiksaurus)

find_library(AIKSAURUSLIB_LIBRARY NAMES ${POTENTIAL_AIKSAURUS_LIBS}
	PATHS
	/usr/lib /usr/local/lib)

if(AIKSAURUS_INCLUDE_DIR AND AIKSAURUSLIB_LIBRARY)
   set(AIKSAURUSLIB_FOUND TRUE)
endif()

if(AIKSAURUSLIB_FOUND)
   if(NOT AIKSAURUS_FIND_QUIETLY)
      message(STATUS "Found AIKSAURUS: ${AIKSAURUSLIB_LIBRARY}")
   endif()
else()
   if(AIKSAURUS_FIND_REQUIRED)
      message(STATUS "Looked for AIKSAURUS libraries named ${POTENTIAL_AIKSAURUS_LIBS}.")
      message(STATUS "Found no acceptable AIKSAURUS library. This is fatal.")
   endif()
endif()

mark_as_advanced(AIKSAURUSLIB_LIBRARY AIKSAURUSLIB_INCLUDE_DIR)
