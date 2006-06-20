project(frontends)

include_directories(${TOP_SRC_DIR}/src/frontends)

add_subdirectory(controllers) 
add_subdirectory(${qt_postfix}) 



file(GLOB frontends_sources ${TOP_SRC_DIR}/src/frontends/*.C)
file(GLOB frontends_headers ${TOP_SRC_DIR}/src/frontends/*.h)

add_library(frontends STATIC ${frontends_sources} ${frontends_headers})



