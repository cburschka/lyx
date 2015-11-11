#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file updatedocs.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Georg Baum

# Full author contact details are available in file CREDITS

# This script converts documentation .lyx files to current format
# The old files are backuped with extension ".old"


import os, re, string, sys, subprocess, shutil


def convertdir(docdir, prefix, lyx2lyx):
    olddir = os.getcwd()
    os.chdir(docdir)
    for i in os.listdir("."):
        if os.path.isdir(i):
            subdir = os.path.join(docdir, i)
            subprefix = os.path.join(prefix, i)
            convertdir(subdir, subprefix, lyx2lyx)
            continue
        (base, ext) = os.path.splitext(i)
        if ext != ".lyx":
            continue
        old = i + ".old"
        shutil.copy(i, old)
        if sys.executable and sys.executable != '':
            cmd = [sys.executable, lyx2lyx, old, '-o', i]
        else:
            # assume that python is in the path
            cmd = [lyx2lyx, old, '-o', i]
        sys.stderr.write('Converting %s\n' % os.path.join(prefix, i))
        subprocess.call(cmd)
    os.chdir(olddir)


def main(argv):

    toolsdir = os.path.dirname(argv[0])
    docdir = os.path.abspath(os.path.join(toolsdir, '../../lib/doc'))
    lyx2lyx = os.path.abspath(os.path.join(toolsdir, "../../lib/lyx2lyx/lyx2lyx"))
    convertdir(docdir, '', lyx2lyx)

    return 0


if __name__ == "__main__":
    main(sys.argv)
