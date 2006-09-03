project(insets)

file(GLOB insets_sources ${TOP_SRC_DIR}/src/insets/*.C)
file(GLOB insets_headers ${TOP_SRC_DIR}/src/insets/*.h)

list(REMOVE_ITEM insets_sources ${TOP_SRC_DIR}/src/insets/insettheorem.C)

include_directories(${TOP_SRC_DIR}/src/insets)

add_library(insets STATIC ${insets_sources} ${insets_headers})

project_source_group("${GROUP_CODE}" insets_sources insets_headers)

