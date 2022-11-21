# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
#

project(frontend_qt)

file(GLOB frontends_qt_sources ${TOP_SRC_DIR}/src/frontends/qt/${LYX_CPP_FILES})
if (LYX_USE_FILEDIALOG MATCHES "NATIVE")
  set_property(SOURCE "${TOP_SRC_DIR}/src/frontends/qt/FileDialog.cpp" PROPERTY COMPILE_FLAGS -DUSE_NATIVE_FILEDIALOG=1)
endif()
file(GLOB moc_files ${TOP_SRC_DIR}/src/frontends/qt/${LYX_MOC_FILES})

list(REMOVE_ITEM frontends_qt_sources ${moc_files} .)

file(GLOB frontends_qt_headers ${TOP_SRC_DIR}/src/frontends/qt/${LYX_HPP_FILES})
file(GLOB frontend_qt_UI ${TOP_SRC_DIR}/src/frontends/qt/ui/*.ui)

lyx_add_msvc_pch(frontends_qt)

lyx_automoc(${frontends_qt_sources})

lyx_add_ui_files(frontends_qt_sources ui_files ${frontend_qt_UI})

add_definitions(-DQT_GENUINE_STR -DLYX_BUILD_QT_FRONTEND)

include_directories(
	${TOP_SRC_DIR}/src/frontends/qt
	${QT_INCLUDES}
	${CMAKE_CURRENT_BINARY_DIR})

if (LYX_USE_QT MATCHES "QT6")
        include_directories(${Qt6Gui_PRIVATE_INCLUDE_DIRS})
endif()

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
set_target_properties(frontend_qt PROPERTIES FOLDER "applications/LyX" QT_NO_UNICODE_DEFINES TRUE)

if(Qt6Core_FOUND)
       qt_use_modules(frontend_qt Core Gui Widgets Concurrent Svg SvgWidgets)
elseif(Qt5Core_FOUND AND APPLE)
       qt_use_modules(frontend_qt Core Gui Widgets Concurrent Svg MacExtras)
else()
       if (Qt5WinExtras_FOUND)
           qt_use_modules(frontend_qt Core Gui Widgets Concurrent Svg WinExtras)
       else()
           qt_use_modules(frontend_qt Core Gui Widgets Concurrent Svg)
       endif()
endif()
target_link_libraries(frontend_qt
	frontends
	${QT_QTCORE_LIBRARY}
	${QT_QTGUI_LIBRARY})

lyx_add_gcc_pch(frontend_qt)

if(WIN32)
	target_link_libraries(frontend_qt gdi32)
endif()

project_source_group("${GROUP_CODE}" frontends_qt_sources frontends_qt_headers)

source_group("Uic files" FILES ${frontend_qt_UI})

