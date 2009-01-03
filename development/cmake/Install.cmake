# TODO: set correct path

message(STATUS "installing to ${CMAKE_INSTALL_PREFIX}, defined by CMAKE_INSTALL_PREFIX")

# the macro scans the directories "_parent_src_dir/_dir/_current_dir" for *._file_type files
# and installs the files in CMAKE_INSTALL_PREFIX/_current_dir
# dir_item is on item of the remaining arguments
macro(lyx_install _parent_src_dir _dir _file_type)
  foreach(_glob_dir ${ARGN})
    file(GLOB _dir_list ${_parent_src_dir}/${_dir}/${_glob_dir})
    foreach(_current_dir ${_dir_list})
      file(GLOB files_list ${_current_dir}/${_file_type})
      list(REMOVE_ITEM files_list "${_current_dir}/.svn")
      list(REMOVE_ITEM files_list "${_current_dir}/Makefile.in")
      list(REMOVE_ITEM files_list "${_current_dir}/Makefile.am")
      GET_FILENAME_COMPONENT(_base_dir ${_current_dir} NAME)
      install(FILES ${files_list} DESTINATION ${_dir}/${_base_dir})
      #message(STATUS "install ${_dir}/${_base_dir}: ${files_list} ")
      #message(STATUS "install at ${CMAKE_INSTALL_PREFIX}/${_dir}/${_base_dir}")
    endforeach(_current_dir)
  endforeach(_glob_dir)
endmacro(lyx_install)


lyx_install(${TOP_SRC_DIR}/lib bind         *.bind   . [a-z][a-z])
lyx_install(${TOP_SRC_DIR}/lib commands     *.def    .)
# this is handled in doc/CMakefile.txt
#lyx_install(${TOP_SRC_DIR}/lib doc          *.lyx    . [a-z][a-z])
#lyx_install(${TOP_SRC_DIR}/lib doc          *.txt    . [a-z][a-z])
lyx_install(${TOP_SRC_DIR}/lib doc          *      biblio clipart)
lyx_install(${TOP_SRC_DIR}/lib doc/de       *      clipart)
lyx_install(${TOP_SRC_DIR}/lib doc/es       *      clipart)
lyx_install(${TOP_SRC_DIR}/lib doc/fr       *      clipart)
lyx_install(${TOP_SRC_DIR}/lib doc/it       *      clipart)
lyx_install(${TOP_SRC_DIR}/lib doc/ja       *      clipart)
lyx_install(${TOP_SRC_DIR}/lib doc/uk       *      clipart)
lyx_install(${TOP_SRC_DIR}/lib examples     *      . [a-z][a-z])
lyx_install(${TOP_SRC_DIR}/lib fonts        *      .)
lyx_install(${TOP_SRC_DIR}/lib images       *      . math commands attic)
lyx_install(${TOP_SRC_DIR}/lib kbd          *      .)
lyx_install(${TOP_SRC_DIR}/lib layouts      *      .)
lyx_install(${TOP_SRC_DIR}/lib lyx2lyx      *      .)
lyx_install(${TOP_SRC_DIR}/lib scripts      *      .)
lyx_install(${TOP_SRC_DIR}/lib templates    *      .)
lyx_install(${TOP_SRC_DIR}/lib tex          *      .)
lyx_install(${TOP_SRC_DIR}/lib ui           *      .)
lyx_install(${TOP_SRC_DIR}/lib .            *      .)

install(PROGRAMS ${TOP_SRC_DIR}/lib/lyx2lyx/lyx2lyx DESTINATION lyx2lyx)
