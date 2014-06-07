#
#  Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  1. Redistributions of source code must retain the copyright
#	  notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the copyright
#	  notice, this list of conditions and the following disclaimer in the
#	  documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote products
#	  derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
#  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

include (MacroAddFileDependencies)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

macro(lyx_add_path _list _prefix)
	set(_tmp)
	foreach(_current ${${_list}})
		set(_tmp ${_tmp} ${_prefix}/${_current})
	endforeach(_current)
	set(${_list} ${_tmp})
endmacro(lyx_add_path _out _prefix)


#create the implementation files from the ui files and add them
#to the list of sources
#usage: LYX_ADD_QT4_UI_FILES(foo_SRCS ${ui_files})
macro(LYX_ADD_UI_FILES _sources _ui_files)
	set(uifiles})
	foreach (_current_FILE ${ARGN})
		get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
		get_filename_component(_basename ${_tmp_FILE} NAME_WE)
		set(_header ${CMAKE_CURRENT_BINARY_DIR}/ui_${_basename}.h)

		# we need to run uic and replace some things in the generated file
		# this is done by executing the cmake script LyXuic.cmake
		# ######
		# Latest test showed on linux and windows show no bad consequeces,
		# so we removed the call to LyXuic.cmake
		qt_wrap_uifiles(${_header} ${_tmp_FILE} OPTIONS -tr lyx::qt_)
		list(APPEND uifiles ${_header})
	endforeach()
	set(${_ui_files} ${uifiles})
endmacro(LYX_ADD_UI_FILES)



macro(LYX_AUTOMOC)
	if (QT4_GET_MOC_INC_DIRS)
		QT4_GET_MOC_INC_DIRS(_moc_INCS)
	endif()

	set(_matching_FILES)
	foreach (_current_FILE ${ARGN})

		get_filename_component(_abs_FILE ${_current_FILE} ABSOLUTE)
		# if "SKIP_AUTOMOC" is set to true, we will not handle this file here.
		# here. this is required to make bouic work correctly:
		# we need to add generated .cpp files to the sources (to compile them),
		# but we cannot let automoc handle them, as the .cpp files don't exist yet when
		# cmake is run for the very first time on them -> however the .cpp files might
		# exist at a later run. at that time we need to skip them, so that we don't add two
		# different rules for the same moc file
		get_source_file_property(_skip ${_abs_FILE} SKIP_AUTOMOC)

		if (EXISTS ${_abs_FILE} AND NOT _skip)

			file(READ ${_abs_FILE} _contents)

			get_filename_component(_abs_PATH ${_abs_FILE} PATH)

			string(REGEX MATCHALL "#include +[\"<]moc_[^ ]+\\.cpp[\">]" _match "${_contents}")
			if (_match)
				foreach (_current_MOC_INC ${_match})
					string(REGEX MATCH "moc_[^ <\"]+\\.cpp" _current_MOC "${_current_MOC_INC}")

					get_filename_component(_basename ${_current_MOC} NAME_WE)

					string(LENGTH ${_basename} _length)
					MATH(EXPR _mocless_length ${_length}-4)
					STRING(SUBSTRING  ${_basename} 4 ${_mocless_length} _mocless_name )

					set(_header ${_abs_PATH}/${_mocless_name}.h)

					#message(STATUS "moc : ${_header}")
					#set(_header ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.h)
					#set(_header ${_abs_PATH}/${_basename}.h)

					set(_moc  ${CMAKE_CURRENT_BINARY_DIR}/${_current_MOC})
					if (WIN32)
							  set(_def -D_WIN32)
					endif()
					#set(_moc ${_abs_PATH}/${_current_MOC})
					add_custom_command(OUTPUT ${_moc}
							  COMMAND ${QT_MOC_EXECUTABLE}
							  ARGS "-DQT_VERSION=${QT4_VERSION}" ${_def} ${_moc_INCS} ${_header} -o ${_moc}
							  MAIN_DEPENDENCY ${_header})
					macro_add_file_dependencies(${_abs_FILE} ${_moc})
					SET_SOURCE_FILES_PROPERTIES(${_moc} GENERATED)
				endforeach (_current_MOC_INC)
			else()
				#message(STATUS "moc not found : ${_abs_FILE} ")
			endif()
		endif()
	endforeach (_current_FILE)
