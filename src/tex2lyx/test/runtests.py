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

import os, string, sys, time, difflib, filecmp, subprocess, re

def usage(prog_name):
  return "Usage: %s [uselyx2lyx] [<tex2lyx binary> [[<script dir>] [[<output dir>] [testfile]]]]" % prog_name

pat_fl1 = re.compile(r'^#LyX file created by tex2lyx .*$')
pat_fl2 = re.compile(r'^#LyX \d+\.\d+ created this file.*$')

def compareLyx(lines1, lines2):
    if lines1[1:] != lines2[1:]:
        return False
    if not pat_fl1.match(lines1[0]) and not pat_fl2.match(lines1[0]):
        return False
    if not pat_fl1.match(lines2[0]) and not pat_fl2.match(lines2[0]):
        return False
    return True

def main(argv):
    # Parse and manipulate the command line arguments.
    skipcount = 0
    uselyx2lyx = False
    if len(argv) > 1:
        if argv[1] == "uselyx2lyx":
            uselyx2lyx = True
            skipcount = 1
    if len(argv) >= 3+skipcount:
        sys.path.append(os.path.join(sys.argv[2+skipcount]))
    else:
        sys.path.append(os.path.join(os.path.dirname(sys.argv[0]), '../../../lib/scripts'))

    from lyxpreview_tools import error

    if len(argv) < 2+skipcount:
        tex2lyx = './tex2lyx'
    elif len(argv) <= 5+skipcount:
        tex2lyx = argv[1+skipcount]
    else:
        error(usage(argv[0]))

    suffixre = re.search(r'\d+\.\d+$', tex2lyx)
    if suffixre:
        suffix = suffixre.group()
    else:
        suffix = ""
    lyx = os.path.join(os.path.dirname(tex2lyx), "lyx" + suffix)
    inputdir = os.path.dirname(argv[0])
    if len(argv) >= 4+skipcount:
        outputdir = sys.argv[3+skipcount]
    else:
#        outputdir = inputdir
        outputdir = os.path.join(os.path.dirname(tex2lyx), "test")

    if len(argv) >= 5+skipcount:
        files = [sys.argv[4+skipcount]]
    else:
        files = ['test.ltx', \
                 'box-color-size-space-align.tex', \
                 'CJK.tex', \
                 'CJKutf8.tex', \
                 'test-insets.tex', \
                 'test-memoir.tex', \
                 'test-modules.tex', \
                 'test-refstyle-theorems.tex', \
                 'test-scr.tex', \
                 'test-structure.tex', \
                 'verbatim.tex', \
                 'XeTeX-polyglossia.tex', \
                 'algo2e.tex']

    errors = []
    overwrite = (outputdir == inputdir)
    for f in files:
        (base, ext) = os.path.splitext(f)
        texfile = os.path.join(inputdir, f)
        if overwrite:
            # we are updating the test references, so use roundtrip to allow
            # for checking the LyX export as well.
            cmd = '%s -roundtrip -f %s' % (tex2lyx, texfile)
        else:
            lyxfile = os.path.join(outputdir, base + ".lyx")
            cmd = '%s -roundtrip -copyfiles -f %s %s' % (tex2lyx, texfile, lyxfile)
        print 'Executing: ' + cmd + "\n"
        proc = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        proc.wait()
        err = proc.returncode
        errorstring = proc.stderr.read()
        if not errorstring is None:
            print errorstring
        if err != 0:
            errors.append(f)
        elif not overwrite:
            lyxfile1 = getlyxinput(lyx,
                        os.path.join(inputdir, base + ".lyx.lyx"),
                        os.path.join(outputdir, base + ".lyx1.lyx") , uselyx2lyx)
            if lyxfile1 is None:
                errors.append(f)
            else:
                lyxfile2 = getlyxinput(lyx,
                          os.path.join(outputdir, base + ".lyx"),
                          os.path.join(outputdir, base + ".lyx2.lyx"), uselyx2lyx)
                if lyxfile2 is None:
                    errors.append(f)
                else:
                    t1 = time.ctime(os.path.getmtime(lyxfile1))
                    t2 = time.ctime(os.path.getmtime(lyxfile2))
                    f1 = open(lyxfile1, 'r')
                    f2 = open(lyxfile2, 'r')
                    lines1 = f1.readlines()
                    lines2 = f2.readlines()
                    f1.close()
                    f2.close()
                    # ignore the first line e.g. the version of lyx
                    if not compareLyx(lines1, lines2):
                        diff = difflib.unified_diff(lines1, lines2, lyxfile1, lyxfile2, t1, t2)
                        sys.stdout.writelines(diff)
                        errors.append(f)


    if len(errors) > 0:
        error('Converting the following files failed: %s' % ', '.join(errors))

def getlyxinput(lyx, lyxfx, lyxf, uselyx2lyx):
    if uselyx2lyx:
        cmd = '%s -E lyx %s %s' % (lyx, lyxf, lyxfx)
        sys.stdout.writelines(cmd)
        if os.system(cmd) != 0:
            return None
        return lyxf
    else:
        return lyxfx

if __name__ == "__main__":
    main(sys.argv)

