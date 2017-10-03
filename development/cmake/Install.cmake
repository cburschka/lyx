
# TODO: set correct path in call to cmake
# e.g. cmake /usr/src/lyx/lyx-devel/development/cmake -DCMAKE_INSTALL_PREFIX=/usr/local -Dnls=1

if(NOT(LYX_BUNDLE) AND APPLE)
        message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}, defined by CMAKE_INSTALL_PREFIX")
endif()

# the macro scans the directories "_parent_src_dir/_dir/_current_dir" for *._file_type files
# and installs the files in CMAKE_INSTALL_PREFIX/_current_dir
# dir_item is on item of the remaining arguments
#
# Special handling for ${_file_type} == "*"
#     Here we try to exclude files which we do not want to install
# Depending on _what, installation on UNIX goes to
# data -> ${LYX_DATA_SUBDIR}
# font -> share/${_lyx}/fonts/
# bin -> bin
# tex -> share/texmf/tex/latex/${_lyx}/
macro(lyx_install _what _parent_src_dir _gl_dir _file_type)
  #message("checking parents ${_parent_src_dir}")
  file(GLOB _dirs RELATIVE "${_parent_src_dir}" ${_parent_src_dir}/${_gl_dir})
  #cmake bug on Windows: if _gl_dir==. _dirs is empty but on linux _dirs==.
  if(NOT _dirs)
    set(_dirs .)
  endif()
  # Select installation dir
  if ("${_what}" STREQUAL "data")
    get_data_destination(_dest_subdir)
  elseif ("${_what}" STREQUAL "font")
    get_font_destination(_dest_subdir)
  elseif ("${_what}" STREQUAL "tex")
    get_tex_destination(_dest_subdir)
  else()
    message(FATAL_ERROR "Undefined parameter _what = ${_what} in call to lyx_install")
  endif()
  foreach(_dir ${_dirs})
    foreach(_glob_dir ${ARGN})
      file(GLOB _dir_list ${_parent_src_dir}/${_dir}/${_glob_dir})
      if(NOT _dir_list)
        if(_glob_dir STREQUAL ".")
          set(_dir_list ${_parent_src_dir}/${_dir}/.)
        endif()
      endif()
      #message(STATUS "${_dir}/${_glob_dir} -> ${_dir_list} ")
      foreach(_current_dir ${_dir_list})
        file(GLOB _item_list ${_current_dir}/${_file_type})
        #cmake bug: globbing with * also returns directories on Windows
        set(files_list ${_item_list})
	set(program_list)
        foreach(_current_item ${_item_list})
	  if(files_list)
	    if(IS_DIRECTORY ${_current_item})
	      #message(STATUS "removing dir ${_current_item}")
              list(REMOVE_ITEM files_list "${_current_item}")
            else()
	      if ("${_file_type}" STREQUAL "*")
		foreach(_mask "/\\.[^/]+$" "\\.in$" "\\.py$" "\\.patch$" "CMakeLists\\.txt$" "\\.lyx~$")
		  if(_current_item MATCHES ${_mask})
		    #message(STATUS "removing item ${_current_item}")
		    list(REMOVE_ITEM files_list "${_current_item}")
		  endif()
		endforeach()
	      endif()
	    endif()
          endif()
        endforeach(_current_item)
        if(files_list)
          list(REMOVE_ITEM files_list "${_current_dir}/lyx.svg")
          list(REMOVE_ITEM files_list "${_current_dir}/Makefile.am")
          list(REMOVE_ITEM files_list "${_current_dir}/layouttranslations")
          GET_FILENAME_COMPONENT(_base_dir ${_current_dir} NAME)
          if(_glob_dir STREQUAL ".")
            set(_base_dir .)
          endif()

          install(FILES ${files_list} DESTINATION "${_dest_subdir}${_dir}/${_base_dir}")
          #message(STATUS "Installing  ${files_list} to ${_dest_subdir}${_dir}/${_base_dir}")
        endif()
	if(program_list)
	  if(_glob_dir STREQUAL ".")
	    set(_base_dir .)
	  endif()
          install(FILES ${program_list} DESTINATION "${_dest_subdir}${_dir}/${_base_dir}")
	endif()
      endforeach(_current_dir)
    endforeach(_glob_dir)
  endforeach(_dir)
endmacro(lyx_install)


# language-specific-directories (like ca, de, es ...) are now globbed as "[a-z][a-z]"
set(_all_languages "[a-z][a-z]")

lyx_install("data" ${TOP_SRC_DIR}/lib bind         *.bind   . ${_all_languages})
lyx_install("data" ${TOP_SRC_DIR}/lib commands     *.def    .)

