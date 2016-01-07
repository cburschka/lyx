#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file updatelayouts.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Georg Baum

# Full author contact details are available in file CREDITS

# This script converts all layout files to current format
# The old files are backuped with extension ".old"


import os, re, string, sys, subprocess, tempfile, shutil

sys.path.append(os.path.join(os.path.dirname(sys.argv[0]), "../../lib/scripts"))
from layout2layout import main as layout2layout

def main(argv):

    toolsdir = os.path.dirname(argv[0])
    layoutdir = os.path.join(toolsdir, '../../lib/layouts')
    os.chdir(layoutdir)
    for i in os.listdir("."):
        (base, ext) = os.path.splitext(i)
        if ext == ".old":
            continue
        args = ["layout2layout", i + ".old", i] 
        shutil.copy(args[2], args[1])
        layout2layout(args)

    return 0


if __name__ == "__main__":
    main(sys.argv)
