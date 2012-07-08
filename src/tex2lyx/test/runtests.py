#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file src/tex2lyx/test/runtests.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Georg Baum

# Full author contact details are available in file CREDITS

# This script reads a unicode symbol file and completes it in the given range

import os, string, sys


def usage(prog_name):
    return "Usage: %s [<tex2lyx binary>]" % prog_name


def main(argv):
    # Parse and manipulate the command line arguments.
    sys.path.append(os.path.join(os.path.dirname(sys.argv[0]), '../../../lib/scripts'))
    from lyxpreview_tools import error

    if len(argv) == 1:
        tex2lyx = './tex2lyx'
    elif len(argv) == 2:
        tex2lyx = argv[1]
    else:
        error(usage(argv[0]))

    basedir = os.path.dirname(argv[0])

    files = ['test.ltx', 'test-structure.tex', 'test-insets.tex', \
             'box-color-size-space-align.tex', 'CJK.tex', \
             'XeTeX-polyglossia.tex']

    errors = []
    for f in files:
        texfile = os.path.join(os.path.dirname(argv[0]), f)
        cmd = '%s -roundtrip -f %s' % (tex2lyx, texfile)
        if os.system(cmd) != 0:
            errors.append(f)

    if len(errors) > 0:
        error('Converting the following files failed: %s' % ', '.join(errors))

if __name__ == "__main__":
    main(sys.argv)

