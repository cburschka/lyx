#!/usr/bin/env python
# -*- coding: utf-8 -*-

# file svg2pdftex.py
#
# This script converts an SVG image to something that pdflatex can process
# into high quality PDF.

# Usage:
#   python svg2pdftex.py ${base}.svg ${base}.pdft
# This command generates
#   ${base}.pdf  the converted pdf file
#   ${base}.pdft a tex file that can be included in your latex document
#       using '\input{${base}.pdft}'
#
# Note:
#   Do not use this command as
#     python svg2pdftex.py file.svg file.pdf
#   the real pdf file will be overwritten by a tex file named file.pdf.
#



import os, sys, re


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



# Inkscape 0.48 can output the image as a PDF file ${base}.pdf and place the text 
# in a LaTeX file ${base}.pdf_tex, which is renamed to ${output}, for typesetting 
# by pdflatex itself. 
runCommand('inkscape --file=%s --export-pdf=%s.pdf --export-latex' % (input, outbase))

os.rename('%s.pdf_tex' % outbase, output)

