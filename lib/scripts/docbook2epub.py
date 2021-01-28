# -*- coding: utf-8 -*-

# file docbook2epub.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# \author Thibaut Cuvelier
#
# Full author contact details are available in file CREDITS

# Usage:
#   python docbook2epub.py in.docbook out.epub

from __future__ import print_function

import os
import shutil
import sys
import tempfile
import zipfile
import glob

if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.exit(1)
    own_path, java_path, input, output = sys.argv
    script_folder = os.path.dirname(own_path) + '/../'

    print('Generating ePub:')
    print(own_path)
    print(input)
    print(output)

    output_dir = tempfile.mkdtemp().replace('\\', '/')
    # os.chmod(output_dir, 0o777)
    print('Temporary output directory:')
    print(output_dir)

    # Start the XSLT transformation.
    xslt = script_folder + 'docbook/epub3/chunk.xsl'
    saxon_jar = script_folder + 'scripts/saxon6.5.5.jar'
    saxon_params = 'base.dir=%s' % output_dir
    command = '"' + java_path + '" -jar "' + saxon_jar + '" ' + input + ' ' + xslt + ' ' + saxon_params

    print('XSLT style sheet to use:')
    print(xslt)
    print('Command to execute:')
    print(command)

    if os.system('"' + command + '"') != 0:
        print('docbook2epub fails')
        shutil.rmtree(output_dir, ignore_errors=True)
        sys.exit(1)

    print('Generated ePub contents.')

    # TODO: Copy the assets to the OEBPS/images/.

    # Create the actual ePub file.
    with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as zip:
        for file in glob.glob(output_dir + '/**/*', recursive=True):
            zip.write(file, os.path.relpath(file, output_dir), compress_type=zipfile.ZIP_STORED)

    shutil.rmtree(output_dir)
    print('Generated ePub.')
