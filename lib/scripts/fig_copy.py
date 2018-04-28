# -*- coding: utf-8 -*-

# file fig_copy.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# \author Angus Leeming
# \author Bo Peng
#
# Full author contact details are available in file CREDITS

# Usage:
# fig_copy.py <from file> <to file>

# This script will copy an XFIG .fig file "$1" to "$2". In the process,
# it will modify the contents of the .fig file so that the names of any
# picture files that are stored as relative paths are replaced
# with the absolute path.

from __future__ import print_function
import os, sys

if len(sys.argv) != 3:
    print ("Usage: fig_copy.py <from file> <to file>", file=sys.stderr)
    sys.exit(1)

if not os.path.isfile(sys.argv[1]):
    print ("Unable to read", sys.argv[1], file=sys.stderr)
    sys.exit(1)

from_dir = os.path.split(os.path.realpath(sys.argv[1]))[0]
to_dir = os.path.split(os.path.realpath(sys.argv[2]))[0]

# The work is trivial if "to" and "from" are in the same directory.
if from_dir == to_dir:
    import shutil
    try:
        shutil.copy(sys.argv[1], sys.argv[2])
    except:
        sys.exit(1)
    sys.exit(0)

# Ok, they're in different directories. The .fig file must be modified.
import re

# We're looking for a line of text that defines an entry of
# type '2' (a polyline), subtype '5' (an external picture file).
# The line has 14 other data fields.
patternline = re.compile(br'^\s*2\s+5(\s+[0-9.+-]+){14}\s*$')
emptyline   = re.compile(br'^\s*$')
commentline = re.compile(br'^\s*#.*$')
# we allow space in path name
figureline  = re.compile(br'^(\s*[01]\s*)(\S[\S ]*)(\s*)$')

input = open(sys.argv[1], 'rb')
output = open(sys.argv[2], 'wb')

# path in the fig is relative to this path
os.chdir(from_dir)

found = False
for line in input:
    if found and not emptyline.match(line) and not commentline.match(line):
        # The contents of the final line containing the file name
        # are ' X <file name>', where X = 0 or 1.
        # We extract filename and replace it with the absolute filename.
        (pre, path, post) = figureline.match(line).groups()
        line = pre + os.path.realpath(path) + post
        found = False
    elif patternline.match(line):
        found = True
    output.write(line)

input.close()
output.close()
