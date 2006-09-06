project(boost_filesystem)

set(boost_filesystem_sources
	exception.cpp
	operations.cpp
	path.cpp
)

lyx_add_path(boost_filesystem_sources ${TOP_SRC_DIR}/boost/libs/filesystem/src)

add_library(boost_filesystem STATIC ${boost_filesystem_sources})