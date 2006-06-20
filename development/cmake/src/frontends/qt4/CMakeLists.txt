project(frontend_qt4)


file(GLOB frontends_qt4_sources ${TOP_SRC_DIR}/src/frontends/qt4/*.C)
file(GLOB frontends_qt4_headers ${TOP_SRC_DIR}/src/frontends/qt4/*.h)
file(GLOB frontend_qt4_UI       ${TOP_SRC_DIR}/src/frontends/qt4/ui/*.ui)


lyx_automoc(${frontends_qt4_sources})

lyx_add_ui_files(frontends_qt4_sources ${frontend_qt4_UI})

source_group("Uic files" FILES ${frontend_qt4_UI})

add_definitions(-DQT3_SUPPORT -DQT_NO_STL)

include_directories(	${TOP_SRC_DIR}/src/frontends/qt4 
			${TOP_SRC_DIR}/src/frontends/controllers
			${CMAKE_CURRENT_BINARY_DIR})

add_library(frontend_qt4 STATIC ${frontends_qt4_sources} ${frontends_qt4_headers})

target_link_libraries(frontend_qt4 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} controllers)

if(WIN32)
	target_link_libraries(frontend_qt4 Gdi32)
endif(WIN32)

