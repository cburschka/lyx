#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file ext_copy.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Richard Heck

# Full author contact details are available in file CREDITS

# Usage:
# ext_copy.py [-e ext1,ext2,...] <from file> <to file>

# This script is to be used as a "copier" script in the sense needed by
# the converter definitions. Given a <from file> and <to file>, it will copy
# all files in the directory in which from_file is found that have the 
# extensions given in the -e argument, or all files in that directory if no 
# such argument is given. So, for example, we can do:
#   python ext_copy.py -e png,html,css /path/from/file.html /path/to/file.html
# and all html, png, and css files in /path/from/ will be copied to the 
# (possibly new) directory /path/to/file.html.LyXconv/.
# The -t argument determines the extension added, the default being "LyXconv".
# If just . is given, no extension is added.

# KNOWN BUG: This script is not aware of generated subdirectories.

import os, sys, getopt
from lyxpreview_tools import error


def usage(prog_name):
    return "Usage: %s [-e extensions] [-t target extension] <from file> <to file>" % prog_name


def main(argv):
    progname = argv[0]

    exts = [] #list of extensions for which we're checking
    targext = "LyXconv" #extension for target directory
    opts, args = getopt.getopt(sys.argv[1:], "e:t:")
    for o, v in opts:
      if o == "-e":
        exts = v.split(',')
      if o == "-t":
        targext = v

    # input directory
    if len(args) != 2:
      error(usage(progname))
    abs_from_file = args[0]
    if not os.path.isabs(abs_from_file):
      error("%s is not an absolute file name.\n%s" % abs_from_file, usage(progname))
    from_dir = os.path.dirname(abs_from_file)

    # output directory
    to_dir = args[1]
    if targext != '.':
      to_dir += "." + targext
    if not os.path.isabs(to_dir):
      error("%s is not an absolute file name.\n%s" % to_dir, usage(progname))

    # try to create the output directory if it doesn't exist
    if not os.path.isdir(to_dir):
      try:
        os.makedirs(to_dir)
      except:
        error("Unable to create %s" % to_dir)

    import shutil

    # copy all matching files in from_dir to to_dir
    for file in os.listdir(from_dir):
      if os.path.isdir(file):
        continue
      junk, ext = os.path.splitext(os.path.basename(file))
      ext = ext.lower()[1:] #strip the leading dot
      try:
        # if exts is empty we ignore it
        # otherwise check if the extension is in the list
        not exts or exts.index(ext)
      except:
        continue #not found
      from_file = os.path.join(from_dir, file)
      to_file  = os.path.join(to_dir, file)
      shutil.copyfile(from_file, to_file)
      try:
        shutil.copymode(from_file, to_file)
      except:
        pass
    return 0

if __name__ == "__main__":
    main(sys.argv)
