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
from gen_lfuns import main as genlfuns


def convertdir(docdir, prefix, lyx2lyx, lyx, systemlyxdir):
    olddir = os.getcwd()
    os.chdir(docdir)
    for i in os.listdir("."):
        if os.path.isdir(i):
            if i != 'attic':
                subdir = os.path.join(docdir, i)
                subprefix = os.path.join(prefix, i)
                convertdir(subdir, subprefix, lyx2lyx, lyx, systemlyxdir)
            continue
        (base, ext) = os.path.splitext(i)
        if ext != ".lyx":
            continue
        old = i + ".old"
        shutil.copy(i, old)
        if sys.executable and sys.executable != '':
            cmd = [sys.executable, lyx2lyx, old, '-s', systemlyxdir, '-o', i]
        else:
            # assume that python is in the path
            cmd = [lyx2lyx, old, '-s', systemlyxdir, '-o', i]
        sys.stderr.write('Converting %s\n' % os.path.join(prefix, i))
        subprocess.call(cmd)
        if lyx != '':
            # This is a hack, but without modifying the doc LyX refuses to save and stays open
            # FIXME: Is self-insert a; char-delete-backward always a noop?
            #        What if change-tracking is enabled?
            cmd = [lyx, '-f', '-x', 'command-sequence self-insert a; char-delete-backward; buffer-write; lyx-quit', i]
            subprocess.call(cmd)
    os.chdir(olddir)


def main(argv):

    toolsdir = os.path.dirname(argv[0])

    # first generate LFUNs.lyx
    lyxaction = os.path.abspath(os.path.join(toolsdir, "../../src/LyXAction.cpp"))
    lfunspath = os.path.abspath(os.path.join(toolsdir, "../../lib/doc"))
    lfuns = os.path.join(lfunspath, "LFUNs.lyx")
    # genlfuns refuses to overwrite files
    if os.path.exists(lfuns):
        os.rename(lfuns, os.path.join(lfunspath, "LFUNs.lyx.old"))
    # genlfuns requires a trailing slash
    genlfuns(["genlfuns", lyxaction, lfunspath + os.sep])

    # then update all docs
    lyx2lyx = os.path.abspath(os.path.join(toolsdir, "../../lib/lyx2lyx/lyx2lyx"))
    systemlyxdir = os.path.abspath(os.path.join(toolsdir, "../../lib"))
    if len(argv) > 1:
        sys.stderr.write('Warning: Converting with LyX is experimental. Check the results carefully.\n')
        lyx = os.path.abspath(argv[1])
    else:
        lyx = ''
    docpaths = ['../../lib/doc', '../../lib/examples', '../../lib/templates', '../../development/MacOSX/ReadMe']
    for docpath in docpaths:
        docdir = os.path.abspath(os.path.join(toolsdir, docpath))
        convertdir(docdir, '', lyx2lyx, lyx, systemlyxdir)

    return 0


if __name__ == "__main__":
    main(sys.argv)
