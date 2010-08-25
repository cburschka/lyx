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

check_include_file_cxx(aspell.h HAVE_ASPELL_H)
check_include_file_cxx(aspell/aspell.h HAVE_ASPELL_ASPELL_H)
#check_include_file_cxx(istream HAVE_ISTREAM)
#check_include_file_cxx(ostream HAVE_OSTREAM)
#check_include_file_cxx(ios HAVE_IOS)
#check_include_file_cxx(sstream HAVE_SSTREAM)
#check_include_file_cxx(locale HAVE_LOCALE)

check_include_files(limits.h HAVE_LIMITS_H)
check_include_files(locale.h HAVE_LOCALE_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(sys/utime.h HAVE_SYS_UTIME_H)
check_include_files(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(utime.h HAVE_UTIME_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(argz.h HAVE_ARGZ_H)


check_function_exists(open HAVE_OPEN)
check_function_exists(chmod HAVE_CHMOD)
check_function_exists(close HAVE_CLOSE)
check_function_exists(dcgettext HAVE_DCGETTEXT)
check_function_exists(popen HAVE_POPEN)
check_function_exists(pclose HAVE_PCLOSE)
check_function_exists(_open HAVE__OPEN)
check_function_exists(_close HAVE__CLOSE)
check_function_exists(_popen HAVE__POPEN)
check_function_exists(_pclose HAVE__PCLOSE)
check_function_exists(getpid HAVE_GETPID)
check_function_exists(gettext HAVE_GETTEXT)
check_function_exists(_getpid HAVE__GETPID)
check_function_exists(mkdir  HAVE_MKDIR)
check_function_exists(_mkdir HAVE__MKDIR)
check_function_exists(setenv HAVE_SETENV)
check_function_exists(putenv HAVE_PUTENV)
check_function_exists(fcntl HAVE_FCNTL)
check_function_exists(strerror HAVE_STRERROR)
check_function_exists(getcwd HAVE_GETCWD)
check_function_exists(stpcpy HAVE_STPCPY)
check_function_exists(strcasecmp HAVE_STRCASECMP)
check_function_exists(strdup HAVE_STRDUP)
check_function_exists(strtoul HAVE_STRTOUL)
check_function_exists(alloca HAVE_ALLOCA)
check_function_exists(__fsetlocking HAVE___FSETLOCKING)
check_function_exists(mempcpy HAVE_MEMPCPY)
check_function_exists(__argz_count HAVE___ARGZ_COUNT)
check_function_exists(__argz_next HAVE___ARGZ_NEXT)
check_function_exists(__argz_stringify HAVE___ARGZ_STRINGIFY)
check_function_exists(setlocale HAVE_SETLOCALE)
check_function_exists(tsearch HAVE_TSEARCH)
check_function_exists(getegid HAVE_GETEGID)
check_function_exists(getgid HAVE_GETGID)
check_function_exists(getuid HAVE_GETUID)
check_function_exists(wcslen HAVE_WCSLEN)
check_function_exists(mkfifo HAVE_MKFIFO)

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

