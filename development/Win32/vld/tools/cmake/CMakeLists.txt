///////////////////////////////////////////////////////////////////////////////
//
//  Visual Leak Detector - CallStack Class Implementations
//  Copyright (c) 2005-2006 Dan Moulding
//  Copyright (c) 2007 Peter Kümmel, CMake files
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
//  See COPYING.txt for the full terms of the GNU Lesser General Public License.
//
////////////////////////////////////////////////////////////////////////////////

project(vld)

include(CheckCXXSourceCompiles)

check_cxx_source_compiles(
	"
	#pragma comment(lib, \"dbghelp.lib\")
	#define UNICODE
	#define _UNICODE
	#include <windows.h>
	#include <dbghelp.h>
	int main(){void* ptr = (void*)&SymInitializeW; return 0;}
	"
HAVE_DEBUGGING_TOOLS_FOR_WINDOWS)

if (NOT HAVE_DEBUGGING_TOOLS_FOR_WINDOWS)
	message(STATUS "'Debugging Tools for Windows' not found.")
	message(STATUS "Please install the tools from http://www.microsoft.com/whdc/devtools/debugging/default.mspx")
	message(STATUS "and add it to the Visual Studio include and lib serach paths.")
	message(FATAL_ERROR "")
endif (NOT HAVE_DEBUGGING_TOOLS_FOR_WINDOWS)

file(GLOB vld_sources ${vld_path}/src/*.cpp)
file(GLOB vld_headers ${vld_path}/src/*.h)

add_definitions(-DVLD_BUILD -DUNICODE -D_UNICODE)

add_library(vld SHARED ${vld_sources} ${vld_headers} ${vld_path}/include/vld.h)
  
