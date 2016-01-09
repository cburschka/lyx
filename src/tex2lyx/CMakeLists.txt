# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c) 2008-2011 Kornel Benko, <Kornel.Benko@berlin.de>
#

project(${_tex2lyx})

# There is no header file lengthcommon.h
set(LINKED_sources ${TOP_SRC_DIR}/src/lengthcommon.cpp)
set(LINKED_headers)

foreach(_src graphics/GraphicsParams insets/ExternalTemplate
	insets/ExternalTransforms insets/InsetLayout Author Color Counters
	Encoding FloatList Floating FontInfo LaTeXPackages Layout
	LayoutFile LayoutModuleList Length Lexer ModuleList TextClass
	Spacing version)
	list(APPEND LINKED_sources ${TOP_SRC_DIR}/src/${_src}.cpp)
	list(APPEND LINKED_headers ${TOP_SRC_DIR}/src/${_src}.h)
endforeach(_src)

file(GLOB tex2lyx_sources ${TOP_SRC_DIR}/src/tex2lyx/${LYX_CPP_FILES})

file(GLOB tex2lyx_headers ${TOP_SRC_DIR}/src/tex2lyx/${LYX_HPP_FILES})

include_directories((BEFORE
	${TOP_SRC_DIR}/src/tex2lyx)
	${ZLIB_INCLUDE_DIR})

if(WIN32 AND NOT MINGW)
	set(FILE_RC ${TOP_CMAKE_PATH}/lyx.rc)
	message(STATUS "Using icon defined in resource file: ${FILE_RC}")
endif()

if(NOT LYX_MERGE_FILES)
	add_executable(${_tex2lyx} ${tex2lyx_sources} ${LINKED_sources} ${tex2lyx_headers} ${LINKED_headers} 	${FILE_RC})
else()
	lyx_const_touched_files(_allinone  tex2lyx_sources)
	lyx_const_touched_files(_allinone_linked  LINKED_sources)
	add_executable(${_tex2lyx} ${_allinone_files} ${_allinone_linked_files} ${FILE_RC})
endif()
set_target_properties(${_tex2lyx} PROPERTIES FOLDER "applications/TeX2LyX")

target_link_libraries(${_tex2lyx}
	support
	${Lyx_Boost_Libraries}
	${QT_QTCORE_LIBRARY}
	${QT_QTGUI_LIBRARY}
	${ICONV_LIBRARY})

lyx_target_link_libraries(${_tex2lyx} Magic)

add_dependencies(${_tex2lyx} lyx_version)

if(WIN32)
	target_link_libraries(${_tex2lyx} shlwapi ole32 psapi)
endif()

if(CYGWIN)
	target_link_libraries(${_tex2lyx} shlwapi ole32)
endif()

if(APPLE)
	target_link_libraries(${_tex2lyx} "-framework AppKit")
endif()


project_source_group("${GROUP_CODE}" tex2lyx_sources tex2lyx_headers)

add_subdirectory(test)

install(TARGETS ${_tex2lyx} DESTINATION ${LYX_UTILITIES_INSTALL_PATH})

