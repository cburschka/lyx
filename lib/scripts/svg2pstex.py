#!/usr/bin/env python
# -*- coding: utf-8 -*-

# file svg2pstex.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Daniel Gloger
# author Martin Vermeer
# author Jürgen Spitzmüller

# Full author contact details are available in file CREDITS

# This script converts an SVG image to two files that can be processed
# with latex into high quality DVI/PostScript. It requires Inkscape.

# Usage:
#   python svg2pstex.py [--unstable] [inkscape_command] inputfile.svg outputfile.eps_tex
# This command generates
#   1. outputfile.eps     -- the converted EPS file (text from SVG stripped)
#   2. outputfile.eps_tex -- a TeX file that can be included in your
#                             LaTeX document using '\input{outputfile.eps_text}'
# use --unstable for inkscape < 1.0
#
# Note:
#   Do not use this command as
#     python svg2pstex.py [inkscape_command] inputfile.svg outputfile.pdf
#   the real EPS file would be overwritten by a TeX file named outputfile.eps.
#

# This script converts an SVG image to something that latex can process
# into high quality PostScript.

from __future__ import print_function

import os, sys, re, subprocess

def runCommand(cmd):
    ''' Utility function:
        run a command, quit if fails
    '''
    res = subprocess.check_call(cmd)
    if res != 0:
        print("Command '%s' fails (exit code: %i)." % (res.cmd, res.returncode))
        sys.exit(1)

InkscapeCmd = "inkscape"
InputFile = ""
OutputFile = ""
unstable = False

# We expect two to four args: the names of the input and output files
# and optionally the inkscape command (with path if needed) and --unstable.
args = len(sys.argv)
if args == 3:
    # Two args: input and output file only
    InputFile, OutputFile = sys.argv[1:]
elif args == 4:
    # Three args: check whether we have --unstable as first arg
    if sys.argv[1] == "--unstable":
        unstable = True
        InputFile, OutputFile = sys.argv[2:]
    else:
        InkscapeCmd, InputFile, OutputFile = sys.argv[1:]
elif args == 5:
    # Four args: check whether we have --unstable as first arg
    if sys.argv[1] != "--unstable":
        # Invalid number of args. Exit with error.
        sys.exit(1)
    else:
        unstable = True
        InkscapeCmd, InputFile, OutputFile = sys.argv[2:]
else:
    # Invalid number of args. Exit with error.
    sys.exit(1)

# Fail silently if the file doesn't exist
if not os.path.isfile(InputFile):
    sys.exit(0)

# Strip the extension from ${OutputFile}
OutBase = os.path.splitext(OutputFile)[0]

# Inkscape (as of 0.48) can output SVG images as an EPS file without text, ${OutBase}.eps,
# while outsourcing the text to a LaTeX file ${OutBase}.eps_tex which includes and overlays
# the EPS image and can be \input to LaTeX files. We rename the latter file to ${OutputFile}
# (although this is probably the name it already has).
runCommand([r'%s' % InkscapeCmd, '--file=%s' % InputFile, '--export-eps=%s.eps' % OutBase, '--export-latex'])

os.rename('%s.eps_tex' % OutBase, OutputFile)

