project(lyx)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

include(LyXPaths)
include(LyXMacros)

find_package(Qt4 REQUIRED)
find_package(ZLIB REQUIRED)


if(all OR nls)
	find_package(ICONV REQUIRED)
else(all OR nls)
	find_package(ICONV)
endif(all OR nls)
set(nls)

if(all OR aspell)
	find_package(ASPELL REQUIRED)
else(all OR aspell)
	find_package(ASPELL)
endif(all OR aspell)
set(aspell)

message("")
if(ICONV_FOUND)
	add_definitions(-DENABLE_NLS=1 -DHAVE_ICONV=1)
	message("----- Building with ENABLE_NLS and HAVE_ICONV")
else(ICONV_FOUND)
	message("----- No iconv found, to get more information use -Dnls=1")
endif(ICONV_FOUND)
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
