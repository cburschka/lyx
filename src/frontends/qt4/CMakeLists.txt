# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
#

project(frontend_qt)

file(GLOB frontends_qt_sources ${TOP_SRC_DIR}/src/frontends/qt4/${LYX_CPP_FILES})
file(GLOB moc_files ${TOP_SRC_DIR}/src/frontends/qt4/${LYX_MOC_FILES})

list(REMOVE_ITEM frontends_qt_sources ${moc_files} .)
list(REMOVE_ITEM frontends_qt_sources ${TOP_SRC_DIR}/src/frontends/qt4/liblyxqt4.cpp)
list(REMOVE_ITEM frontends_qt_sources ${TOP_SRC_DIR}/src/frontends/qt4/Resources.cpp)

file(GLOB frontends_qt_headers ${TOP_SRC_DIR}/src/frontends/qt4/${LYX_HPP_FILES})
file(GLOB frontend_qt_UI ${TOP_SRC_DIR}/src/frontends/qt4/ui/*.ui)

lyx_add_msvc_pch(frontends_qt4)

lyx_automoc(${frontends_qt_sources})

lyx_add_ui_files(frontends_qt_sources ui_files ${frontend_qt_UI})

set(resource_name ${CMAKE_CURRENT_BINARY_DIR}/Resources.qrc)

add_custom_command(
	OUTPUT	${resource_name}
	COMMAND ${CMAKE_COMMAND}
		-DIMAGES_DIR=${TOP_SRC_DIR}/lib/images
		-DRESOURCE_NAME=${resource_name}
		-DMAPPED_DIR=${TOP_SRC_DIR}/lib/
                -DSUFFIX=${LYX_INSTALL_SUFFIX}
		-P ${TOP_SCRIPT_PATH}/LyXCreateImagesResource.cmake
	)

qt_add_resources(resource_files ${resource_name})

add_definitions(-DQT_GENUINE_STR -DLYX_BUILD_QT_FRONTEND)

include_directories(
	${TOP_SRC_DIR}/src/frontends/qt4
	${QT_INCLUDES}
	${CMAKE_CURRENT_BINARY_DIR})

if(LYX_MERGE_FILES)
	lyx_const_touched_files(_allinone frontends_qt_sources)
	set(depends_moc_uic ${frontends_qt_headers} ${ui_files})
	set_source_files_properties(_allinone_const.C PROPERTIES OBJECT_DEPENDS "${depends_moc_uic}")
	set_source_files_properties(_allinone_touched.C PROPERTIES OBJECT_DEPENDS "${depends_moc_uic}")
	add_library(frontend_qt ${library_type}
		${_allinone_files} ${frontends_qt_headers} ${ui_files} ${resource_files})
else()
	add_library(frontend_qt ${library_type} ${frontends_qt_sources} ${frontends_qt_headers} ${ui_files} ${resource_files})
endif()
set_target_properties(frontend_qt PROPERTIES FOLDER "applications/LyX")

if(Qt5Core_FOUND AND APPLE)
       qt_use_modules(frontend_qt Core Gui Widgets Concurrent MacExtras)
else()
       qt_use_modules(frontend_qt Core Gui Widgets Concurrent)
endif()
target_link_libraries(frontend_qt
	frontends
	${QT_QTCORE_LIBRARY}
	${QT_QTGUI_LIBRARY})

lyx_add_gcc_pch(frontend_qt4)

if(WIN32)
	target_link_libraries(frontend_qt gdi32)
endif()

project_source_group("${GROUP_CODE}" frontends_qt_sources frontends_qt_headers)

source_group("Uic files" FILES ${frontend_qt_UI})

