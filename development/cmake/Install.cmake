# TODO: set correct path
#set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

message(STATUS "installing to ${CMAKE_INSTALL_PREFIX}, defined by CMAKE_INSTALL_PREFIX")

# the macro scans the directories "_parent_src_dir/_dir/_current_dir" for *._file_type files 
# and installs the files in CMAKE_INSTALL_PREFIX/_current_dir
# dir_item is on item of the remaining arguments
macro(lyx_install _parent_src_dir _dir _file_type)
   	foreach(_current_dir ${ARGN})
   		file(GLOB files_list ${_parent_src_dir}/${_dir}/${_current_dir}/*.${_file_type})
   		list(REMOVE_ITEM files_list "${_parent_src_dir}/${_dir}/${_current_dir}/.svn")
   		list(REMOVE_ITEM files_list "${_parent_src_dir}/${_dir}/${_current_dir}/Makefile.in")
   		list(REMOVE_ITEM files_list "${_parent_src_dir}/${_dir}/${_current_dir}/Makefile.am")
   		install(FILES ${files_list} DESTINATION ${_dir}/${_current_dir})
   		#message(STATUS "install ${_dir}/${_current_dir}: ${files_list} ")
   		#message(STATUS "install at ${CMAKE_INSTALL_PREFIX}/${_dir}/${_current_dir}")
   	endforeach(_current_dir)  	 
endmacro(lyx_install)


lyx_install(${TOP_SRC_DIR}/lib bind         bind   . de fi pt sv)
lyx_install(${TOP_SRC_DIR}/lib commands     def    .)
lyx_install(${TOP_SRC_DIR}/lib doc          lyx    . ca cs da de es eu fr gl he hu it ja nl nb pl pt ro ru sk sl sv uk)
lyx_install(${TOP_SRC_DIR}/lib doc          *      clipart)
lyx_install(${TOP_SRC_DIR}/lib doc/de       *      clipart)
lyx_install(${TOP_SRC_DIR}/lib doc/es       *      clipart)
lyx_install(${TOP_SRC_DIR}/lib doc/fr       *      clipart)
lyx_install(${TOP_SRC_DIR}/lib doc/it       *      clipart)
lyx_install(${TOP_SRC_DIR}/lib doc/ja       *      clipart)
lyx_install(${TOP_SRC_DIR}/lib doc/uk       *      clipart)
lyx_install(${TOP_SRC_DIR}/lib examples     *      .  ca cs da de es eu fr gl he hu it ja nl nb pl pt ro ru sk sl sv uk)
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

# TODO also get dot-less filenames in lyx_install
install(FILES ${TOP_SRC_DIR}/lib/lyx2lyx/lyx2lyx DESTINATION lyx2lyx)
foreach(_file unicodesymbols encodings languages)
	install(FILES ${TOP_SRC_DIR}/lib/${_file} DESTINATION .)
endforeach(_file)

# TODO
# DESTINATION/lyx2lyx
# is mode 644 but it should be 755

