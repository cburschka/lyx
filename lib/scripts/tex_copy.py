#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file tex_copy.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Angus Leeming
# author Georg Baum

# Full author contact details are available in file CREDITS

# Usage:
# tex_copy.py <from file> <to file> <latex name>

# This script will copy a file <from file> to <to file>.
# <to file> is no exact copy of <from file>, but any occurence of <basename>
# where <basename> is <from file> without directory and extension parts is
# replaced by <latex name> without extension.


import os, string, sys

from lyxpreview_tools import error


def usage(prog_name):
    return "Usage: %s <from file> <to file> <latex name>" % prog_name


def main(argv):
    # Parse and manipulate the command line arguments.
    if len(argv) != 4:
        error(usage(argv[0]))

    # input file
    abs_from_file = argv[1]
    if not os.path.isabs(abs_from_file):
        error("%s is no absolute file name.\n%s"\
              % abs_from_file, usage(argv[0]))
    from_dir, rel_from_file = os.path.split(abs_from_file)
    from_base, from_ext = os.path.splitext(rel_from_file)

    # output file
    abs_to_file = argv[2]
    if not os.path.isabs(abs_to_file):
        error("%s is no absolute file name.\n%s"\
              % abs_to_file, usage(argv[0]))
    to_dir, rel_to_file = os.path.split(abs_to_file)
    to_base, to_ext = os.path.splitext(rel_to_file)

    # latex file name
    latex_file = argv[3]
    latex_base, latex_ext = os.path.splitext(latex_file)

    # Read the input file and write the output file
    if(not os.path.isfile(abs_from_file)):
         error("%s is not a valid file.\n" % abs_from_file)
    from_file = open(abs_from_file, 'rb')
    to_file = open(abs_to_file, 'wb')
    lines = from_file.readlines()
    for line in lines:
        to_file.write(line.replace(from_base, latex_base))
    from_file.close()
    to_file.close()

    return 0


if __name__ == "__main__":
    main(sys.argv)
