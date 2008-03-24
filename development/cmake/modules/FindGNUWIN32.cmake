#
#  from kdelibs
#

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

if(WIN32)

	# check if GNUWIN32_DIR is already set 
	# (e.g. by command line argument or the calling script)
	if(NOT GNUWIN32_DIR)
		# check for enviroment variable
		file(TO_CMAKE_PATH "$ENV{GNUWIN32_DIR}" GNUWIN32_DIR)
		if(NOT GNUWIN32_DIR)
			# search in the default program install folder
			file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _progFiles)
			find_file(GNUWIN32_DIR_tmp gnuwin32 win32libs 
					PATHS
					"${_progFiles}"
					"C:/" "D:/" "E:/" "F:/" "G:/"
			)
			set(GNUWIN32_DIR ${GNUWIN32_DIR_tmp})
		endif()
	else()
		set(GNUWIN32_DIR ${GNUWIN32_DIR} CACHE TYPE STRING)
	endif()

	if(GNUWIN32_DIR)
		 set(GNUWIN32_INCLUDE_DIR ${GNUWIN32_DIR}/include)
		 set(GNUWIN32_LIBRARY_DIR ${GNUWIN32_DIR}/lib)
		 set(GNUWIN32_BINARY_DIR  ${GNUWIN32_DIR}/bin)
		 set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${GNUWIN32_INCLUDE_DIR})
		 set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${GNUWIN32_LIBRARY_DIR})
		 set(GNUWIN32_FOUND TRUE)
	else()
		 set(GNUWIN32_FOUND)
	endif()

	if(GNUWIN32_FOUND)
		if (NOT GNUWIN32_FIND_QUIETLY)
			message(STATUS "Found GNUWIN32: ${GNUWIN32_DIR}")
		endif()
	else()
		if (GNUWIN32_FIND_REQUIRED)
			message(SEND_ERROR "Could NOT find GNUWIN32, please set GNUWIN32_DIR")
		endif()
	endif()

endif()

