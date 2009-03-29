# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2008, 2009 Peter Kümmel, <syntheticpp@gmx.net>
# Copyright (c) 2008, 2009 Kornel Benko, <Kornel.Benko@berlin.de>
#

project(man)

SET(_manuals)
set(program_suffix ${PROGRAM_SUFFIX})
set(version_suffix ${PROGRAM_SUFFIX})
set(VERSION "${LYX_MAJOR_VERSION}.${LYX_MINOR_VERSION}")
macro(add_manual _mansrc)
  get_filename_component(_mandest ${_mansrc} NAME_WE)
  SET(_created_manual "${CMAKE_CURRENT_BINARY_DIR}/${_mandest}${PROGRAM_SUFFIX}.1")
  configure_file("${TOP_SRC_DIR}/${_mansrc}" ${_created_manual} @ONLY)
  SET_SOURCE_FILES_PROPERTIES(${_created_manual} GENERATED)
  LIST(APPEND _manuals ${_created_manual})
endmacro(add_manual)

set(real_pkgdatadir ${LYX_ABS_INSTALLED_DATADIR})
foreach(_mn lyx.1in src/client/lyxclient.1in src/tex2lyx/tex2lyx.1in)
  add_manual(${_mn})
endforeach(_mn)

ADD_CUSTOM_TARGET(man ALL DEPENDS ${_manuals})

install(FILES ${_manuals} DESTINATION man/man1)

# ADD_MANUALS("lyx")

