
cmake_minimum_required(VERSION 2.8)

project(hunspell)

set(HUNSPELL_VERSION 1.3.3)

set(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/${HUNSPELL_VERSION}/src)

include_directories(./ ${SRCDIR}/hunspell ${SRCDIR}/parsers ${SRCDIR}/tools)

if(WIN32)
    include_directories(${SRCDIR}/win_api)
    set(HUNCONFIG ${SRCDIR}/win_api/config.h)
endif()

# LIBS
set(SRCS
    ${SRCDIR}/hunspell/affentry.cxx
    ${SRCDIR}/hunspell/affixmgr.cxx
    ${SRCDIR}/hunspell/csutil.cxx
    ${SRCDIR}/hunspell/dictmgr.cxx
    ${SRCDIR}/hunspell/hashmgr.cxx
    ${SRCDIR}/hunspell/suggestmgr.cxx
    ${SRCDIR}/hunspell/license.myspell
    ${SRCDIR}/hunspell/license.hunspell
    ${SRCDIR}/hunspell/phonet.cxx
    ${SRCDIR}/hunspell/filemgr.cxx
    ${SRCDIR}/hunspell/hunzip.cxx
    ${SRCDIR}/hunspell/hunspell.cxx
    ${SRCDIR}/hunspell/replist.cxx)

set(HEADERS
    ${SRCDIR}/hunspell/affentry.hxx
    ${SRCDIR}/hunspell/htypes.hxx
    ${SRCDIR}/hunspell/affixmgr.hxx
    ${SRCDIR}/hunspell/csutil.hxx
    ${SRCDIR}/hunspell/hunspell.hxx
    ${SRCDIR}/hunspell/atypes.hxx
    ${SRCDIR}/hunspell/dictmgr.hxx
    ${SRCDIR}/hunspell/hunspell.h
    ${SRCDIR}/hunspell/suggestmgr.hxx
    ${SRCDIR}/hunspell/baseaffix.hxx
    ${SRCDIR}/hunspell/hashmgr.hxx
    ${SRCDIR}/hunspell/langnum.hxx
    ${SRCDIR}/hunspell/phonet.hxx
    ${SRCDIR}/hunspell/filemgr.hxx
    ${SRCDIR}/hunspell/hunzip.hxx
    ${SRCDIR}/hunspell/w_char.hxx
    ${SRCDIR}/hunspell/replist.hxx
    ${SRCDIR}/hunspell/hunvisapi.h)

add_definitions(-DHUNSPELL_STATIC)
add_library(hunspell STATIC ${HEADERS} ${SRCS} ${HUNCONFIG})

set(HUNSPELL_LIBRARY hunspell CACHE STRING "Hunspell library" FORCE)
set(HUNSPELL_INCLUDE_DIR ${SRCDIR} CACHE STRING "Hunspell include dir" FORCE)
set(HUNSPELL_FOUND CACHE STRING "Hunspell found" FORCE)

set_target_properties(hunspell PROPERTIES FOLDER "3rd_party")
