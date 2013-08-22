#
#  based on cmake file
#

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

if (ZLIB_INCLUDE_DIR)
  # Already in cache, be silent
  set(ZLIB_FIND_QUIETLY TRUE)
endif()

set(zlibsearch "/usr/include" "/usr/local/include")
if (GNUWIN32_DIR)
  list(APPEND zlibsearch "${GNUWIN32_DIR}/include")
endif()

find_path(ZLIB_INCLUDE_DIR zlib.h PATHS ${zlibsearch})

find_file(ZLIB_HEADER zlib.h
	PATHS ${ZLIB_INCLUDE_DIR} NO_DEFAULT_PATH)
	
set(POTENTIAL_Z_LIBS z zlib zdll)

find_library(ZLIB_LIBRARY NAMES ${POTENTIAL_Z_LIBS}
	PATHS
	${ICONV_RELATIVE_TO_LIBINTL}
	NO_DEFAULT_PATH)
	

set(zlibsearch "C:\\Programme\\Microsoft Visual Studio 8\\VC\\lib")
if(SYSTEM_LIB_DIRS)
  list(APPEND zlibsearch ${SYSTEM_LIB_DIRS})
endif()
if(GNUWIN32_DIR)
  list(APPEND zlibsearch "${GNUWIN32_DIR}/lib")
endif()
find_library(ZLIB_LIBRARY NAMES ${POTENTIAL_Z_LIBS}
	PATHS ${zlibsearch})

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
      message(FATAL_ERROR "Could NOT find z library, set GNUWIN32_DIR to dir containing /include and /bin folders: -DGNUWIN32_DIR=...")
   endif()
endif()

mark_as_advanced(ZLIB_LIBRARY ZLIB_INCLUDE_DIR)
