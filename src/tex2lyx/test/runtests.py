#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file src/tex2lyx/test/runtests.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Georg Baum

# Full author contact details are available in file CREDITS

# This script is a very basic test suite runner for tex2lyx
# The defaults for optional command line arguments are tailored to the
# standard use case of testing without special build settings like a version
# suffix, since I don't know how to transport command line arguments through
# the autotools "make check" mechanism.

import os, string, sys, time, difflib, filecmp


def usage(prog_name):
  return "Usage: %s [<tex2lyx binary> [<script dir>] [<output dir>]]" % prog_name


def main(argv):
    # Parse and manipulate the command line arguments.
    if len(argv) >= 3:
	sys.path.append(os.path.join(sys.argv[2]))
    else:
	sys.path.append(os.path.join(os.path.dirname(sys.argv[0]), '../../../lib/scripts'))

    from lyxpreview_tools import error

    if len(argv) < 2:
        tex2lyx = './tex2lyx'
    elif len(argv) <= 4:
        tex2lyx = argv[1]
    else:
        error(usage(argv[0]))

    inputdir = os.path.dirname(argv[0])
    if len(argv) >= 4:
        outputdir = sys.argv[3]
    else:
        outputdir = inputdir
#        outputdir = os.path.join(os.path.dirname(tex2lyx), "test")

    files = ['test.ltx', 'test-structure.tex', 'test-insets.tex', \
             'box-color-size-space-align.tex', 'CJK.tex', \
             'XeTeX-polyglossia.tex']

    errors = []
    overwrite = (outputdir == inputdir)
    for f in files:
        (base, ext) = os.path.splitext(f)
        texfile = os.path.join(inputdir, f)
        if overwrite:
            cmd = '%s -roundtrip -f %s' % (tex2lyx, texfile)
        else:
            lyxfile = os.path.join(outputdir, base + ".lyx")
            cmd = '%s -roundtrip -copyfiles -f %s %s' % (tex2lyx, texfile, lyxfile)
        if os.system(cmd) != 0:
            errors.append(f)
        elif not overwrite:
            lyxfile1 = os.path.join(inputdir, base + ".lyx.lyx")
            lyxfile2 = os.path.join(outputdir, base + ".lyx")
            if not filecmp.cmp(lyxfile1, lyxfile2, False):
                t1 = time.ctime(os.path.getmtime(lyxfile1))
                t2 = time.ctime(os.path.getmtime(lyxfile2))
                f1 = open(lyxfile1, 'r')
                f2 = open(lyxfile2, 'r')
                lines1 = f1.readlines()
                lines2 = f2.readlines()
                diff = difflib.unified_diff(lines1, lines2, lyxfile1, lyxfile2, t1, t2)
                f1.close()
                f2.close()
                sys.stdout.writelines(diff)
                errors.append(f)

    if len(errors) > 0:
        error('Converting the following files failed: %s' % ', '.join(errors))

if __name__ == "__main__":
    main(sys.argv)