# this is handled in doc/CMakeLists.txt
#lyx_install("data" ${TOP_SRC_DIR}/lib doc          *.lyx    . ${_all_languages})
#lyx_install("data" ${TOP_SRC_DIR}/lib doc          *.txt    . ${_all_languages})
lyx_install("data" ${TOP_SRC_DIR}/lib doc          *      biblio clipart)

lyx_install("data" ${TOP_SRC_DIR}/lib doc/${_all_languages} *    clipart)

lyx_install("data" ${TOP_SRC_DIR}/lib examples     *      . ${_all_languages} springer thesis)
lyx_install("font" ${TOP_SRC_DIR}/lib/fonts .      *      .)
foreach(_imgext "png" "svgz" "gif" "xpm")
  lyx_install("data" ${TOP_SRC_DIR}/lib images       "*.${_imgext}"      . ipa commands attic classic oxygen)
  lyx_install("data" ${TOP_SRC_DIR}/lib images/math  "*.${_imgext}"      . oxygen)
endforeach()
lyx_install("data" ${TOP_SRC_DIR}/lib kbd          *      .)
lyx_install("data" ${TOP_SRC_DIR}/lib layouts      *      .)
lyx_install("data" ${TOP_SRC_DIR}/lib lyx2lyx      *.py   .)
lyx_install("data" ${TOP_SRC_DIR}/lib scripts      *.py   .)
lyx_install("data" ${TOP_SRC_DIR}/lib citeengines  *.citeengine .)
lyx_install("data" ${TOP_SRC_DIR}/lib .            *.py   .)
lyx_install("data" ${TOP_SRC_DIR}/lib scripts      *.R    .)
lyx_install("data" ${TOP_SRC_DIR}/lib templates    *      . springer)
lyx_install("data" ${TOP_SRC_DIR}/lib xtemplates  *.xtemplate .)
lyx_install("tex"  ${TOP_SRC_DIR}/lib/tex .        *      .)
lyx_install("data" ${TOP_SRC_DIR}/lib ui           *      .)
lyx_install("data" ${TOP_SRC_DIR}/lib .            *      .)

# Install
if(APPLE)
  if(LYX_BUNDLE)
    install(FILES ${TOP_SRC_DIR}/development/MacOSX/spotlight/Info.plist DESTINATION "${MACOSX_BUNDLE_STARTUP_COMMAND}/Contents/Library/Spotlight/LyX-Metadata.mdimporter/Contents")
    install(FILES "${TOP_SRC_DIR}/development/MacOSX/PkgInfo" DESTINATION "${MACOSX_BUNDLE_STARTUP_COMMAND}/Contents")
  endif()

  foreach(_i lyxeditor maxima inkscape)
    install(FILES "${TOP_SRC_DIR}/development/MacOSX/${_i}" DESTINATION "${LYX_UTILITIES_INSTALL_PATH}")
  endforeach()
  install(FILES "${TOP_SRC_DIR}/development/MacOSX/LyX.icns" DESTINATION "${LYX_DATA_SUBDIR}")
  set(program_suffix ${PROGRAM_SUFFIX})
elseif(UNIX)
  set(program_suffix ${PROGRAM_SUFFIX})
  configure_file(${TOP_SRC_DIR}/lib/lyx.desktop.in lyx${PROGRAM_SUFFIX}.desktop)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lyx${PROGRAM_SUFFIX}.desktop DESTINATION ${SYSTEM_DATADIR}/applications)
  install(FILES ${TOP_SRC_DIR}/lib/images/lyx.svg RENAME lyx${PROGRAM_SUFFIX}.svg DESTINATION ${SYSTEM_DATADIR}/icons/hicolor/scalable/apps/)
endif()

if(USE_POSIX_PACKAGING AND EXISTS "${TOP_SRC_DIR}/lib/usr.bin.lyxwrap.in")
  # handle lyxwrap
  #get_filename_component(prefix "${CMAKE_INSTALL_PREFIX}" REALPATH)
  set(prefix "${CMAKE_INSTALL_PREFIX}")
  set(version_suffix ${PROGRAM_SUFFIX})
  string(REGEX REPLACE "^/" "" lyxwrapprefix1 "${prefix}")
  string(REPLACE "/" "." lyxwrapprefix ${lyxwrapprefix1})
  #message(STATUS "lyxwrapprefix = ${lyxwrapprefix}")
  configure_file(${TOP_SRC_DIR}/lib/usr.bin.lyxwrap.in "${lyxwrapprefix}.${LYX_UTILITIES_INSTALL_PATH}.lyx${PROGRAM_SUFFIX}")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${lyxwrapprefix}.${LYX_UTILITIES_INSTALL_PATH}.lyx${PROGRAM_SUFFIX}" DESTINATION /etc/apparmor.d/)
endif()
