#
#  Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  
#  1. Redistributions of source code must retain the copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote products 
#     derived from this software without specific prior written permission.
#  
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
#  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#  
# - Try to find ASPELL
# Once done this will define
#
#	ASPELL_FOUND - system has ASPELL
#	ASPELL_INCLUDE_DIR - the ASPELL include directory
#	ASPELL_LIBRARY - The libraries needed to use ASPELL
#	ASPELL_DEFINITIONS - Compiler switches required for using ASPELL
#

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

if(WIN32)
	file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _program_FILES_DIR)
endif()


if(ASPELL_INCLUDE_DIR AND ASPELL_LIBRARY)
	# Already in cache, be silent
	set(ASPELL_FIND_QUIETLY TRUE)
endif()

FIND_PATH(ASPELL_INCLUDE_DIR aspell.h
	/usr/include
	/usr/local/include
	/usr/local/include/aspell
	${_program_FILES_DIR}/gnuwin32/include
)

FIND_LIBRARY(ASPELL_LIBRARY_RELEASE NAMES aspell aspell-15 libaspell libaspell-15
	PATHS
	/usr/lib
	/usr/local/lib
)

# msvc makes a difference between debug and release
if(MSVC)
	find_library(ASPELL_LIBRARY_DEBUG NAMES aspelld libaspelld libaspell-15
		 PATHS 
		 ${_program_FILES_DIR}/kdewin32/lib)
		 		
	if(MSVC_IDE)
		if(NOT ASPELL_LIBRARY_DEBUG OR NOT ASPELL_LIBRARY_RELEASE)
			set(ASPELL_LIBRARY)
		else()	
			set(ASPELL_LIBRARY optimized ${ASPELL_LIBRARY_RELEASE} debug ${ASPELL_LIBRARY_DEBUG})
		endif()	
	else()
		string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
		if(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
			set(ASPELL_LIBRARY ${ASPELL_LIBRARY_DEBUG})
		else()
			set(ASPELL_LIBRARY ${ASPELL_LIBRARY_RELEASE})
		endif()
	endif()
else()
	set(ASPELL_LIBRARY ${ASPELL_LIBRARY_RELEASE})
endif()

if(ASPELL_INCLUDE_DIR AND ASPELL_LIBRARY)
	 set(ASPELL_FOUND TRUE)
endif()

if(ASPELL_FOUND)
	if (NOT ASPELL_FIND_QUIETLY)
	message(STATUS "Found ASPELL: ${ASPELL_LIBRARY}")
	endif()
else()
	if (ASPELL_FIND_REQUIRED)
	message("aspell header      : ${ASPELL_INCLUDE_DIR}")
	message("aspell lib release : ${ASPELL_LIBRARY_RELEASE}")
	message("aspell lib debug   : ${ASPELL_LIBRARY_DEBUG}")	
	if(MSVC_IDE)
		# the ide needs	the debug and release version
		if(NOT ASPELL_LIBRARY_DEBUG OR NOT ASPELL_LIBRARY_RELEASE)
			 message(FATAL_ERROR "\nCould NOT find the debug AND release version of the aspell library.\nYou need to have both to use MSVC projects.\nPlease build and install both kdelibs/win/ libraries first.\n")
		endif()
	endif()
	message(FATAL_ERROR "Could NOT find ASPELL")
	endif()
endif()

MARK_AS_ADVANCED(ASPELL_INCLUDE_DIR ASPELL_LIBRARY)
