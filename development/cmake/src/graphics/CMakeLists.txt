project(graphics)

file(GLOB graphics_sources ${TOP_SRC_DIR}/src/graphics/*.C)
file(GLOB graphics_headers ${TOP_SRC_DIR}/src/graphics/*.h)

include_directories(${TOP_SRC_DIR}/src/graphics)

add_library(graphics STATIC ${graphics_sources} ${graphics_headers})

project_source_group("${GROUP_CODE}" graphics_sources graphics_headers)

