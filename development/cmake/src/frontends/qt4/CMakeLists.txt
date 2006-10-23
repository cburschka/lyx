# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(frontend_qt4)


file(GLOB frontends_qt4_sources ${TOP_SRC_DIR}/src/frontends/qt4/*.C)
file(GLOB frontends_qt4_headers ${TOP_SRC_DIR}/src/frontends/qt4/*.h)
file(GLOB frontend_qt4_UI       ${TOP_SRC_DIR}/src/frontends/qt4/ui/*.ui)

lyx_add_msvc_pch(frontends_qt4)

lyx_automoc(${frontends_qt4_sources})

lyx_add_ui_files(frontends_qt4_sources ${frontend_qt4_UI})

add_definitions(
		-DQT_CLEAN_NAMESPACE
		-DQT_GENUINE_STR
		-DQT_NO_STL
		-DQT_NO_KEYWORDS
		-DLYX_BUILD_QT4_FRONTEND
		)


include_directories(	${TOP_SRC_DIR}/src/frontends/qt4 
			${TOP_SRC_DIR}/src/frontends/controllers
			${CMAKE_CURRENT_BINARY_DIR})

add_library(frontend_qt4 STATIC ${frontends_qt4_sources} ${frontends_qt4_headers})

target_link_libraries(frontend_qt4 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} controllers)

if(WIN32)
	target_link_libraries(frontend_qt4 Gdi32)
endif(WIN32)

project_source_group("${GROUP_CODE}" frontends_qt4_sources frontends_qt4_headers)
source_group("Uic files" FILES ${frontend_qt4_UI})

