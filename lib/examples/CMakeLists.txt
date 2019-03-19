# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2012 Kornel Benko, <kornel@lyx.org>
#

project(testExamples)

foreach(_arg "External_Material/longsheet.gnumeric"
             "Graphics_\&_Insets/iecc05.fen"
             "Presentations/beamer-icsi-logo.pdf")
  configure_file("${_arg}" "${CMAKE_CURRENT_BINARY_DIR}/${_arg}" COPYONLY)
endforeach(_arg)
