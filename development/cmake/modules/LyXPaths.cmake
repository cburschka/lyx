if(WIN32)

find_package(GNUWIN32 REQUIRED)

file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _prog_path)

set(_zlib_path ${_prog_path}/zlib)
set(_iconv_path ${_prog_path}/iconv)

set(_gnuwin32_dir ${GNUWIN32_DIR})

set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} 
	${_gnuwin32_dir}/include
	${_zlib_path}/include
	${_iconv_path}/include
	)
	
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} 
	${_gnuwin32_dir}/lib
	${_zlib_path}/lib
	${_iconv_path}/lib
	)

#chek_include_files path
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${_gnuwin32_dir}/include)


else(WIN32)

	set(_prog_path ~/bin)
	
endif(WIN32)


GET_FILENAME_COMPONENT(lyx_dir_readme ${CMAKE_SOURCE_DIR}/../../README ABSOLUTE)
GET_FILENAME_COMPONENT(TOP_SRC_DIR ${lyx_dir_readme} PATH)

if(WIN32)
	set(locale_dir Resources/locale)
else(WIN32)	
	set(locale_dir share/locale)
endif(WIN32)

set(PREFIX ${_prog_path}/LyX)
set(LOCAL_DIR ${PREFIX}/${locale_dir})

#message(" PREFIX : ${PREFIX}")
#message(" LOCAL_DIR : ${LOCAL_DIR}")
#message(" TOP_SRC_DIR : ${TOP_SRC_DIR}")



