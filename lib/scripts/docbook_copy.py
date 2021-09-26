# -*- coding: utf-8 -*-

# file docbook_copy.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# \author Thibaut Cuvelier
#
# Full author contact details are available in file CREDITS

# Usage:
#   python docbook_copy.py lilypond_book_command in.docbook out.docbook
# This script copies the original DocBook file (directly produced by LyX) to the output DocBook file,
# potentially applying a post-processing step. For now, the only implemented post-processing step is
# LilyPond.
# lilypond_book_command is either directly the binary to call OR the equivalent Python script that is
# not directly executable.
# /!\ The original file may be modified by this script!


import os
import shutil
import sys


def need_lilypond(file):
    # Really tailored to the kind of output lilypond.module makes (in lib/layouts).
    with open(file, 'r') as f:
        return "language='lilypond'" in f.read()


def copy_docbook(args):
    print(args)
    if len(args) != 4:
        print('Exactly four arguments are expected, only %s found: %s.' % (len(args), args))
        sys.exit(1)

    # Parse the command line.
    lilypond_command = args[1]
    in_file = args[2]
    out_file = args[3]

    has_lilypond = lilypond_command != "" and lilypond_command != "none"

    # Apply LilyPond to the original file if available and needed.
    if has_lilypond and need_lilypond(in_file):
        # LilyPond requires that its input file has the .lyxml extension.
        # Move the file, so that LilyPond doesn't have to erase the contents of the original file before
        # writing the converted output.
        in_lily_file = in_file.replace(".xml", ".lyxml")
        shutil.move(in_file, in_lily_file)

        # Start LilyPond on the copied file. First test the binary, then check if adding Python helps.
        command = lilypond_command + ' --format=docbook ' + in_lily_file
        print(command)
        if os.system(command) != 0:
            command = 'python -tt "' + lilypond_command + '" --format=docbook ' + in_lily_file
            print(command)
            if os.system(command) != 0:
                print('Error from LilyPond')
                sys.exit(1)

        # Now, in_file should have the LilyPond-processed contents.

    # Perform the final copy.
    shutil.copyfile(in_file, out_file, follow_symlinks=False)


if __name__ == '__main__':
    copy_docbook(sys.argv)
