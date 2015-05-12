# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckCXXSourceCompiles)
include(MacroBoolTo01)
include(TestBigEndian)

test_big_endian(WORDS_BIGENDIAN)

#check_include_file_cxx(istream HAVE_ISTREAM)
#check_include_file_cxx(ostream HAVE_OSTREAM)
#check_include_file_cxx(sstream HAVE_SSTREAM)
#check_include_file_cxx(ios HAVE_IOS)
#check_include_file_cxx(locale HAVE_LOCALE)

# defines will be written to configIncludes.h
set(Include_Defines)
foreach(_h_file aspell.h aspell/aspell.h limits.h locale.h
	stdlib.h sys/stat.h sys/time.h sys/types.h sys/utime.h
	sys/socket.h unistd.h inttypes.h utime.h string.h argz.h)
	string(REGEX REPLACE "[/\\.]" "_" _hf ${_h_file})
	string(TOUPPER ${_hf} _HF)
	check_include_files(${_h_file} HAVE_${_HF})
	set(Include_Defines "${Include_Defines}#cmakedefine HAVE_${_HF} 1\n")
endforeach()
configure_file(${LYX_CMAKE_DIR}/configIncludes.cmake ${TOP_BINARY_DIR}/configIncludes.h.cmake)
configure_file(${TOP_BINARY_DIR}/configIncludes.h.cmake ${TOP_BINARY_DIR}/configIncludes.h)

# defines will be written to configFunctions.h
set(Function_Defines)
foreach(_f alloca __argz_count __argz_next __argz_stringify
	chmod close _close dcgettext fcntl fork __fsetlocking
	getcwd getegid getgid getpid _getpid gettext getuid lstat lockf mempcpy mkdir _mkdir
	mkfifo open _open pclose _pclose popen _popen putenv readlink
	setenv setlocale strcasecmp stpcpy strdup strerror strtoul tsearch unsetenv wcslen)
  string(TOUPPER ${_f} _UF)
  check_function_exists(${_f} HAVE_${_UF})
  set(Function_Defines "${Function_Defines}#cmakedefine HAVE_${_UF} 1\n")
endforeach()
configure_file(${LYX_CMAKE_DIR}/configFunctions.cmake ${TOP_BINARY_DIR}/configFunctions.h.cmake)
configure_file(${TOP_BINARY_DIR}/configFunctions.h.cmake ${TOP_BINARY_DIR}/configFunctions.h)

check_symbol_exists(alloca "malloc.h" HAVE_SYMBOL_ALLOCA)
check_symbol_exists(asprintf "stdio.h" HAVE_ASPRINTF)
check_symbol_exists(wprintf "stdio.h" HAVE_WPRINTF)
check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
check_symbol_exists(printf "stdio.h" HAVE_POSIX_PRINTF)
check_symbol_exists(pid_t "sys/types.h" HAVE_PID_T)
check_symbol_exists(intmax_t "inttypes.h" HAVE_INTTYPES_H_WITH_UINTMAX)
check_symbol_exists(uintmax_t "stdint.h" HAVE_STDINT_H_WITH_UINTMAX)
check_symbol_exists(LC_MESSAGES "locale.h" HAVE_LC_MESSAGES)

check_type_size(intmax_t HAVE_INTMAX_T)
macro_bool_to_01(HAVE_UINTMAX_T HAVE_STDINT_H_WITH_UINTMAX)

check_type_size("long double"  HAVE_LONG_DOUBLE)
check_type_size("long long"  HAVE_LONG_LONG)
check_type_size(wchar_t HAVE_WCHAR_T)
check_type_size(wint_t  HAVE_WINT_T)


#check_cxx_source_compiles(
#	"
#	#include <algorithm>
#	using std::count;
#	int countChar(char * b, char * e, char const c)
#	{
#		return count(b, e, c);
#	}
#	int main(){return 0;}
#	"
#HAVE_STD_COUNT)

#check_cxx_source_compiles(
#	"
#	#include <cctype>
#	using std::tolower;
#	int main(){return 0;}
#	"
#CXX_GLOBAL_CSTD)

check_cxx_source_compiles(
	"
	#include <iconv.h>
	// this declaration will fail when there already exists a non const char** version which returns size_t
	double iconv(iconv_t cd,  char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft);
	int main() { return 0; }
	"
HAVE_ICONV_CONST)

check_cxx_source_compiles(
	"
	int i[ ( sizeof(wchar_t)==2 ? 1 : -1 ) ];
	int main(){return 0;}
	"
SIZEOF_WCHAR_T_IS_2)

check_cxx_source_compiles(
	"
	int i[ ( sizeof(wchar_t)==4 ? 1 : -1 ) ];
	int main(){return 0;}
	"
SIZEOF_WCHAR_T_IS_4)

# Check whether STL is libstdc++
check_cxx_source_compiles(
	"
	#include <vector>
	int main() {
	#if ! defined(__GLIBCXX__) && ! defined(__GLIBCPP__)
		this is not libstdc++
	#endif
		return(0);
	}
	"
lyx_cv_lib_stdcxx)

# Check whether STL is libstdc++ with C++11 ABI
check_cxx_source_compiles(
	"
	#include <vector>
	int main() {
	#if ! defined(_GLIBCXX_USE_CXX11_ABI) || ! _GLIBCXX_USE_CXX11_ABI
		this is not libstdc++ using the C++11 ABI
	#endif
		return(0);
	}
	"
USE_GLIBCXX_CXX11_ABI)

check_cxx_source_compiles(
	"
	#ifndef __clang__
		this is not clang
	#endif
	int main() {
	  return(0);
	}
	"
lyx_cv_prog_clang)

set(USE_LLVM_LIBCPP)
set(USE_GLIBCXX_CXX11_ABI)
if(NOT lyx_cv_lib_stdcxx)
  if(lyx_cv_prog_clang)
    # use libc++ provided by llvm instead of GNU libstdc++
    set(USE_LLVM_LIBCPP 1)
  endif()
endif()

