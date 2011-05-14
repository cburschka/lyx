# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006,2011 Peter Kümmel, <syntheticpp@gmx.net>
#

project(intl) 

# libgnuintl.h.in => libintl.h
configure_file("${TOP_SRC_DIR}/intl/libgnuintl.h.cmake" ${CMAKE_BINARY_DIR}/intl/libgnuintl.h)
configure_file("${TOP_SRC_DIR}/intl/libgnuintl.h.cmake" ${CMAKE_BINARY_DIR}/intl/libintl.h)


add_definitions(
	-DHAVE_CONFIG_H=1
	-DMAKE_INTL_LIB
	-DIN_LIBINTL
	-DENABLE_RELOCATABLE=1
	-DIN_LIBRARY
	-DNO_XMALLOC
	-Dset_relocation_prefix=libintl_set_relocation_prefix
	-Drelocate=libintl_relocate
	-DDEPENDS_ON_LIBICONV=1)
    
if(NOT APPLE)    
	add_definitions(
		-DLOCALEDIR=\\"${LOACLE_DIR}\\" 
		-DLOCALE_ALIAS_PATH=\\"${LOACLE_DIR}\\"
		-DLIBDIR=\\"${TOP_SRC_DIR}\\"
		-DINSTALLDIR=\\"${PREFIX}\\" )
else()
	add_definitions(
		-DLOCALEDIR='\"${LOCALE_DIR}\"' 
		-DLOCALE_ALIAS_PATH='\"${LOCALE_DIR}\"'
		-DLIBDIR='\"${TOP_SRC_DIR}\"'
		-DINSTALLDIR='\"${PREFIX}\"' )
endif()

file(GLOB intl_headers ${TOP_SRC_DIR}/intl/*.h)

file(GLOB intl_sources RELATIVE "${TOP_SRC_DIR}/intl" "${TOP_SRC_DIR}/intl/*.c")
list(REMOVE_ITEM intl_sources vasnprintf.c version.c printf-parse.c os2compat.c intl-exports.c)

lyx_add_path(intl_sources ${TOP_SRC_DIR}/intl)

include_directories(${TOP_SRC_DIR}/intl ${CMAKE_CURRENT_BINARY_DIR})

if(ICONV_FOUND)
	include_directories(${ICONV_INCLUDE_DIR})
endif()

add_library(intl STATIC ${intl_sources} ${intl_headers})

get_target_property(libintl_location intl LOCATION)
set(LIBINTL_LIBRARIES ${libintl_location} CACHE STRING "libintl library" FORCE)

find_path(LIBINTL_INCLUDE_DIR intl/libintl.h
	PATHS ${CMAKE_BINARY_DIR} NO_DEFAULT_PATH)
	
find_file(LIBINTL_HEADER intl/libintl.h
	PATHS ${CMAKE_BINARY_DIR} NO_DEFAULT_PATH)
		
if(ICONV_FOUND)
	target_link_libraries(intl ${ICONV_LIBRARY})
endif()

project_source_group("${GROUP_CODE}" intl_sources intl_headers)

