# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2008, 2009 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c) 2008, 2009 Kornel Benko, <Kornel.Benko@berlin.de>
#

project(po)

include_directories(${TOP_SRC_DIR}/po)

SET(_py_sources)
macro(add_gettext_python  _par _dir)
  file(GLOB _sources ${TOP_SRC_DIR}/${_dir}/${ARGN})
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

SET_SOURCE_FILES_PROPERTIES("${CMAKE_CURRENT_BINARY_DIR}/lyx.cat.pot" GENERATED)
SET_SOURCE_FILES_PROPERTIES("${CMAKE_CURRENT_BINARY_DIR}/lyx.fmt.pot" GENERATED)
SET_SOURCE_FILES_PROPERTIES("${CMAKE_CURRENT_BINARY_DIR}/lyx.pot" GENERATED)

add_gettext_python(encodings lib encodings)
add_gettext_python(external lib external_templates)
add_gettext_python(formats lib configure.py)
add_gettext_python(languages lib languages)
add_gettext_python(layouts lib/layouts *.layouts *.inc *.module)
add_gettext_python(qt4 src/frontends/qt4/ui *.ui)
add_gettext_python(ui lib/ui *.ui *.inc)

FIND_PROGRAM(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
FIND_PROGRAM(GETTEXT_MSGUNIQ_EXECUTABLE msguniq)

# TODO:
#         We need here perl, because I don't know how
#	  to write the alternative for "cat" in python
#	  e.g. perl -e "\"while(<>){print;}\""
# But maybe Jose knows?

ADD_CUSTOM_COMMAND(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lyx.cat.pot"
    COMMAND perl
    ARGS -e "\"while(<>){print;}\"" ${_py_sources} > "${CMAKE_CURRENT_BINARY_DIR}/lyx.cat.pot"
    DEPENDS ${_py_sources}
    )

ADD_CUSTOM_COMMAND(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lyx.fmt.pot"
    COMMAND ${GETTEXT_MSGUNIQ_EXECUTABLE}
    ARGS -o "${CMAKE_CURRENT_BINARY_DIR}/lyx.fmt.pot" "${CMAKE_CURRENT_BINARY_DIR}/lyx.cat.pot"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/lyx.cat.pot"
    )

file(STRINGS "${TOP_SRC_DIR}/po/POTFILES.in" _tmppotfiles_dep)
SET(_potfiles_dep)
foreach(_f ${_tmppotfiles_dep})
  LIST(APPEND _potfiles_dep "${TOP_SRC_DIR}/${_f}")
endforeach(_f)

ADD_CUSTOM_COMMAND(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lyx.pot"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/lyx.fmt.pot" "${CMAKE_CURRENT_BINARY_DIR}/lyx.pot"
    COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE}
    ARGS --default-domain=lyx --directory=${TOP_SRC_DIR} --add-comments=TRANSLATORS: --language=C++ --join-existing --keyword=_ --keyword=N_ --keyword=B_ --keyword=qt_ --files-from="${TOP_SRC_DIR}/po/POTFILES.in" --copyright-holder='LyX Developers' --msgid-bugs-address=lyx-devel@lists.lyx.org -o "${CMAKE_CURRENT_BINARY_DIR}/lyx.pot"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/lyx.fmt.pot" ${_potfiles_dep}
    )

file(GLOB LYX_PO_FILES ${TOP_SRC_DIR}/po/*.po)

GETTEXT_CREATE_TRANSLATIONS(${CMAKE_CURRENT_BINARY_DIR}/lyx.pot ALL ${LYX_PO_FILES})

# ADD_POFILES("lyx")

