cmake_minimum_required(VERSION 2.4.4)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(zlib C)

set(VERSION "1.2.8")
set(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/1.2.8)

option(ASM686 "Enable building i686 assembly implementation")
option(AMD64 "Enable building amd64 assembly implementation")

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
enable_testing()

check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h    HAVE_STDINT_H)
check_include_file(stddef.h    HAVE_STDDEF_H)

#
# Check to see if we have large file support
#
set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE=1)
# We add these other definitions here because CheckTypeSize.cmake
# in CMake 2.4.x does not automatically do so and we want
# compatibility with CMake 2.4.x.
if(HAVE_SYS_TYPES_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_SYS_TYPES_H)
endif()
if(HAVE_STDINT_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDINT_H)
endif()
if(HAVE_STDDEF_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDDEF_H)
endif()
check_type_size(off64_t OFF64_T)
if(HAVE_OFF64_T)
   add_definitions(-D_LARGEFILE64_SOURCE=1)
endif()
set(CMAKE_REQUIRED_DEFINITIONS) # clear variable

#
# Check for fseeko
#
check_function_exists(fseeko HAVE_FSEEKO)
if(NOT HAVE_FSEEKO)
    add_definitions(-DNO_FSEEKO)
endif()

#
# Check for unistd.h
#
check_include_file(unistd.h Z_HAVE_UNISTD_H)

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    # If we're doing an out of source build and the user has a zconf.h
    # in their source tree...
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/zconf.h)
        message(STATUS "Renaming")
        message(STATUS "    ${CMAKE_CURRENT_SOURCE_DIR}/zconf.h")
        message(STATUS "to 'zconf.h.included' because this file is included with zlib")
        message(STATUS "but CMake generates it automatically in the build directory.")
        file(RENAME ${CMAKE_CURRENT_SOURCE_DIR}/zconf.h ${CMAKE_CURRENT_SOURCE_DIR}/zconf.h.included)
  endif()
endif()

#set(ZLIB_PC ${CMAKE_CURRENT_BINARY_DIR}/zlib.pc)
#configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/zlib.pc.cmakein ${ZLIB_PC} @ONLY)
set(Z_PREFIX 1)
configure_file(${SRCDIR}/zconf.h.cmakein ${CMAKE_BINARY_DIR}/zconf.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})


#============================================================================
# zlib
#============================================================================

set(ZLIB_PUBLIC_HDRS
    ${CMAKE_BINARY_DIR}/zconf.h
    ${SRCDIR}/zlib.h
)
set(ZLIB_PRIVATE_HDRS
    ${SRCDIR}/crc32.h
    ${SRCDIR}/deflate.h
    ${SRCDIR}/gzguts.h
    ${SRCDIR}/inffast.h
    ${SRCDIR}/inffixed.h
    ${SRCDIR}/inflate.h
    ${SRCDIR}/inftrees.h
    ${SRCDIR}/trees.h
    ${SRCDIR}/zutil.h
)
set(ZLIB_SRCS
    ${SRCDIR}/adler32.c
    ${SRCDIR}/compress.c
    ${SRCDIR}/crc32.c
    ${SRCDIR}/deflate.c
    ${SRCDIR}/gzclose.c
    ${SRCDIR}/gzlib.c
    ${SRCDIR}/gzread.c
    ${SRCDIR}/gzwrite.c
    ${SRCDIR}/inflate.c
    ${SRCDIR}/infback.c
    ${SRCDIR}/inftrees.c
    ${SRCDIR}/inffast.c
    ${SRCDIR}/trees.c
    ${SRCDIR}/uncompr.c
    ${SRCDIR}/zutil.c
)


if(CMAKE_COMPILER_IS_GNUCC)
    if(ASM686)
        set(ZLIB_ASMS contrib/asm686/match.S)
    elseif (AMD64)
        set(ZLIB_ASMS contrib/amd64/amd64-match.S)
    endif ()

	if(ZLIB_ASMS)
		add_definitions(-DASMV)
		set_source_files_properties(${ZLIB_ASMS} PROPERTIES LANGUAGE C COMPILE_FLAGS -DNO_UNDERLINE)
	endif()
endif()

if(MSVC)
    if(ASM686)
		ENABLE_LANGUAGE(ASM_MASM)
        set(ZLIB_ASMS
			contrib/masmx86/inffas32.asm
			contrib/masmx86/match686.asm
		)
    elseif (AMD64)
		ENABLE_LANGUAGE(ASM_MASM)
        set(ZLIB_ASMS
			contrib/masmx64/gvmat64.asm
			contrib/masmx64/inffasx64.asm
		)
    endif()

	if(ZLIB_ASMS)
		add_definitions(-DASMV -DASMINF)
	endif()
endif()

add_library(zlibstatic STATIC ${ZLIB_SRCS} ${ZLIB_ASMS} ${ZLIB_PUBLIC_HDRS} ${ZLIB_PRIVATE_HDRS})


set(ZLIB_LIBRARY zlibstatic CACHE STRING "Zlib library" FORCE)
set(ZLIB_INCLUDE_DIR  ${SRCDIR} CACHE STRING "Zlib include dir" FORCE)
set(ZLIB_FOUND CACHE STRING "Zlib found" FORCE )

set_target_properties(zlibstatic PROPERTIES FOLDER "3rd_party")
