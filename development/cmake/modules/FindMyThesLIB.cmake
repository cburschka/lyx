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

set(POTENTIAL_MYTHES_LIBS mythes)

find_library(MYTHESLIB_LIBRARY NAMES ${POTENTIAL_MYTHES_LIBS}
	PATHS
	/usr/lib /usr/local/lib)

if(MYTHESLIB_INCLUDE_DIR AND MYTHESLIB_LIBRARY)
   set(MYTHESLIB_FOUND TRUE)
endif()

if(MYTHESLIB_FOUND)
   if(NOT MYTHESLIB_FIND_QUIETLY)
      message(STATUS "Found MYTHES: ${MYTHESLIB_LIBRARY}")
   endif()
else()
   if(MYTHESLIB_FIND_REQUIRED)
      message(STATUS "Looked for MYTHES libraries named ${POTENTIAL_MYTHES_LIBS}.")
      message(STATUS "Found no acceptable MYTHES library. This is fatal.")
   endif()
endif()

mark_as_advanced(MYTHESLIB_LIBRARY MYTHESLIB_INCLUDE_DIR)
