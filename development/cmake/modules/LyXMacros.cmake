
macro(lyx_add_path _list _prefix)
	set(_tmp)
   	foreach(_current ${${_list}})
   		set(_tmp ${_tmp} ${_prefix}/${_current})
   		#message( ${_prefix}/${_current})
   	endforeach(_current)  	 
   	set(${_list} ${_tmp})  
endmacro(lyx_add_path _out _prefix)


#create the implementation files from the ui files and add them to the list of sources
#usage: LYX_ADD_QT4_UI_FILES(foo_SRCS ${ui_files})
macro (LYX_ADD_UI_FILES _sources )
   foreach (_current_FILE ${ARGN})

      get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_tmp_FILE} NAME_WE)
      set(_header ${CMAKE_CURRENT_BINARY_DIR}/ui/${_basename}.h)

      # we need to run uic and replace some things in the generated file
      # this is done by executing the cmake script kde4uic.cmake
      add_custom_command(OUTPUT ${_header}
         COMMAND ${CMAKE_COMMAND}
         ARGS
         -DKDE4_HEADER:BOOL=ON
         -DKDE_UIC_EXECUTABLE:FILEPATH=${QT_UIC_EXECUTABLE}
         -DKDE_UIC_FILE:FILEPATH=${_tmp_FILE}
         -DKDE_UIC_H_FILE:FILEPATH=${_header}
         -DKDE_UIC_BASENAME:STRING=${_basename}
         -P ${CMAKE_MODULE_PATH}/LyXuic.cmake
         MAIN_DEPENDENCY ${_tmp_FILE}
      )
      set(${_sources} ${${_sources}} ${_header})
   endforeach (_current_FILE)
endmacro (LYX_ADD_UI_FILES)



MACRO (LYX_AUTOMOC)
   if(QT4_GET_MOC_INC_DIRS)
      QT4_GET_MOC_INC_DIRS(_moc_INCS)
   endif(QT4_GET_MOC_INC_DIRS)

   set(_matching_FILES )
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

         string(REGEX MATCHALL "#include +[^ ]+_moc\\.cpp[\">]" _match "${_contents}")
         if (_match)
            foreach (_current_MOC_INC ${_match})
               string(REGEX MATCH "[^ <\"]+_moc\\.cpp" _current_MOC "${_current_MOC_INC}")

               get_filename_component(_basename ${_current_MOC} NAME_WE)               
         		
				string(LENGTH ${_basename} _length)
				MATH(EXPR _mocless_length ${_length}-4)
				STRING(SUBSTRING  ${_basename} 0 ${_mocless_length} _mocless_name )
   
          set(_header ${_abs_PATH}/${_mocless_name}.h)
          
          #message(STATUS "moc : ${_header}")



               #set(_header ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.h)
               #set(_header ${_abs_PATH}/${_basename}.h)
               set(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_current_MOC})
               
   #set(_moc    ${_abs_PATH}/${_current_MOC})
       add_custom_command(OUTPUT ${_moc}
        COMMAND ${QT_MOC_EXECUTABLE}
        ARGS ${_moc_INCS} ${_header} -o ${_moc}
        MAIN_DEPENDENCY ${_header}
     )
               macro_add_file_dependencies(${_abs_FILE} ${_moc})

            endforeach (_current_MOC_INC)
         else(_match)
         	#message(STATUS "moc not found : ${_abs_FILE} ")
         endif (_match)

      endif (EXISTS ${_abs_FILE} AND NOT _skip)
   endforeach (_current_FILE)
endmacro (LYX_AUTOMOC)



# folders in the project
# mode==flat  : no folders
# mode==split : standart behavior of cmake, split heders and sources
# mode== <other values" : code is in this folder
macro(project_source_group mode sources headers)
	#message(STATUS ${mode})
	#message(STATUS ${sources} ${headers})
	if(${mode} MATCHES "flat")
		source_group("Source Files" Files)
		source_group("Header Files" Files)
	else(${mode} MATCHES "flat")
		if(NOT ${mode} MATCHES "split")
			source_group("${mode}" FILES ${${sources}} ${${headers}})
			source_group("cmake" FILES CMakeLists.txt)
		endif(NOT ${mode} MATCHES "split")
	endif(${mode} MATCHES "flat")
endmacro(project_source_group mode sources headers)