#!/usr/bin/env python
# -*- coding: utf-8 -*-

# file convertDefault.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# \author Herbert Voß
# \author Bo Peng

# Full author contact details are available in file CREDITS.

# The default converter if no other has been defined by the user from the
# Conversion->Converter tab of the Preferences dialog.

# The user can also redefine this default converter, placing their
# replacement in ~/.lyx/scripts

# converts an image from $1 to $2 format
import os, sys

opts = "-depth 8"
# for pdf source formats, check whether convert supports the -define option
if sys.argv[1][:4] == 'pdf:':
    defopt = "-define pdf:use-cropbox=true"
    fout = os.popen('convert ' + defopt + ' 2>&1')
    output = fout.read()
    fout.close()
    if not 'unrecognized' in output.lower():
        opts = defopt + ' ' + opts

# for ppm target formats, we need to flatten image, as ppm has no support
# for alpha channel, see bug 4749
if sys.argv[2][:4] == 'ppm:':
    opts = opts + ' -flatten'

if os.system(r'convert %s "%s" "%s"' % (opts, sys.argv[1], sys.argv[2])) != 0:
    print >> sys.stderr, sys.argv[0], 'ERROR'
    print >> sys.stderr, 'Execution of "convert" failed.'
    sys.exit(1)
