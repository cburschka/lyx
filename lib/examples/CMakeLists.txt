# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko, <kornel@lyx.org>
#

project(test)

foreach(_arg longsheet.gnumeric iecc05.fen beamer-icsi-logo.pdf)
  configure_file(${_arg} "${CMAKE_CURRENT_BINARY_DIR}/${_arg}" COPYONLY)
endforeach(_arg)