endmacro (LYX_AUTOMOC)


macro(lyx_const_touched_files _allinone_name _list)
	set(_file_list ${_allinone_name}_files)
	set(_file_const ${CMAKE_CURRENT_BINARY_DIR}/${_allinone_name}_const.C)
	set(_file_touched ${CMAKE_CURRENT_BINARY_DIR}/${_allinone_name}_touched.C)

	# don't touch exisiting or non-empty file,
	# so a cmake re-run doesn't touch all created files
	set(_rebuild_file_const 0)
	if (NOT EXISTS ${_file_const})
		set(_rebuild_file_const 1)
	else()
		FILE(READ ${_file_const} _file_content)
		if (NOT _file_content)
			set(_rebuild_file_const 1)
		endif()
	endif()

	set(_rebuild_file_touched 0)
	if (NOT EXISTS ${_file_touched})
		set(_rebuild_file_touched 1)
	else()
		FILE(READ ${_file_touched} _file_content)
		if (NOT _file_content)
			set(_rebuild_file_touched 1)
		endif()
	endif()

	if (LYX_MERGE_REBUILD)
		#message(STATUS "Merge files build: rebuilding generated files")
		set(_rebuild_file_const 1)
		set(_rebuild_file_touched 1)
	endif()

	if (_rebuild_file_const)
		file(WRITE  ${_file_const} "// autogenerated file \n//\n")
		file(APPEND ${_file_const} "//	 * clear or delete this file to build it again by cmake \n//\n\n")
	endif()

	if (_rebuild_file_touched)
		file(WRITE  ${_file_touched} "// autogenerated file \n//\n")
		file(APPEND ${_file_touched} "//	 * clear or delete this file to build it again by cmake \n//\n")
		file(APPEND ${_file_touched} "//	 * don't touch this file \n//\n\n")
		file(APPEND ${_file_touched} "#define DONT_INCLUDE_CONST_FILES\n")
		file(APPEND ${_file_touched} "#include \"${_file_const}\"\n\n\n")
	endif()

	#add merged files also to the project so they become editable
		if(${GROUP_CODE} MATCHES "flat")
		lyx_add_info_files_no_group(${${_list}})
	else()
		lyx_add_info_files(MergedFiles ${${_list}})
	endif()
	
	set(${_file_list} ${_file_const} ${_file_touched} ${lyx_${groupname}_info_files})

	foreach (_current_FILE ${${_list}})
		get_filename_component(_abs_FILE ${_current_FILE} ABSOLUTE)
		# don't include any generated files in the final-file
		# because then cmake will not know the dependencies
		get_source_file_property(_isGenerated ${_abs_FILE} GENERATED)
		if (_isGenerated)
			list(APPEND ${_file_list} ${_abs_FILE})
		else()
		  GET_FILENAME_COMPONENT(_file_name ${_abs_FILE} NAME_WE)
		  STRING(REGEX REPLACE "-" "_" _file_name "${_file_name}")
		  set(__macro_name ${_file_name}___ASSUME_CONST)

		  if (_rebuild_file_const)
			  file(APPEND ${_file_const}  "#define ${__macro_name}\n")
			  file(APPEND ${_file_const}  "#if defined(${__macro_name}) && !defined(DONT_INCLUDE_CONST_FILES)\n")
			  file(APPEND ${_file_const}  "#include \"${_abs_FILE}\"\n")
			  file(APPEND ${_file_const}  "#endif\n\n")
		  endif()

		  if (_rebuild_file_touched)
			  file(APPEND ${_file_touched}  "#ifndef ${__macro_name}\n")
			  file(APPEND ${_file_touched}  "#include \"${_abs_FILE}\"\n")
			  file(APPEND ${_file_touched}  "#endif\n\n")
		  endif()
		endif()
	endforeach (_current_FILE)
