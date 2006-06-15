project(boost_filesystem)

set(boost_filesystem_sources
	convenience.cpp
	exception.cpp
	operations_posix_windows.cpp
	path_posix_windows.cpp
)

lyx_add_path(boost_filesystem_sources ${TOP_SRC_DIR}/boost/libs/filesystem/src)

add_library(boost_filesystem STATIC ${boost_filesystem_sources})