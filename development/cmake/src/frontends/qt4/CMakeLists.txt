# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(frontend_qt4)


file(GLOB frontends_qt4_sources ${TOP_SRC_DIR}/src/frontends/qt4/${LYX_CPP_FILES})
file(GLOB frontends_qt4_headers ${TOP_SRC_DIR}/src/frontends/qt4/${LYX_HPP_FILES})
file(GLOB frontend_qt4_UI       ${TOP_SRC_DIR}/src/frontends/qt4/ui/*.ui)

lyx_add_msvc_pch(frontends_qt4)

lyx_automoc(${frontends_qt4_sources})

lyx_add_ui_files(frontends_qt4_sources ui_files ${frontend_qt4_UI})

add_definitions(
		-DQT_GENUINE_STR
		-DLYX_BUILD_QT4_FRONTEND
		)


include_directories(	
			${TOP_SRC_DIR}/src/frontends/qt4 
			${TOP_SRC_DIR}/src/frontends/controllers
			${CMAKE_CURRENT_BINARY_DIR})

if(MERGE_FILES)
	list(REMOVE_ITEM frontends_qt4_sources ${TOP_SRC_DIR}/src/frontends/qt4/QCommandEdit.cpp)
	list(APPEND frontends_qt4_sources ${TOP_SRC_DIR}/src/frontends/qt4/QCommandEdit.cpp)
	list(REVERSE frontends_qt4_sources)
	list(REMOVE_ITEM frontends_qt4_sources ${TOP_SRC_DIR}/src/frontends/qt4/GuiApplication.cpp)
	list(APPEND frontends_qt4_sources ${TOP_SRC_DIR}/src/frontends/qt4/GuiApplication.cpp)
	lyx_const_touched_files(_allinone frontends_qt4_sources)
	set(depends_moc_uic ${frontends_qt4_headers} ${ui_files})
	SET_SOURCE_FILES_PROPERTIES(_allinone_const.C PROPERTIES OBJECT_DEPENDS "${depends_moc_uic}")
	SET_SOURCE_FILES_PROPERTIES(_allinone_touched.C PROPERTIES OBJECT_DEPENDS "${depends_moc_uic}")
	add_library(frontend_qt4 ${library_type} 
					${_allinone_files}
					${frontends_qt4_headers} 
					${ui_files}
	)
else(MERGE_FILES)	
	add_library(frontend_qt4 ${library_type}  ${frontends_qt4_sources} ${frontends_qt4_headers} ${ui_files})
endif(MERGE_FILES)

target_link_libraries(frontend_qt4 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} controllers)

if(WIN32)
	target_link_libraries(frontend_qt4 Gdi32)
endif(WIN32)

project_source_group("${GROUP_CODE}" frontends_qt4_sources frontends_qt4_headers)
source_group("Uic files" FILES ${frontend_qt4_UI})