endmacro(lyx_const_touched_files)

macro(LYX_OPTION_INIT)
	set(LYX_OPTIONS)
endmacro()


macro(LYX_OPTION _name _description _default _sys)
	set(_msg OFF)
	if(${_sys} MATCHES "GCC")
		set(_system CMAKE_COMPILER_IS_GNUCXX)
	elseif(${_sys} MATCHES "MAC")
		set(_system APPLE)
	else()
		set(_system ${_sys})
	endif()
	if(${_system} MATCHES "ALL")
		option(LYX_${_name} ${_description} ${_default})
		set(_msg ON)
	else()
		if(${${_system}})
			option(LYX_${_name} ${_description} ${_default})
			set(_msg ON)
		endif()
	endif()
	list(APPEND LYX_OPTIONS LYX_${_name})
	set(LYX_${_name}_description ${_description})
	set(LYX_${_name}_show_message ${_msg})
endmacro()


macro(LYX_OPTION_LIST_ALL)
	if(UNIX)
		set(run_cmake ${CMAKE_BINARY_DIR}/run_cmake.sh)
		file(WRITE ${run_cmake} "#!/bin/bash \n")
		execute_process(COMMAND chmod 755 ${run_cmake})
		set(cont "\\\n")
	elseif(WIN32)
		set(run_cmake ${CMAKE_BINARY_DIR}/run_cmake.bat)
		file(WRITE ${run_cmake} "")
		set(cont "<nul ^\n")
	endif()
	file(APPEND ${run_cmake} "cmake ${CMAKE_SOURCE_DIR}  ${cont}")
	file(APPEND ${run_cmake} " -G\"${CMAKE_GENERATOR}\"  ${cont}")
	foreach(_option ${LYX_OPTIONS})
		if(${_option}_show_message OR ${ARGV0} STREQUAL "help")
			string(SUBSTRING "${_option}                            " 0 25 _var)
			if(${_option})
				set(_isset ON)
			else()
				set(_isset OFF)
			endif()
			string(SUBSTRING "${_isset}     " 0 4 _val)
			message(STATUS "${_var}= ${_val}   : ${${_option}_description}")
			file(APPEND ${run_cmake} " -D${_option}=${${_option}}  ${cont}")
		endif()
	endforeach()
	file(APPEND ${run_cmake} "\n")
	message(STATUS)
	message(STATUS "CMake command with options is available in shell script")
	message(STATUS "    '${run_cmake}'")
endmacro()

macro(lyx_add_info_files group)
	foreach(_it ${ARGN})
		if(NOT IS_DIRECTORY ${_it})
			get_filename_component(name ${_it} NAME)
			if(NOT ${_it} MATCHES "^/\\\\..*$;~$")
				set_source_files_properties(${_it} PROPERTIES HEADER_FILE_ONLY TRUE)
				set(lyx_${group}_info_files ${lyx_${group}_info_files} ${_it})
			endif()
		endif()
	endforeach()
	set(check_group ${group}) #cmake bug?
	if(check_group)
		source_group(${group} FILES ${lyx_${group}_info_files})
	endif()
	set(lyx_info_files ${lyx_info_files} ${lyx_${group}_info_files})
endmacro()

macro(lyx_add_info_files_no_group)
	lyx_add_info_files( "" ${ARGN})
endmacro()

macro(lyx_find_info_files group files)
	file(GLOB _filelist ${files})
	lyx_add_info_files(${group} ${_filelist})
endmacro()

macro(settestlabel testname)
  get_property(_lab_list TEST ${testname} PROPERTY LABELS)
  if(_lab_list)
    list(APPEND _lab_list "${ARGN}")
  else()
    set(_lab_list "${ARGN}")
  endif()
  list(REMOVE_DUPLICATES _lab_list)
  set_tests_properties(${testname} PROPERTIES LABELS "${_lab_list}")
endmacro()

macro(setmarkedtestlabel testname reverted)
  if(reverted)
    settestlabel(${testname} "reverted" ${ARGN})
  else()
    settestlabel(${testname} ${ARGN})
  endif()
endmacro()

