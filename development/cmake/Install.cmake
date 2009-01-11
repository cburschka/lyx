
# TODO: set correct path in call to cmake
# e.g. cmake /usr/src/lyx/lyx-devel/development/cmake -DCMAKE_INSTALL_PREFIX=/usr/local/share/lyx2.0 -Dnls=1

message(STATUS "installing to ${CMAKE_INSTALL_PREFIX}, defined by CMAKE_INSTALL_PREFIX")

# the macro scans the directories "_parent_src_dir/_dir/_current_dir" for *._file_type files
# and installs the files in CMAKE_INSTALL_PREFIX/_current_dir
# dir_item is on item of the remaining arguments
macro(lyx_install _parent_src_dir _gl_dir _file_type)
  #message("checking parents ${_parent_src_dir}")
  file(GLOB _dirs RELATIVE "${_parent_src_dir}" ${_parent_src_dir}/${_gl_dir})
  #cmake bug on Windows: if _gl_dir==. _dirs is empty but on linux _dirs==.
  if(NOT _dirs)
    set(_dirs .)
  endif() 
  foreach(_dir ${_dirs})
    foreach(_glob_dir ${ARGN})
      file(GLOB _dir_list ${_parent_src_dir}/${_dir}/${_glob_dir})
      if(NOT _dir_list)
        set(_dir_list ${_parent_src_dir}/${_dir})
      endif() 
      if(_glob_dir STREQUAL ".")
        set(_dir_list ${_parent_src_dir}/${_dir}/.)
      endif()
      #message(STATUS "${_dir}/${_glob_dir} -> ${_dir_list} ")
      foreach(_current_dir ${_dir_list})
        file(GLOB _item_list ${_current_dir}/${_file_type})
        #cmake bug: globbing with * also returns directories on Windows
        set(files_list ${_item_list})
        foreach(_current_item ${_item_list})
          if(IS_DIRECTORY ${_current_item})
            #message(STATUS "removing ${_current_item}")
            if(files_list)
              list(REMOVE_ITEM files_list "${_current_item}")
            endif()
          endif()
        endforeach(_current_item)
        if(files_list)
          list(REMOVE_ITEM files_list "${_current_dir}/.svn")
          list(REMOVE_ITEM files_list "${_current_dir}/Makefile.in")
          list(REMOVE_ITEM files_list "${_current_dir}/Makefile.am")
          GET_FILENAME_COMPONENT(_base_dir ${_current_dir} NAME)
          if(_glob_dir STREQUAL ".")
            set(_base_dir .)
          endif()
          #message(STATUS "install ${_dir}/${_base_dir}: ${files_list} ")
          #message(STATUS "install at ${CMAKE_INSTALL_PREFIX}/${_dir}/${_base_dir}")
          install(FILES ${files_list} DESTINATION ${_dir}/${_base_dir})
        endif()
      endforeach(_current_dir)
    endforeach(_glob_dir)
  endforeach(_dir)
endmacro(lyx_install)


# language-specific-directories (like ca, de, es ...) are now globbed as "[a-z][a-z]"
set(_all_languages "[a-z][a-z]")

lyx_install(${TOP_SRC_DIR}/lib bind         *.bind   . ${_all_languages})
lyx_install(${TOP_SRC_DIR}/lib commands     *.def    .)

# this is handled in doc/CMakeLists.txt
#lyx_install(${TOP_SRC_DIR}/lib doc          *.lyx    . ${_all_languages})
#lyx_install(${TOP_SRC_DIR}/lib doc          *.txt    . ${_all_languages})
lyx_install(${TOP_SRC_DIR}/lib doc          *      biblio clipart)

lyx_install(${TOP_SRC_DIR}/lib doc/${_all_languages} *    clipart)

lyx_install(${TOP_SRC_DIR}/lib examples     *      . ${_all_languages})
lyx_install(${TOP_SRC_DIR}/lib fonts        *      .)
lyx_install(${TOP_SRC_DIR}/lib images       *      . math commands attic)
lyx_install(${TOP_SRC_DIR}/lib kbd          *      .)
lyx_install(${TOP_SRC_DIR}/lib layouts      *      .)
lyx_install(${TOP_SRC_DIR}/lib lyx2lyx      *.py   .)
lyx_install(${TOP_SRC_DIR}/lib scripts      *.py   .)
lyx_install(${TOP_SRC_DIR}/lib templates    *      .)
lyx_install(${TOP_SRC_DIR}/lib tex          *      .)
lyx_install(${TOP_SRC_DIR}/lib ui           *      .)
lyx_install(${TOP_SRC_DIR}/lib .            *      .)

install(PROGRAMS ${TOP_SRC_DIR}/lib/lyx2lyx/lyx2lyx DESTINATION lyx2lyx)

