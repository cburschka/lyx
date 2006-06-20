project(lyx-${qt_postfix})

include_directories(${TOP_SRC_DIR}/src)

add_subdirectory(frontends) 
add_subdirectory(graphics) 
add_subdirectory(insets) 
add_subdirectory(mathed) 
add_subdirectory(support) 
#add_subdirectory(tex2lyx) 


configure_file(version.C.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.C)

file(GLOB lyx_sources ${TOP_SRC_DIR}/src/*.C)
file(GLOB lyx_headers ${TOP_SRC_DIR}/src/*.h)

list(REMOVE_ITEM lyx_sources
	${TOP_SRC_DIR}/src/aspell.C
	${TOP_SRC_DIR}/src/pspell.C
	${TOP_SRC_DIR}/src/ispell.C
	${TOP_SRC_DIR}/src/Variables.C
	${TOP_SRC_DIR}/src/Sectioning.C)

if(ASPELL_FOUND)
	include_directories(${ASPELL_INCLUDE_DIR})
	set(lyx_sources ${lyx_sources} ${TOP_SRC_DIR}/src/aspell.C)
endif(ASPELL_FOUND)


include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(lyx-${qt_postfix}
	${lyx_sources} 
	${lyx_headers}
	${CMAKE_CURRENT_BINARY_DIR}/version.C
	)

target_link_libraries(lyx-${qt_postfix}
	mathed
	insets
	frontends
	frontend_${qt_postfix}
	graphics
	support
	intl
	${QT_QTMAIN_LIBRARY}
	boost_iostreams
)


if(ASPELL_FOUND)
	target_link_libraries(lyx-${qt_postfix} ${ASPELL_LIBRARY})
endif(ASPELL_FOUND)
