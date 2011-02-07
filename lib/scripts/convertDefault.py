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
import os, re, sys

# We may need some extra options only supported by recent convert versions
re_version = re.compile(r'^Version:.*ImageMagick\s*(\d*)\.(\d*)\.(\d*).*$')
fout = os.popen('convert -version 2>&1')
output = fout.readline()
fout.close()
version = re_version.match(output)

# Imagemagick by default
gm = False

if version != None:
    major = int(version.group(1))
    minor = int(version.group(2))
    patch = int(version.group(3))
    version = hex(major * 65536 + minor * 256 + patch)
else:
    # Try GraphicsMagick
    re_version = re.compile(r'^GraphicsMagick.*http:..www.GraphicsMagick.org.*$')
    version = re_version.match(output)
    if version != None:
        gm = True

opts = "-depth 8"

# If supported, add the -define option for pdf source formats 
if sys.argv[1][:4] == 'pdf:' and (version >= 0x060206 or gm):
    opts = '-define pdf:use-cropbox=true ' + opts

# If supported, add the -flatten option for ppm target formats (see bug 4749)
if sys.argv[2][:4] == 'ppm:' and (version >= 0x060305 or gm):
    opts = opts + ' -flatten'

if os.system(r'convert %s "%s" "%s"' % (opts, sys.argv[1], sys.argv[2])) != 0:
    print >> sys.stderr, sys.argv[0], 'ERROR'
    print >> sys.stderr, 'Execution of "convert" failed.'
    sys.exit(1)
