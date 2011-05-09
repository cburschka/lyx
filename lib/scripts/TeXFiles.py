#!/usr/bin/env python
# -*- coding: utf-8 -*-

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

import os, sys, re

cls_stylefile = 'clsFiles.lst'
sty_stylefile = 'styFiles.lst'
bst_stylefile = 'bstFiles.lst'
bib_files = 'bibFiles.lst'

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
        sys.exit(0)
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
    if ';' in cmdOutput('kpsewhich --show-path=.tex'):
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

