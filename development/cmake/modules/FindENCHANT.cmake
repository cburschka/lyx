if(WIN32)
	find_library(ENCHANT_LIBRARY "libenchant")
	find_path(ENCHANT_INCLUDE_DIR "enchant++.h")
else()
  find_library(ENCHANT_LIBRARY NAMES "enchant-2" "enchant"
	"/usr/local/lib" 
	${SYSTEM_LIB_DIRS}
	"/opt/local/lib")
  find_path(ENCHANT_INCLUDE_DIR "enchant++.h" PATHS 
		/usr/local/include/enchant-2
		/usr/include/enchant-2
		/opt/local/include/enchant-2
		/usr/local/include
		/usr/local/include/enchant
		/usr/include
		/usr/include/enchant
		/opt/local/include/enchant)
endif()

# handle the QUIETLY and REQUIRED arguments and
# set ENCHANT_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ENCHANT DEFAULT_MSG ENCHANT_LIBRARY ENCHANT_INCLUDE_DIR)

mark_as_advanced(ENCHANT_LIBRARY ENCHANT_INCLUDE_DIR)
