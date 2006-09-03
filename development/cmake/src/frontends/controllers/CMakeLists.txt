project(controllers)

file(GLOB controllers_sources ${TOP_SRC_DIR}/src/frontends/controllers/*.C)
file(GLOB controllers_headers ${TOP_SRC_DIR}/src/frontends/controllers/*.h)

include_directories(${TOP_SRC_DIR}/src/frontends/controllers)

add_library(controllers STATIC ${controllers_sources} ${controllers_headers})

target_link_libraries(controllers boost_regex boost_filesystem)

source_group("${CODE_GROUP_NAME}" FILES ${controllers_sources} ${controllers_headers})

