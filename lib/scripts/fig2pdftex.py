#!/usr/bin/env python
# -*- coding: utf-8 -*-

# file fig2pdf.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# \author Angus Leeming
# \author Bo Peng
#
# Full author contact details are available in file CREDITS


# This script converts an XFIG image to something that pdflatex can process
# into high quality PDF.

# Usage:
#   python fig2pdftex.py ${base}.fig ${base}.pdft
# This command generates
#   ${base}.pdf  the converted pdf file
#   ${base}.pdft a tex file that can be included in your latex document
#       using '\input{${base}.pdft}'
#
# Note:
#   Do not use this command as
#     python fig2pdftex.py file.fig file.pdf
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

# Ascertain whether fig2dev is "modern enough".
# If it is, then the help info will mention "pdftex_t" as one of the
# available outputs.
fout = os.popen('fig2dev -h')
help_msg = fout.read()
fout.close()


if 'pdftex_t' in help_msg:
    # Modern versions of xfig can output the image without "special" text as
    # a PDF file ${base}.pdf and place the text in a LaTeX file
    # ${base}.pdftex_t for typesetting by pdflatex itself.
    runCommand('fig2dev -Lpdftex -p1 %s %s.pdf' % (input, outbase))
    runCommand('fig2dev -Lpdftex_t -p%s %s %s' % (outbase, input, output))
else:
    # Older versions of xfig cannot do this, so we emulate the behaviour using
    # pstex and pstex_t output.
    runCommand('fig2dev -Lpstex %s %s.pstex' % (input, outbase))
    runCommand('fig2dev -Lpstex_t -p %s %s %s' % (outbase, input, output))

    # manipulates the Bounding Box info to enable gs to produce
    # the appropriate PDF file from an EPS one.
    # The generated PostScript commands are extracted from epstopdf, distributed
    # with tetex.
    epsfile = outbase + '.pstex'
    tmp = mkstemp()
    boundingboxline = re.compile('%%BoundingBox:\s+(\d*)\s+(\d*)\s+(\d*)\s+(\d*)')
    for line in open(epsfile).xreadlines():
        if line[:13] == '%%BoundingBox':
            (llx, lly, urx, ury) = map(int, boundingboxline.search(line).groups())
            width = urx - llx
            height = ury - lly
            xoffset = - llx
            yoffset = - lly
            tmp.write('''%%%%BoundingBox: 0 0 %d %d
<< /PageSize  [%d %d] >> setpagedevice
gsave %d %d translate
''' % (width, height, width, height, xoffset, yoffset))
        else:
            tmp.write(line)
    tmp.close()
    # direct move(rename) may fail under windows
    os.unlink(epsfile)
    os.rename(epsfile + '.??', epsfile)

    # Convert the ${pstex} EPS file (free of "special" text) to PDF format
    # using gs
    runCommand('gs -q -dNOPAUSE -dBATCH -dSAFER -sDEVICE=pdfwrite -sOutputFile=%s.pdf %s.pstex'\
      % (outbase, outbase))
    os.unlink(epsfile)
