#
#  based on cmake file
#
if (ZLIB_INCLUDE_DIR)
  # Already in cache, be silent
  set(ZLIB_FIND_QUIETLY TRUE)
endif()

FIND_PATH(ZLIB_INCLUDE_DIR zlib.h
 /usr/include
 /usr/local/include
)

set(POTENTIAL_Z_LIBS z zlib zdll)
FIND_LIBRARY(ZLIB_LIBRARY NAMES ${POTENTIAL_Z_LIBS}
PATHS
 "C:\\Programme\\Microsoft Visual Studio 8\\VC\\lib"
 /usr/lib
 /usr/local/lib
)

IF (ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY)
   set(ZLIB_FOUND TRUE)
endif()

IF (ZLIB_FOUND)
   IF (NOT ZLIB_FIND_QUIETLY)
      message(STATUS "Found Z: ${ZLIB_LIBRARY}")
   endif()
else()
   IF (ZLIB_FIND_REQUIRED)
      message(STATUS "Looked for Z libraries named ${POTENTIAL_Z_LIBS}.")
      message(STATUS "Found no acceptable Z library. This is fatal.")
      message(FATAL_ERROR "Could NOT find z library")
   endif()
endif()

MARK_AS_ADVANCED(ZLIB_LIBRARY ZLIB_INCLUDE_DIR)
