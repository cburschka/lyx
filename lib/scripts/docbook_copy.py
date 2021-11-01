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


class DocBookCopier:
    def __init__(self, args):

        # Parse the command line.
        self.lilypond_command = args[1]
        self.in_file = args[2]
        self.out_file = args[3]

        # Compute a few things from the raw parameters.
        self.in_folder = os.path.split(self.in_file)[0]
        self.out_folder = os.path.split(self.out_file)[0]

        self.in_lily_file = self.in_file.replace('.xml', '.lyxml')
        self.has_lilypond = self.lilypond_command not in {'', 'none'}
        self.lilypond_folder = os.path.split(self.lilypond_command)[0] if self.has_lilypond else ''
        self.do_lilypond_processing = self.has_lilypond and self.in_file_needs_lilypond()

        # Help debugging.
        print('>> Given arguments:')
        print('>> LilyPond: ' + ('present' if self.has_lilypond else 'not found') + '.')
        print('>> LilyPond callable as: ' + self.lilypond_command + '.')
        print('>> LilyPond path: ' + self.lilypond_folder + '.')
        print('>> Input file: ' + self.in_file + '.')
        print('>> Output file: ' + self.out_file + '.')
        print('>> Input folder: ' + self.in_folder + '.')
        print('>> Output folder: ' + self.out_folder + '.')

    def in_file_needs_lilypond(self):
        # Really tailored to the kind of output lilypond.module makes (in lib/layouts).
        with open(self.in_file, 'r') as f:
            return "language='lilypond'" in f.read()

    def preprocess_input_for_lilypond(self):
        # LilyPond requires that its input file has the .lyxml extension. Due to a bug in LilyPond,
        # use " instead of ' to encode XML attributes.
        # Bug report: https://gitlab.com/lilypond/lilypond/-/issues/6185
        # Typical transformation:
        # Fixed by 2.23.4.
        #     FROM:  language='lilypond' role='fragment verbatim staffsize=16 ragged-right relative=2'
        #     TO:    language="lilypond" role="fragment verbatim staffsize=16 ragged-right relative=2"

        # Another problem to fix: the output is in XML, with some characters encoded as XML
        # entities. For instance, this could be in a LilyPond snippet:
        #     \new PianoStaff &lt;&lt;
        # instead of:
        #     \new PianoStaff <<
        # (More complete example:
        # https://lilypond.org/doc/v2.23/Documentation/learning/piano-centered-lyrics.)
        # This issue must be fixed by LilyPond, as any change in this part would make the XML
        # file invalid.
        # Bug report: https://gitlab.com/lilypond/lilypond/-/issues/6204
        with open(self.in_file, 'r', encoding='utf-8') as f, open(self.in_lily_file, 'w', encoding='utf-8') as f_lily:
            for line in f:
                if "language='lilypond'" in line:
                    line = re.sub(
                        '<programlisting\\s+language=\'lilypond\'.*?(role=\'(?P<options>.*?)\')?>',
                        '<programlisting language="lilypond" role="\\g<options>">',
                        line
                    )
                f_lily.write(line)
        os.unlink(self.in_file)

    def postprocess_output_for_lilypond(self):
        # Major problem: LilyPond used to output the LilyPond code outside the image, which is then always displayed
        # before the image (instead of only the generated image).
        # Bug report: https://gitlab.com/lilypond/lilypond/-/issues/6186
        # No more necessary with the new version of LilyPond (2.23.4). No efficient way to decide how to post-process
        # for previous versions of LilyPond. Basically, it does not make sense to post-process.
        pass

    def call_lilypond(self):
        # LilyPond requires that its input file has the .lyxml extension (plus bugs in LilyPond).
        print('>> Rewriting ' + self.in_file)
        print('>> as ' + self.in_lily_file + '.')
        self.preprocess_input_for_lilypond()

        # Add LilyPond to the PATH. lilypond-book uses a direct call to lilypond from the PATH.
        if os.path.isdir(self.lilypond_folder):
            os.environ['PATH'] += os.pathsep + self.lilypond_folder

        # Make LilyPond believe it is working from the temporary LyX directory. Otherwise, it tries to find files
        # starting from LyX's working directory... LilyPond bug, most likely.
        # https://lists.gnu.org/archive/html/bug-lilypond/2021-09/msg00041.html
        os.chdir(self.in_folder)

        # Start LilyPond on the copied file. First test the binary, then check if adding Python helps.
        command_args = ['--format=docbook', '--output=' + self.in_folder, self.in_lily_file]
        command_raw = [self.lilypond_command] + command_args
        command_python = ['python', self.lilypond_command] + command_args

        print('>> Running LilyPond.')
        sys.stdout.flush()  # So that the LilyPond output is at the right place in the logs.

        failed = True
        exceptions = []
        for cmd in [command_raw, command_python]:
            try:
                subprocess.check_call(cmd, stdout=sys.stdout.fileno(), stderr=sys.stdout.fileno())
                print('>> Success running LilyPond with ')
                print('>> ' + str(cmd))
                failed = False
            except (subprocess.CalledProcessError, OSError) as e:
                exceptions.append((cmd, e))

        if failed:
            print('>> Error from LilyPond. The successive calls were:')
            for (i, pair) in enumerate(exceptions):
                exc = pair[0]
                cmd = pair[1]

                print('>> (' + i + ') Error from trying ' + str(cmd) + ':')
                print('>> (' + i + ') ' + str(exc))

        if failed:
            sys.exit(1)

        # LilyPond has a distressing tendency to leave the raw LilyPond code in the new file.
        self.postprocess_output_for_lilypond()

        # Now, in_file should have the clean LilyPond-processed contents.

    def copy_lilypond_generated_images(self):
        # LilyPond generates a lot of files in LyX' temporary folder, within the ff folder: source LilyPond files
        # for each snippet to render, images in several formats.
        in_generated_images_folder = os.path.join(self.in_folder, 'ff')
        out_generated_images_folder = os.path.join(self.out_folder, 'ff')

        if not os.path.isdir(out_generated_images_folder):
            os.mkdir(out_generated_images_folder)

        for img in os.listdir(in_generated_images_folder):
            if not img.endswith('.png') and not img.endswith('.pdf'):
                continue

            shutil.copyfile(
                os.path.join(in_generated_images_folder, img),
                os.path.join(out_generated_images_folder, img),
            )

    def copy(self):
        # Apply LilyPond to the original file if available and needed.
        if self.do_lilypond_processing:
            print('>> The input file needs a LilyPond pass and LilyPond is available.')
            self.call_lilypond()

        # Perform the actual copy: both the modified XML file and the generated images, if LilyPond is used.
        shutil.copyfile(self.in_file, self.out_file)
        if self.do_lilypond_processing:
            self.copy_lilypond_generated_images()


if __name__ == '__main__':
    if len(sys.argv) != 4:
        print('Exactly four arguments are expected, only %s found: %s.' % (len(sys.argv), sys.argv))
        sys.exit(1)

    DocBookCopier(sys.argv).copy()
