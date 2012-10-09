if(WIN32)
	find_library(ENCHANT_LIBRARY "libenchant")
	find_path(ENCHANT_INCLUDE_DIR "enchant++.h")
else()
  find_library(ENCHANT_LIBRARY "enchant"  
	"/usr/local/lib" 
	${SYSTEM_LIB_DIRS}
	"/opt/local/lib")
  find_path(ENCHANT_INCLUDE_DIR "enchant++.h" PATHS 
		/usr/local/include
		/usr/local/include/enchant
		/usr/include
		/usr/include/enchant
		/opt/local/include/enchant)
endif()

if (ENCHANT_LIBRARY AND ENCHANT_INCLUDE_DIR)
	set(ENCHANT_FOUND TRUE)
	message(STATUS "Building with USE_ENCHANT")
else()
	message(STATUS "Enchant not found, building without enchant support")
endif()


# handle the QUIETLY and REQUIRED arguments and 
# set ENCHANT_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ENCHANT DEFAULT_MSG ENCHANT_LIBRARY ENCHANT_INCLUDE_DIR)

if(Enchant_FIND_REQUIRED AND NOT ENCHANT_FOUND)
	message(FATAL_ERROR "Could not find Enchant library")
endif()

mark_as_advanced(ENCHANT_LIBRARY ENCHANT_INCLUDE_DIR)
