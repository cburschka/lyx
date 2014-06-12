#
#  Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#                2012, Kornel Benko, <kornel@lyx.org>
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

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

include(CheckFunctionExists)
macro(check_magic_functions_exists _varname)
	set(${_varname} 1)
	if (Magic_LIBRARY)
		set(CMAKE_REQUIRED_LIBRARIES ${Magic_LIBRARY})
	endif()
	foreach(fkt file open load close error)
		check_function_exists(magic_${fkt} HAS_function_magic_${fkt})
		if (NOT HAS_function_magic_${fkt})
			set(${_varname} 0)
			message(STATUS "Function magic_${fkt} not found")
		endif()
	endforeach()
endmacro()
if (Magic_INCLUDE_DIR)
  # Already in cache, be silent
  set(Magic_FIND_QUIETLY TRUE)
endif()

find_path(Magic_INCLUDE_DIR magic.h PATHS
 /usr/include
 /usr/local/include)
 	
find_library(Magic_LIBRARY NAMES "magic")
	
check_magic_functions_exists(HAS_MAGIC_FUNCTIONS)

# handle the QUIET and REQUIRED arguments and DEFAULT_MSG
# set Magic_FOUND to TRUE if all listed variables are TRUE

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Magic DEFAULT_MSG Magic_INCLUDE_DIR Magic_LIBRARY HAS_MAGIC_FUNCTIONS)
