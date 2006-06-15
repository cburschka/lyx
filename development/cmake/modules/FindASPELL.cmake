# - Try to find ASPELL
# Once done this will define
#
#	ASPELL_FOUND - system has ASPELL
#	ASPELL_INCLUDE_DIR - the ASPELL include directory
#	ASPELL_LIBRARY - The libraries needed to use ASPELL
#	ASPELL_DEFINITIONS - Compiler switches required for using ASPELL
#

if(WIN32)
	file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _program_FILES_DIR)
endif(WIN32)


if (ASPELL_INCLUDE_DIR AND ASPELL_LIBRARY)
	# Already in cache, be silent
	set(ASPELL_FIND_QUIETLY TRUE)
endif (ASPELL_INCLUDE_DIR AND ASPELL_LIBRARY)

FIND_PATH(ASPELL_INCLUDE_DIR aspell.h
	/usr/include
	/usr/local/include
	/usr/local/include/aspell
	${_program_FILES_DIR}/gnuwin32/include
)

FIND_LIBRARY(ASPELL_LIBRARY_RELEASE NAMES aspell aspell-15
	PATHS
	/usr/lib
	/usr/local/lib
)

# msvc makes a difference between debug and release
if(MSVC)
	find_library(ASPELL_LIBRARY_DEBUG NAMES aspelld
		 PATHS 
		 ${_program_FILES_DIR}/kdewin32/lib)
		 		
	if(MSVC_IDE)
		if(NOT ASPELL_LIBRARY_DEBUG OR NOT ASPELL_LIBRARY_RELEASE)
			SET(ASPELL_LIBRARY)
		else(NOT ASPELL_LIBRARY_DEBUG OR NOT ASPELL_LIBRARY_RELEASE)	
			SET(ASPELL_LIBRARY optimized ${ASPELL_LIBRARY_RELEASE} debug ${ASPELL_LIBRARY_DEBUG})
		endif(NOT ASPELL_LIBRARY_DEBUG OR NOT ASPELL_LIBRARY_RELEASE)	
	else(MSVC_IDE)
		string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
		if(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
			set(ASPELL_LIBRARY ${ASPELL_LIBRARY_DEBUG})
		else(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
			set(ASPELL_LIBRARY ${ASPELL_LIBRARY_RELEASE})
		endif(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
	endif(MSVC_IDE)
else(MSVC)
	set(ASPELL_LIBRARY ${ASPELL_LIBRARY_RELEASE})
endif(MSVC)

if (ASPELL_INCLUDE_DIR AND ASPELL_LIBRARY)
	 set(ASPELL_FOUND TRUE)
endif (ASPELL_INCLUDE_DIR AND ASPELL_LIBRARY)

if (ASPELL_FOUND)
	if (NOT ASPELL_FIND_QUIETLY)
	message(STATUS "Found ASPELL: ${ASPELL_LIBRARY}")
	endif (NOT ASPELL_FIND_QUIETLY)
else (ASPELL_FOUND)
	if (ASPELL_FIND_REQUIRED)
	message("aspell header      : ${ASPELL_INCLUDE_DIR}")
	message("aspell lib release : ${ASPELL_LIBRARY_RELEASE}")
	message("aspell lib debug   : ${ASPELL_LIBRARY_DEBUG}")	
	if(MSVC_IDE)
		# the ide needs	the debug and release version
		if(NOT ASPELL_LIBRARY_DEBUG OR NOT ASPELL_LIBRARY_RELEASE)
			 message(FATAL_ERROR "\nCould NOT find the debug AND release version of the aspell library.\nYou need to have both to use MSVC projects.\nPlease build and install both kdelibs/win/ libraries first.\n")
		endif(NOT ASPELL_LIBRARY_DEBUG OR NOT ASPELL_LIBRARY_RELEASE)
	endif(MSVC_IDE)
	message(FATAL_ERROR "Could NOT find ASPELL")
	endif (ASPELL_FIND_REQUIRED)
endif (ASPELL_FOUND)

MARK_AS_ADVANCED(ASPELL_INCLUDE_DIR ASPELL_LIBRARY)
