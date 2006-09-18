project(aspell)


########################################################################
#
# Aspell Program
#

set(aspell_SOURCES 
  ../../prog/aspell.cpp 
  ../../prog/check_funs.cpp 
  ../../prog/checker_string.cpp
  )
  
add_executable(${ASPELL_EXE_NAME} ${aspell_SOURCES})

if(WIN32)
	target_link_libraries(${ASPELL_EXE_NAME} ${ASPELL_LIB_NAME} ole32)
endif(WIN32)

########################################################################
#
# Install
#

set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/../gnuwin32)

install(TARGETS ${ASPELL_EXE_NAME}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
	
install(FILES ../interfaces/cc/aspell.h 
	DESTINATION include)
	

