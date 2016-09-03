#!/usr/bin/env python
# -*- coding: utf-8 -*-

# file svg2pstex.py
#
# This script converts an SVG image to something that latex can process
# into high quality PostScript.

# Usage:
#   python svg2pstex.py ${base}.fig ${base}.pstex
# This command generates
#   ${base}.eps    the converted eps file
#   ${base}.pstex  a tex file that can be included in your latex document
#       using '\input{${output}}'.
#
# Note:
#   Do not use this command as
#     python svg2pstex.py file.fig file.eps
#   the real eps file will be overwritten by a tex file named file.eps.
#

import os, sys

def runCommand(cmd):
    ''' Utility function:
        run a command, quit if fails
    '''
    if os.system(cmd) != 0:
        print "Command '%s' fails." % cmd
        sys.exit(1)

# We expect two args, the names of the input and output files.
if len(sys.argv) != 3:
    sys.exit(1)

input, output = sys.argv[1:]

# Fail silently if the file doesn't exist
if not os.path.isfile(input):
    sys.exit(0)

# Strip the extension from ${output}
outbase = os.path.splitext(output)[0]

# Inkscape 0.48 can output the image as a EPS file ${base}.pdf and place the text 
# in a LaTeX file ${base}.eps_tex, which is renamed to ${output}, for typesetting 
# by latex itself. 
runCommand('inkscape --file=%s --export-eps=%s.eps --export-latex' % (input, outbase))
os.rename('%s.eps_tex' % outbase, output)
