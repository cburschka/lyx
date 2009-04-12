#
#  Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  1. Redistributions of source code must retain the copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote products
#     derived from this software without specific prior written permission.
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

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

macro(lyx_add_path _list _prefix)
   set(_tmp)
   foreach(_current ${${_list}})
      set(_tmp ${_tmp} ${_prefix}/${_current})
      #message(${_prefix}/${_current})
   endforeach(_current)
   set(${_list} ${_tmp})
endmacro(lyx_add_path _out _prefix)


#create the implementation files from the ui files and add them
# to the list of sources
#usage: LYX_ADD_QT4_UI_FILES(foo_SRCS ${ui_files})
macro(LYX_ADD_UI_FILES _sources _ui)
   foreach (_current_FILE ${ARGN})

      get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_tmp_FILE} NAME_WE)
      set(_header ${CMAKE_CURRENT_BINARY_DIR}/ui_${_basename}.h)

      # we need to run uic and replace some things in the generated file
      # this is done by executing the cmake script LyXuic.cmake
      # ######
      # Latest test showed on linux and windows show no bad consequeces,
      # so we removed the call to LyXuic.cmake
      add_custom_command(OUTPUT ${_header}
	 COMMAND ${QT_UIC_EXECUTABLE} -tr lyx::qt_ ${_tmp_FILE} -o ${_header}
         MAIN_DEPENDENCY ${_tmp_FILE}
)
      set(${_ui} ${${_ui}} ${_header})
   endforeach (_current_FILE)
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

         string(REGEX MATCHALL "#include +[^ ]+_moc\\.cpp[\">]" _match "${_contents}")
         if (_match)
            foreach (_current_MOC_INC ${_match})
               string(REGEX MATCH "[^ <\"]+_moc\\.cpp" _current_MOC "${_current_MOC_INC}")

               get_filename_component(_basename ${_current_MOC} NAME_WE)

               string(LENGTH ${_basename} _length)
               MATH(EXPR _mocless_length ${_length}-4)
               STRING(SUBSTRING  ${_basename} 0 ${_mocless_length} _mocless_name)

               set(_header ${_abs_PATH}/${_mocless_name}.h)

               #message(STATUS "moc : ${_header}")
               #set(_header ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.h)
               #set(_header ${_abs_PATH}/${_basename}.h)

               set(_moc  ${CMAKE_CURRENT_BINARY_DIR}/${_current_MOC})
               #set(_moc ${_abs_PATH}/${_current_MOC})
               add_custom_command(OUTPUT ${_moc}
                       COMMAND ${QT_MOC_EXECUTABLE}
                       ARGS ${_moc_INCS} ${_header} -o ${_moc}
                       MAIN_DEPENDENCY ${_header})
               macro_add_file_dependencies(${_abs_FILE} ${_moc})
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

   if (merge_rebuild)
      set(_rebuild_file_const 1)
      set(_rebuild_file_touched 1)
   endif()

   if (_rebuild_file_const)
      file(WRITE  ${_file_const} "// autogenerated file \n//\n")
      file(APPEND ${_file_const} "//    * clear or delete this file to build it again by cmake \n//\n\n")
   endif()

   if (_rebuild_file_touched)
      file(WRITE  ${_file_touched} "// autogenerated file \n//\n")
      file(APPEND ${_file_touched} "//    * clear or delete this file to build it again by cmake \n//\n")
      file(APPEND ${_file_touched} "//    * don't touch this file \n//\n\n")
      file(APPEND ${_file_touched} "#define DONT_INCLUDE_CONST_FILES\n")
      file(APPEND ${_file_touched} "#include \"${_file_const}\"\n\n\n")
   endif()

   set(${_file_list} ${_file_const} ${_file_touched})

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


macro(lyx_qt_resources_file _qrc_name _to_dir _list)
   if (NOT EXISTS ${_qrc_name})
      set(_rebuild_file 1)
   else()
      FILE(READ ${_qrc_name} _file_content)
      if (NOT _file_content)
         set(_rebuild_file 1)
      endif()
   endif()

   if (_rebuild_file)
      message(STATUS "Generating ${_qrc_name}")
      file(WRITE  ${_qrc_name} "<!DOCTYPE RCC><RCC version=\"1.0\">\n")
      file(APPEND  ${_qrc_name} "<qresource>\n")

      foreach (_current_FILE ${${_list}})
         get_filename_component(_abs_FILE ${_current_FILE} ABSOLUTE)
         string(REGEX REPLACE "${_to_dir}" "" _file_name ${_abs_FILE})
         file(APPEND  ${_qrc_name} "    <file alias=\"${_file_name}\">${_abs_FILE}</file>\n")
      endforeach (_current_FILE)

      file(APPEND  ${_qrc_name} "</qresource>\n")
      file(APPEND  ${_qrc_name} "</RCC>\n")
   endif()
endmacro(lyx_qt_resources_file)

