# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

if(NOT CODE_GROUP_NAME)
set(CODE_GROUP_NAME "The Golden Code")
endif(NOT CODE_GROUP_NAME)

include(LyXPaths)
include(LyXMacros)

if(release)
	set(CMAKE_BUILD_TYPE Release)
	set(release)
endif(release)	


if(qt3)
	set(qt_postfix qt3)
	project(lyx-${qt_postfix})
	set(QT_MT_REQUIRED TRUE)
	find_package(Qt3 REQUIRED)
	set(qt3)
	set(build_qt3_frontend TRUE)
	if(QT_FOUND)
		message("-- Found Qt3: ${QT_QT_LIBRARY}")
	else(QT_FOUND)
		 MESSAGE( FATAL_ERROR "Qt3 not found")
	endif(QT_FOUND)
else(qt3)
	set(qt_postfix qt4)
	project(lyx-${qt_postfix})
	find_package(Qt4 REQUIRED)
	
endif(qt3)	

find_package(ZLIB REQUIRED)
find_package(ICONV REQUIRED)
add_definitions(-DHAVE_ICONV=1)

if(all OR aspell)
	find_package(ASPELL REQUIRED)
else(all OR aspell)
	find_package(ASPELL)
endif(all OR aspell)
set(aspell)

message("")
if(nls OR all)
	add_definitions(-DENABLE_NLS=1)
	message("----- Building with ENABLE_NLS")
else(nls OR all)	
	message("----- No nls, to enable use -Dnls=1")
endif(nls OR all)
set(nls)
if(ASPELL_FOUND)
	add_definitions(-DUSE_ASPELL=1)
	message("----- Building with USE_ASPELL")
else(ASPELL_FOUND)	
	message("----- No aspell, to get more information use -Daspell=1")
endif(ASPELL_FOUND)
message("")
set(all)

# create config.h
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h )


if(MSVC)
	if(MSVC_IDE)
		add_definitions(-DBOOST_USER_CONFIG=&lt\;config.h&gt\;)
	else(MSVC_IDE)
		add_definitions(-DBOOST_USER_CONFIG="<config.h>")
		SET(CMAKE_EXE_LINKER_FLAGS /MANIFEST)
	endif(MSVC_IDE)

	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Zi -wd4996 -wd4800" CACHE STRING "runtime-library flags" FORCE)
	SET(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -wd4996 -wd4800" CACHE STRING "runtime-library flags" FORCE)
	SET(CMAKE_C_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	SET(CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
else(MSVC)
	add_definitions(-DBOOST_USER_CONFIG="<config.h>")
endif(MSVC)

     
include_directories( 
	${CMAKE_BINARY_DIR} 
	${TOP_SRC_DIR}/src 
	${TOP_SRC_DIR}/boost 
	${QT_INCLUDES} 
)

add_subdirectory(boost)
add_subdirectory(intl)
add_subdirectory(src)
