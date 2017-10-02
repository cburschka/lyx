project(${_convert})

include_directories(${QT_INCLUDES})
add_definitions(-DQT_NO_CAST_TO_ASCII -DQT_NO_STL)
add_executable(${_convert} "${TOP_SRC_DIR}/src/convert/lyxconvert.cpp")

qt_use_modules(${_convert} Core Widgets)
if(APPLE)
  target_link_libraries(${_convert} "-framework AppKit")
endif()

install(TARGETS ${_convert} DESTINATION ${LYX_UTILITIES_INSTALL_PATH})
