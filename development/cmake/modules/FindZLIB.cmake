#
#  based on cmake file
#

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

if (ZLIB_INCLUDE_DIR)
  # Already in cache, be silent
  set(ZLIB_FIND_QUIETLY TRUE)
endif()

find_path(ZLIB_INCLUDE_DIR zlib.h
 /usr/include
 /usr/local/include
 "${GNUWIN32_DIR}"/include)

set(POTENTIAL_Z_LIBS z zlib zdll)

find_library(ZLIB_LIBRARY NAMES ${POTENTIAL_Z_LIBS}
	PATHS 
	"C:\\Programme\\Microsoft Visual Studio 8\\VC\\lib"
	/usr/lib /usr/local/lib
	"${GNUWIN32_DIR}"/lib)

if(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY)
   set(ZLIB_FOUND TRUE)
endif()

if(ZLIB_FOUND)
   if(NOT ZLIB_FIND_QUIETLY)
      message(STATUS "Found Z: ${ZLIB_LIBRARY}")
   endif()
else()
   if(ZLIB_FIND_REQUIRED)
      message(STATUS "Looked for Z libraries named ${POTENTIAL_Z_LIBS}.")
      message(STATUS "Found no acceptable Z library. This is fatal.")
      message(FATAL_ERROR "Could NOT find z library")
   endif()
endif()

mark_as_advanced(ZLIB_LIBRARY ZLIB_INCLUDE_DIR)
