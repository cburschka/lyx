#!/usr/bin/env python
# -*- coding: utf-8 -*-

# file fig2pstex.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# \author Angus Leeming
# \author Bo Peng
#
# Full author contact details are available in file CREDITS


# This script converts an XFIG image to something that latex can process
# into high quality PostScript.

# Usage:
#   python fig2pstex.py ${base}.fig ${base}.pstex
# This command generates
#   ${base}.eps    the converted eps file
#   ${base}.pstex  a tex file that can be included in your latex document
#       using '\input{${output}}'.
#
# Note:
#   Do not use this command as
#     python fig2pstex.py file.fig file.eps
#   the real eps file will be overwritten by a tex file named file.eps.
#

import os, sys

# We expect two args, the names of the input and output files.
if len(sys.argv) != 3:
    sys.exit(1)

input, output = sys.argv[1:]

# Fail silently if the file doesn't exist
if not os.path.isfile(input):
    sys.exit(0)

# Strip the extension from ${output}
outbase = os.path.splitext(output)[0]

# Generate the EPS file
# Generate the PSTEX_T file
if os.system('fig2dev -Lpstex %s %s.eps' % (input, outbase)) != 0 or \
  os.system('fig2dev -Lpstex_t -p%s %s %s' % (outbase, input, output)) != 0:
  print 'fig2dev fails'
  sys.exit(1)
