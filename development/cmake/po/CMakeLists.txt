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
    list(APPEND _sources ${_s1})
  endforeach(_arg)
  SET(_dst "${CMAKE_CURRENT_BINARY_DIR}/${_par}_l10n.pot")
  ADD_CUSTOM_COMMAND(
    OUTPUT "${_dst}"
    PRE_BUILD
    COMMAND python
    ARGS "${TOP_SRC_DIR}/po/lyx_pot.py" -b "${TOP_SRC_DIR}" -o "${_dst}" -t ${_par} ${_sources}
    DEPENDS ${_sources}
    )
  SET_SOURCE_FILES_PROPERTIES("${_dst}" GENERATED)
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

FIND_PROGRAM(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
FIND_PROGRAM(GETTEXT_MSGUNIQ_EXECUTABLE msguniq)

# TODO:
#         We need here perl, because I don't know how
#	  to write the alternative for "cat" in python
#	  e.g. perl -e "\"while(<>){print;}\""
# But maybe Jose knows?

ADD_CUSTOM_COMMAND(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.cat.pot"
    COMMAND ${_PERL_EXECUTABLE}
    ARGS -e "\"while(<>){print;}\"" ${_py_sources} > "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.cat.pot"
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
foreach(_dir src/frontends/qt4 src/insets src/mathed src/support src/tex2lyx src)
    file(GLOB _tmp_potfiles RELATIVE ${TOP_SRC_DIR} ${TOP_SRC_DIR}/${_dir}/*.cpp)
    foreach(_f ${_tmp_potfiles})
	LIST(APPEND _potfiles_dep "${TOP_SRC_DIR}/${_f}")
	file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/POTFILES.in "${_f}\n")
    endforeach(_f)
endforeach(_dir)

ADD_CUSTOM_COMMAND(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.pot"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.fmt.pot" "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.pot"
    COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE}
    ARGS --default-domain=${_lyxname} --directory=${TOP_SRC_DIR} --add-comments=TRANSLATORS: --language=C++ --join-existing --keyword=_ --keyword=N_ --keyword=B_ --keyword=qt_ --files-from="${CMAKE_CURRENT_BINARY_DIR}/POTFILES.in" --copyright-holder='LyX Developers' --msgid-bugs-address=lyx-devel@lists.lyx.org -o "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.pot"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.fmt.pot" ${_potfiles_dep}
    )

#
# We have to copy the po-files first,
# otherwise they would be modified (inside the macro GETTEXT_CREATE_TRANSLATIONS())
# This makes the "svn up" on po-files work again
file(GLOB LYX_BASE_PO_FILES RELATIVE ${TOP_SRC_DIR}/po ${TOP_SRC_DIR}/po/*.po)
set(LYX_PO_FILES)
foreach(_pofile ${LYX_BASE_PO_FILES})
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_pofile}
    COMMAND ${CMAKE_COMMAND} -E copy ${TOP_SRC_DIR}/po/${_pofile} ${CMAKE_CURRENT_BINARY_DIR}/${_pofile}
    DEPENDS ${TOP_SRC_DIR}/po/${_pofile}
    )
  list(APPEND LYX_PO_FILES ${CMAKE_CURRENT_BINARY_DIR}/${_pofile})
endforeach(_pofile)

GETTEXT_CREATE_TRANSLATIONS(${CMAKE_CURRENT_BINARY_DIR}/${_lyxname}.pot ALL ${LYX_PO_FILES})

# ADD_POFILES("lyx")

