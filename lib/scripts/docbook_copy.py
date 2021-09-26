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


import subprocess
import os
import os.path
import re
import shutil
import sys


def need_lilypond(file):
    # Really tailored to the kind of output lilypond.module makes (in lib/layouts).
    with open(file, 'r') as f:
        return "language='lilypond'" in f.read()


def copy_docbook(args):
    if len(args) != 4:
        print('Exactly four arguments are expected, only %s found: %s.' % (len(args), args))
        sys.exit(1)

    # Parse the command line.
    lilypond_command = args[1]
    in_file = args[2]
    out_file = args[3]

    has_lilypond = lilypond_command not in {'', 'none'}
    in_folder = os.path.split(in_file)[0]

    # Guess the path for LilyPond.
    lilypond_folder = os.path.split(lilypond_command)[0] if has_lilypond else ''

    # Help debugging.
    print('>> Given arguments:')
    print('>> LilyPond: ' + ('present' if has_lilypond else 'not found') + '.')
    print('>> LilyPond callable as: ' + lilypond_command + '.')
    print('>> LilyPond path: ' + lilypond_folder + '.')
    print('>> Input file: ' + in_file + '.')
    print('>> Input folder: ' + in_folder + '.')
    print('>> Output file: ' + out_file + '.')

    # Apply LilyPond to the original file if available and needed.
    if has_lilypond and need_lilypond(in_file):
        in_lily_file = in_file.replace(".xml", ".lyxml")
        print('>> The input file needs a LilyPond pass and LilyPond is available.')
        print('>> Rewriting ' + in_file)
        print('>> as ' + in_lily_file + '.')

        # LilyPond requires that its input file has the .lyxml extension. Due to a bug in LilyPond,
        # use " instead of ' to encode XML attributes.
        # https://lists.gnu.org/archive/html/bug-lilypond/2021-09/msg00039.html
        # Typical transformation:
        #     FROM:  language='lilypond' role='fragment verbatim staffsize=16 ragged-right relative=2'
        #     TO:    language="lilypond" role="fragment verbatim staffsize=16 ragged-right relative=2"
        with open(in_file, 'r', encoding='utf-8') as f, open(in_lily_file, 'w', encoding='utf-8') as f_lily:
            for line in f:
                if "language='lilypond'" in line:
                    line = re.sub(
                        '<programlisting\\s+language=\'lilypond\'.*?(role=\'(?P<options>.*?)\')?>',
                        '<programlisting language="lilypond" role="\\g<options>">',
                        line
                    )
                f_lily.write(line)
        os.unlink(in_file)

        # Add LilyPond to the PATH. lilypond-book uses a direct call to lilypond from the PATH.
        if os.path.isdir(lilypond_folder):
            os.environ['PATH'] += os.pathsep + lilypond_folder

        # Make LilyPond believe it is working from the temporary LyX directory. Otherwise, it tries to find files
        # starting from LyX's working directory...
        os.chdir(in_folder)

        # Start LilyPond on the copied file. First test the binary, then check if adding Python helps.
        command_args = ['--format=docbook', '--output=' + in_folder, in_lily_file]
        command_raw = [lilypond_command] + command_args
        command_python = ['python', lilypond_command] + command_args

        print('>> Running LilyPond.')
        sys.stdout.flush()  # So that the LilyPond output is at the right place in the logs.

        failed = False
        try:
            subprocess.check_call(command_raw, stdout=sys.stdout.fileno(), stderr=sys.stdout.fileno())
            print('>> Success running LilyPond with ')
            print('>> ' + str(command_raw))
        except (subprocess.CalledProcessError, OSError) as e1:
            try:
                subprocess.check_call(command_python, stdout=sys.stdout.fileno(), stderr=sys.stdout.fileno())
                print('>> Success running LilyPond with ')
                print('>> ' + str(command_python) + '.')
            except (subprocess.CalledProcessError, OSError) as e2:
                print('>> Error from LilyPond. The successive calls were:')
                print('>> (1) Error from trying ' + str(command_raw) + ':')
                print('>> (1) ' + str(e1))
                print('>> (2) Error from trying ' + str(command_python) + ':')
                print('>> (2) ' + str(e2))
                failed = True

        if failed:
            sys.exit(1)

        # Now, in_file should have the LilyPond-processed contents.

    # Perform the final copy.
    shutil.copyfile(in_file, out_file, follow_symlinks=False)


if __name__ == '__main__':
    copy_docbook(sys.argv)
