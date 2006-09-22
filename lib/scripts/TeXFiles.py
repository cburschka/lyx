#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-

# file TeXFiles.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# \author Herbert Voß
# \author Jean-Marc Lasgouttes
# \author Jürgen Spitzmüller
# \author Bo Peng

# Full author contact details are available in file CREDITS.

# all files		-> without option
# TeX class files	-> option cls
# TeX style files 	-> option sty
# bibtex style files 	-> option bst
# bibtex database files -> option bib
#
# with the help
# of kpsewhich and creates a
# bstFiles.lst, clsFiles.lst, styFiles.lst, bibFiles.lst
# without any parameter all files are created.
#
# Herbert Voss <voss@perce.org>
#
# Updates from Jean-Marc Lasgouttes.
#
# bib support added by Juergen Spitzmueller (v0.3)
#
# translated to python by Bo Peng, so that the script only 
# relies on python and kpsewhich (no shell command is used).
# 

# this import is required for 2.2 compatibility
from __future__ import generators

import os, sys, re

cls_stylefile = 'clsFiles.lst'
sty_stylefile = 'styFiles.lst'
bst_stylefile = 'bstFiles.lst'
bib_files = 'bibFiles.lst'

# this code was taken from twisted
# http://twistedmatrix.com/trac/browser/trunk/twisted/python/compat.py?rev=14178&format=txt
# and allow us to use os.walk with python 2.2
try:
    os.walk
except AttributeError:
    if sys.version_info[:3] == (2, 2, 0):
        __builtin__.True = (1 == 1)
        __builtin__.False = (1 == 0)
        def bool(value):
            """Demote a value to 0 or 1, depending on its truth value

            This is not to be confused with types.BooleanType, which is
            way too hard to duplicate in 2.1 to be worth the trouble.
            """
            return not not value
        __builtin__.bool = bool
        del bool

    def walk(top, topdown=True, onerror=None):
        from os.path import join, isdir, islink

        try:
            names = os.listdir(top)
        except OSError, e:
            if onerror is not None:
                onerror(err)
            return

        nondir, dir = [], []
        nameLists = [nondir, dir]
        for name in names:
            nameLists[isdir(join(top, name))].append(name)

        if topdown:
            yield top, dir, nondir

        for name in dir:
            path = join(top, name)
            if not islink(path):
                for x in walk(path, topdown, onerror):
                    yield x

        if not topdown:
            yield top, dir, nondir
    os.walk = walk
# end compatibility chunk

def cmdOutput(cmd):
    '''utility function: run a command and get its output as a string
        cmd: command to run
    '''
    fout = os.popen(cmd)
    output = fout.read()
    fout.close()
    return output

# processing command line options
if len(sys.argv) > 1:
    if sys.argv[1] in ['--help', '-help']:
        print '''Usage: TeXFiles.py [-version | cls | sty | bst | bib ]
            Default is without any Parameters,
            so that all files will be created'''
        sye.exit(0)
    else:
        types = sys.argv[1:]
        for type in types:
            if type not in ['cls', 'sty', 'bst', 'bib']:
                print 'ERROR: unknown type', type
                sys.exit(1)
else:
    # if no parameter is specified, assume all
    types = ['cls', 'sty', 'bst', 'bib']

#
# MS-DOS and MS-Windows define $COMSPEC or $ComSpec and use `;' to separate
# directories in path lists whereas Unix uses `:'.  Make an exception for
# Cygwin, where we could have either teTeX (using `:') or MikTeX (using `;').
# Create a variable that holds the right character to be used by the scripts.
path_sep = os.pathsep
if sys.platform == 'cygwin':
    # MikTeX's kpsewhich says "kpathsea emulation version x.x.x", whereas
    # teTeX's simply "kpathsea version x.x.x".
    if 'emulation' in cmdOutput('kpsewhich --version'):
        path_sep = ';'
    else:
        path_sep = ':'

# process each file type
for type in types:
    print "Indexing files of type", type
    if type == 'cls':
        outfile = cls_stylefile
        kpsetype = '.tex'
    elif type == 'sty':
        outfile = sty_stylefile
        kpsetype = '.tex'
    elif type == 'bst':
        outfile = bst_stylefile
        kpsetype = '.bst'
    elif type == 'bib':
        outfile = bib_files
        kpsetype = '.bib'

    dirs = cmdOutput('kpsewhich --show-path=' + kpsetype).replace('!!', '').strip()
    # remove excessive //
    dirs = re.sub('//+', '/', dirs)
    
    file_ext = '.' + type
    out = open(outfile, 'w')
    for dir in dirs.split(path_sep):
        # for each valid directory
        if not os.path.isdir(dir):
            continue
        # walk down the file hierarchy
        for root,path,files in os.walk(dir):
            # check file type
            for file in files:
                if len(file) > 4 and file[-4:] == file_ext:
                    # force the use of / since miktex uses / even under windows
                    print >> out, root.replace('\\', '/') + '/' + file
    out.close()

