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

# converts an image $2 (format $1) to $4 (format $3)
from __future__ import print_function
import os, re, sys

# We may need some extra options only supported by recent convert versions
re_version = re.compile(r'^Version:.*ImageMagick\s*(\d*)\.(\d*)\.(\d*).*$')
# imagemagick 7
command = 'magick'
fout = os.popen('magick -version 2>&1')
output = fout.readline()
if fout.close() is not None:
    # older versions
    # caution: windows has a convert.exe for converting file systems
    command = 'convert'
    fout = os.popen('convert -version 2>&1')
    output = fout.readline()
    fout.close()

version = re_version.match(output)

# Imagemagick by default
im = False
gm = False

if version is not None:
    major = int(version.group(1))
    minor = int(version.group(2))
    patch = int(version.group(3))
    version = (major, minor, patch)
    im = True
else:
    # Try GraphicsMagick
    re_version = re.compile(r'^GraphicsMagick.*http:..www.GraphicsMagick.org.*$')
    version = re_version.match(output)
    if version is not None:
        gm = True
        # we need version to be a valid integer 3-tuple
        version = (1,0,0)

# IM >= 5.5.8 separates options for source and target files
# See http://www.imagemagick.org/Usage/basics/#why
if im or gm:
    sopts = ""
    topts = ""
elif sys.platform == 'darwin':
    command = 'lyxconvert'

# If supported, add the -define option for pdf source formats
if sys.argv[1] == 'pdf' and (version >= (6,2,6) or gm):
    sopts = '-define pdf:use-cropbox=true ' + sopts

# If supported, add the -flatten option for ppm target formats (see bug 4749)
if sys.argv[3] == 'ppm' and (im and version >= (6,3,5) or gm):
    topts = '-flatten'

# print (command, sys.argv[2], sys.argv[4], file= sys.stdout)
if (im or gm) and os.system(r'%s %s "%s" %s "%s"' % (command, sopts, sys.argv[2], topts, sys.argv[3] + ':' + sys.argv[4])) != 0:
    print(sys.argv[0], 'ERROR', file= sys.stderr)
    print('Execution of "%s" failed.' % command, file= sys.stderr)
    sys.exit(1)
elif not im and not gm and sys.platform == 'darwin' and os.system(r'%s "%s" "%s"' % (command, sys.argv[2], sys.argv[4])) != 0:
    print(sys.argv[0], 'ERROR', file= sys.stderr)
    print('Execution of "%s" failed.' % command, file=sys.stderr)
    sys.exit(1)
