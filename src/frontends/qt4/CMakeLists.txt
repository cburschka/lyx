# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
#

project(frontend_qt4)

file(GLOB frontends_qt4_sources ${TOP_SRC_DIR}/src/frontends/qt4/${LYX_CPP_FILES})
file(GLOB moc_files ${TOP_SRC_DIR}/src/frontends/qt4/${LYX_MOC_FILES})

list(REMOVE_ITEM frontends_qt4_sources ${moc_files} .)
list(REMOVE_ITEM frontends_qt4_sources ${TOP_SRC_DIR}/src/frontends/qt4/liblyxqt4.cpp)
list(REMOVE_ITEM frontends_qt4_sources ${TOP_SRC_DIR}/src/frontends/qt4/Resources.cpp)

file(GLOB frontends_qt4_headers ${TOP_SRC_DIR}/src/frontends/qt4/${LYX_HPP_FILES})
file(GLOB frontend_qt4_UI ${TOP_SRC_DIR}/src/frontends/qt4/ui/*.ui)

lyx_add_msvc_pch(frontends_qt4)

lyx_automoc(${frontends_qt4_sources})

lyx_add_ui_files(frontends_qt4_sources ui_files ${frontend_qt4_UI})

file(GLOB_RECURSE frontend_qt4_images_png      ${TOP_SRC_DIR}/lib/images/*.png)
file(GLOB_RECURSE frontend_qt4_images_gif      ${TOP_SRC_DIR}/lib/images/*.gif)
set(resource_name ${CMAKE_CURRENT_BINARY_DIR}/Resources.qrc)
set(frontend_qt4_images ${frontend_qt4_images_png} ${frontend_qt4_images_gif})

#lyx_qt_resources_file(${resource_name} "${TOP_SRC_DIR}/lib/" frontend_qt4_images)

add_custom_command(
	OUTPUT	${resource_name}
	COMMAND ${CMAKE_COMMAND}
		-DIMAGES_DIR=${TOP_SRC_DIR}/lib/images
		-DRESOURCE_NAME=${resource_name}
		-DMAPPED_DIR=${TOP_SRC_DIR}/lib/
		-P ${TOP_SRC_DIR}/development/cmake/modules/LyXCreateImagesResource.cmake
	)

add_custom_target(Resources.qrc DEPENDS ${resource_name})

qt4_add_resources(resource_files ${resource_name})


add_definitions(-DQT_GENUINE_STR -DLYX_BUILD_QT4_FRONTEND)

include_directories(
	${TOP_SRC_DIR}/src/frontends/qt4
	${QT_INCLUDES}
	${CMAKE_CURRENT_BINARY_DIR})

if(LYX_MERGE_FILES)
	lyx_const_touched_files(_allinone frontends_qt4_sources)
	set(depends_moc_uic ${frontends_qt4_headers} ${ui_files})
	set_source_files_properties(_allinone_const.C PROPERTIES OBJECT_DEPENDS "${depends_moc_uic}")
	set_source_files_properties(_allinone_touched.C PROPERTIES OBJECT_DEPENDS "${depends_moc_uic}")
	add_library(frontend_qt4 ${library_type}
		${_allinone_files} ${frontends_qt4_headers} ${ui_files} ${resource_files})
else()
	add_library(frontend_qt4 ${library_type} ${frontends_qt4_sources} ${frontends_qt4_headers} ${ui_files} ${resource_files})
endif()


target_link_libraries(frontend_qt4
	frontends
	${QT_QTCORE_LIBRARY}
	${QT_QTGUI_LIBRARY})

lyx_add_gcc_pch(frontend_qt4)

if(WIN32)
	target_link_libraries(frontend_qt4 Gdi32)
endif()

project_source_group("${GROUP_CODE}" frontends_qt4_sources frontends_qt4_headers)

source_group("Uic files" FILES ${frontend_qt4_UI})

