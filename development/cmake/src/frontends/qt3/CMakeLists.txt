project(frontend_qt3)

file(GLOB frontends_qt3_sources ${TOP_SRC_DIR}/src/frontends/qt3/*.C)
file(GLOB frontends_qt3_headers ${TOP_SRC_DIR}/src/frontends/qt3/*.h)
file(GLOB frontend_qt3_UI       ${TOP_SRC_DIR}/src/frontends/qt3/ui/*.ui)

lyx_add_ui_files(dummy_gen_in_ui ${frontend_qt3_UI})

QT_WRAP_UI(frontend_qt3 qt3_uic_h qt3_uic_cxx ${frontend_qt3_UI})

qt_wrap_cpp(frontend_qt3 qt3_moc ${frontends_qt3_headers})

source_group("Uic files" FILES ${frontend_qt3_UI})

add_definitions(-DQT_NO_STL)

include_directories(	${QT_INCLUDE_DIR}
			${TOP_SRC_DIR}/src/frontends/qt3
			${TOP_SRC_DIR}/src/frontends/controllers
			${CMAKE_CURRENT_BINARY_DIR})

add_library(frontend_qt3 STATIC 
		${frontends_qt3_sources} 
		${frontends_qt3_headers} 
		${qt3_moc}
		${qt3_uic_cxx}
		${qt3_uic_h}
		)

target_link_libraries(frontend_qt3 ${QT_QT_LIBRARY}  controllers)



if(WIN32)
	target_link_libraries(frontend_qt3 Gdi32)
endif(WIN32)

project_source_group("${GROUP_CODE}" frontends_qt3_sources frontends_qt3_headers)
source_group("Uic files" FILES ${frontend_qt3_UI})

