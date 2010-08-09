# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2008, 2009 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c) 2008, 2009 Kornel Benko, <Kornel.Benko@berlin.de>
#

project(po)

include_directories(${TOP_SRC_DIR}/po)

SET(_lyxname ${PACKAGE})
SET(_py_sources)
macro(add_gettext_python  _par _dir)
  set(_sources)
  foreach(_arg ${ARGN})
    file(GLOB _s1 ${TOP_SRC_DIR}/${_dir}/${_arg})
    list(SORT _s1)
    list(APPEND _sources ${_s1})
  endforeach(_arg)
  SET(_dst "${CMAKE_CURRENT_BINARY_DIR}/${_par}_l10n.pot")
  SET(_tmp_src_files "${CMAKE_CURRENT_BINARY_DIR}/${_par}_tmp_files")
  SET(_src_files "${CMAKE_CURRENT_BINARY_DIR}/${_par}_files")
  file(WRITE ${_tmp_src_files} "")
  foreach(_s ${_sources})
    FILE(APPEND ${_tmp_src_files} "${_s}\n")
  endforeach()
  if(${_par} MATCHES "^\(.*\)_[0-9]+$")
    set(_par1 ${CMAKE_MATCH_1})
  else()
    set(_par1 ${_par})
  endif()
  ADD_CUSTOM_COMMAND(
    OUTPUT ${_src_files}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_tmp_src_files} ${_src_files}
    DEPENDS ${_tmp_src_files}
  )
  ADD_CUSTOM_COMMAND(
    OUTPUT "${_dst}"
    PRE_BUILD
    COMMAND ${LYX_PYTHON_EXECUTABLE}
    ARGS "${TOP_SRC_DIR}/po/lyx_pot.py" -b "${TOP_SRC_DIR}" -o "${_dst}" -t ${_par1} "--src_file=${_src_files}"
    DEPENDS ${_sources} ${_src_files}
    )
  SET_SOURCE_FILES_PROPERTIES("${_dst}" GENERATED)
  SET_SOURCE_FILES_PROPERTIES("${_src_files}" GENERATED)
  LIST(APPEND _py_sources "${_dst}")
endmacro(add_gettext_python)

SET_SOURCE_FILES_PROPERTIES("${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.cat.pot" GENERATED)
SET_SOURCE_FILES_PROPERTIES("${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.fmt.pot" GENERATED)
SET_SOURCE_FILES_PROPERTIES("${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.pot" GENERATED)

add_gettext_python(qt4 src/frontends/qt4/ui *.ui)
add_gettext_python(layouts lib/layouts *.layout *.inc *.module)
add_gettext_python(languages lib languages)
add_gettext_python(encodings lib encodings)
add_gettext_python(ui lib/ui *.ui *.inc)
add_gettext_python(external lib external_templates)
add_gettext_python(formats lib configure.py)

ADD_CUSTOM_COMMAND(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.cat.pot"
    COMMAND ${LYX_PYTHON_EXECUTABLE}
    ARGS   "${CMAKE_SOURCE_DIR}/po/cat.py" ${_py_sources} > "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.cat.pot"
    DEPENDS ${_py_sources}
    )

ADD_CUSTOM_COMMAND(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.fmt.pot"
    COMMAND ${GETTEXT_MSGUNIQ_EXECUTABLE}
    ARGS -o "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.fmt.pot" "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.cat.pot"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.cat.pot"
    )

SET(_potfiles_dep)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/POTFILES.in "")
file(GLOB_RECURSE _tmp_potfiles RELATIVE ${TOP_SRC_DIR}/ ${TOP_SRC_DIR}/src/*.h  ${TOP_SRC_DIR}/src/*.cpp  ${TOP_SRC_DIR}/src/*.cpp.in)
list(SORT _tmp_potfiles)
list(REMOVE_ITEM _tmp_potfiles src/support/Package.cpp)

foreach(_f ${_tmp_potfiles})
  # we don't need to select only relevant files because the outcome of xgettext
  # will stay the same.
  # Relevant files are files containing the string '_(".*")'
  LIST(APPEND _potfiles_dep "${TOP_SRC_DIR}/${_f}")
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/POTFILES.in "${_f}\n")
endforeach(_f)

ADD_CUSTOM_COMMAND(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.pot"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.fmt.pot" "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.pot"
    COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE}
    ARGS --default-domain=${_lyxname} --directory=${TOP_SRC_DIR} --add-comments=TRANSLATORS: --language=C++ --join-existing --keyword=_ --keyword=N_ --keyword=B_ --keyword=qt_ --files-from="${CMAKE_CURRENT_BINARY_DIR}/POTFILES.in" --copyright-holder="LyX Developers" --msgid-bugs-address=lyx-devel@lists.lyx.org -o "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.pot"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.fmt.pot" ${_potfiles_dep}
    )

#
# We have to copy the po-files first,
# otherwise they would be modified (inside the macro GETTEXT_CREATE_TRANSLATIONS())
# This makes the "svn up" on po-files work again
file(GLOB LYX_BASE_PO_FILES RELATIVE ${TOP_SRC_DIR}/po ${TOP_SRC_DIR}/po/*.po)
set(LYX_PO_FILES)
SET(LYX_UPDATED_PO_FILES)
SET(LYX_DEST_PO ${TOP_SRC_DIR}/po)

foreach(_pofile ${LYX_BASE_PO_FILES})
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_pofile}
    COMMAND ${CMAKE_COMMAND} -E copy ${TOP_SRC_DIR}/po/${_pofile} ${CMAKE_CURRENT_BINARY_DIR}/${_pofile}
    DEPENDS ${TOP_SRC_DIR}/po/${_pofile}
    )
  list(APPEND LYX_PO_FILES ${CMAKE_CURRENT_BINARY_DIR}/${_pofile})
endforeach(_pofile)

foreach(_pofile ${LYX_BASE_PO_FILES})
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_pofile}.copied
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/${_pofile} ${LYX_DEST_PO}/${_pofile}
    COMMAND ${CMAKE_COMMAND} -E touch ${_pofile}.copied
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${_pofile}
    )
  list(APPEND LYX_UPDATED_PO_FILES ${CMAKE_CURRENT_BINARY_DIR}/${_pofile}.copied)
endforeach(_pofile)

if(LYX_CPACK OR LYX_INSTALL)
  set(_addALLOption ALL)
endif()
GETTEXT_CREATE_TRANSLATIONS(${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.pot ${_addALLOption} ${LYX_PO_FILES})

# ALL omitted here, to prevent from automatic creation
ADD_CUSTOM_TARGET(update_po_files DEPENDS ${LYX_UPDATED_PO_FILES})
add_dependencies(update_po_files translations)

# ADD_POFILES("lyx")

