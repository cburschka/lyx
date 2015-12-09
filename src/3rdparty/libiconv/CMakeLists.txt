# Copyright (C) 2007-2012 LuaDist.
# Created by Peter Drahoš
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
# Please note that the package source code is licensed under its own license.

project ( libiconv C )
cmake_minimum_required ( VERSION 2.8 )
set(CMAKE_MODULE_PATH .)
include ( configure.cmake )

set(LIBICONV_VERSION 1.14)

set(SRCDIR ${LIBICONV_VERSION})

# Options
option(ENABLE_EXTRA "Enable a few rarely used encodings" OFF)
option(ENABLE_NLS "Translation of program messages to the user's native
   language is requested" OFF)
#option(ENABLE_RELOCATABLE "The package shall run at any location in the file system" ON)

# iconv.h
set(USE_MBSTATE_T 1)
set(BROKEN_WCHAR_H 0)
set(HAVE_WCHAR_T 0)

set(BINDIR ${CMAKE_BINARY_DIR}/libiconv)
configure_file(config.h.cmake ${BINDIR}/config.h)
configure_file(${SRCDIR}/include/iconv.h.build.in ${BINDIR}/include/iconv.h)
configure_file(${SRCDIR}/libcharset/include/libcharset.h.in ${BINDIR}/include/libcharset.h)
configure_file(${SRCDIR}/srclib/uniwidth.in.h ${BINDIR}/srclib/uniwidth.h)
configure_file(${SRCDIR}/srclib/unitypes.in.h ${BINDIR}/srclib/unitypes.h)

# Dirty fix for MinGW
if(MINGW)
  add_definitions(-DELOOP=0 -DHAVE_DECL_STRERROR_R=0)
endif ()

include_directories( ${BINDIR}/include ${SRCDIR}/include ${SRCDIR}/srclib)
add_definitions(-Dset_relocation_prefix=libcharset_set_relocation_prefix -Drelocate=libcharset_relocate -DHAVE_CONFIG_H -DINSTALLPREFIX=NULL -DNO_XMALLOC -DBUILDING_LIBCHARSET -DINSTALLDIR="" -DLIBDIR="" -DENABLE_RELOCATABLE=1 -DIN_LIBRARY)

# libcharset
set(SRC_LIBCHARSET ${SRCDIR}/libcharset/lib/localcharset.c)

# libiconv
set(SRC_LIBICONV ${SRCDIR}/lib/iconv.c)

add_library(iconv ${SRC_LIBICONV} ${SRC_LIBCHARSET})
set_target_properties(iconv PROPERTIES COMPILE_FLAGS -DBUILDING_LIBICONV)

set(ICONV_INCLUDE_DIR ${BINDIR}/include CACHE STRING "libiconv include  dir" FORCE)
set(ICONV_LIBRARY iconv CACHE STRING "libiconv library" FORCE)
set(ICONV_FOUND iconv CACHE STRING "libiconv found" FORCE)
