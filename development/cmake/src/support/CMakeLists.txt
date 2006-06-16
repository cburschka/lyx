project(support)

configure_file(package.C.cmake ${CMAKE_CURRENT_BINARY_DIR}/package.C)

file(GLOB support_sources ${TOP_SRC_DIR}/src/support/*.C)
file(GLOB support_headers ${TOP_SRC_DIR}/src/support/*.h)

list(REMOVE_ITEM support_sources 	
	${TOP_SRC_DIR}/src/support/os_win32.C
	${TOP_SRC_DIR}/src/support/os_unix.C
	${TOP_SRC_DIR}/src/support/os_cygwin.C
	${TOP_SRC_DIR}/src/support/os_os2.C
	${TOP_SRC_DIR}/src/support/atexit.c)
	

include_directories(${TOP_SRC_DIR}/src/support)

add_library(support STATIC 
	${support_sources} 
	${support_headers} 
	${CMAKE_CURRENT_BINARY_DIR}/package.C)

target_link_libraries(support boost_signals)

if(WIN32)
	target_link_libraries(support shlwapi)
endif(WIN32)



