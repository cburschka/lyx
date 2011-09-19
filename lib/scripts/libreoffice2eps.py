#!/usr/bin/env python
# -*- coding: utf-8 -*-

# file libreoffice2eps.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# \author Tommaso Cucinotta
#
# Full author contact details are available in file CREDITS


# This script converts an OpenOffice drawing to EPS.

# Usage:
#   python libreoffice2eps.py input.odg output.eps

import os, sys, tempfile, shutil

def runCommand(cmd):
    ''' Utility function:
        run a command, quit if fails
    '''
    if os.system(cmd) != 0:
        print "Command '%s' failed." % cmd
        sys.exit(1)

# We expect two args, the names of the input and output files.
if len(sys.argv) != 3:
    sys.exit(1)

input, output = sys.argv[1:]

# Fail silently if the file doesn't exist
if not os.path.isfile(input):
    sys.exit(0)

tmpdir = tempfile.mkdtemp()
fname = os.path.splitext(os.path.basename(input))[0]

# Generate the EPS file
runCommand('libreoffice -nologo -headless -convert-to eps -outdir "%s" "%s"' % (tmpdir, input))
shutil.move('%s/%s.eps' % (tmpdir, fname), '%s/%s.ps' % (tmpdir, fname))
runCommand('ps2eps "%s/%s.ps"' % (tmpdir, fname))
shutil.move('%s/%s.eps' % (tmpdir, fname), output)
os.remove('%s/%s.ps' % (tmpdir, fname))
os.rmdir(tmpdir)
