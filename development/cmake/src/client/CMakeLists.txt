# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2008, 2009 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c) 2008, 2009 Kornel Benko, <Kornel.Benko@berlin.de>
#

set(_lyxclient lyxclient${PROGRAM_SUFFIX})
project(${_lyxclient})

file(GLOB _lyxclient_sources ${TOP_SRC_DIR}/src/client/*.cpp)
file(GLOB _lyxclient_headers ${TOP_SRC_DIR}/src/client/*.h)
list(REMOVE_ITEM _lyxclient_headers "${TOP_SRC_DIR}/src/client/pch.h")

include_directories(BEFORE "${TOP_SRC_DIR}/src/client"
  "${TOP_SRC_DIR}/boost" ${ZLIB_INCLUDE_DIR})

add_executable(${_lyxclient} ${_lyxclient_sources} ${_lyxclient_headers})


target_link_libraries(${_lyxclient}
	support
	boost_regex
	${LIBINTL_LIBRARIES}
        ${ICONV_LIBRARY}
	${QT_QTCORE_LIBRARY}
	${QT_QTGUI_LIBRARY}
	)

if (ASPELL_FOUND)
        target_link_libraries(${_lyxclient} ${ASPELL_LIBRARY})
endif()

if (APPLE)
	target_link_libraries(${_lyxclient} "-framework Carbon")
endif()

install(TARGETS ${_lyxclient} DESTINATION bin)

