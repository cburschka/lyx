# -*- coding: utf-8 -*-

# file docbook2epub.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# \author Thibaut Cuvelier
#
# Full author contact details are available in file CREDITS

# Usage:
#   python docbook2epub.py java_binary in.docbook out.epub

from __future__ import print_function

import glob
import os
import shutil
import sys
import tempfile
import zipfile


def parse_arguments():
    if len(sys.argv) != 4:
        sys.exit(1)
    own_path, java_path, input, output = sys.argv
    script_folder = os.path.dirname(own_path) + '/../'

    print('Generating ePub with the following parameters:')
    print(own_path)
    print(java_path)
    print(input)
    print(output)

    return java_path, input, output, script_folder


def create_temporary_folder():
    output_dir = tempfile.mkdtemp().replace('\\', '/')
    print('Temporary output directory:')
    print(output_dir)
    return output_dir


def start_xslt_transformation(input, output_dir, script_folder, java_path):
    xslt = script_folder + 'docbook/epub3/chunk.xsl'
    saxon_jar = script_folder + 'scripts/saxon6.5.5.jar'
    saxon_params = 'base.dir=%s' % output_dir
    command = '"' + java_path + '" -jar "' + saxon_jar + '" "' + input + '" "' + xslt + '" "' + saxon_params + '"'

    print('XSLT style sheet to use:')
    print(xslt)
    print('Command to execute:')
    print(command)

    quoted_command = command
    if os.name == 'nt':
        # On Windows, it is typical to have spaces in folder names, and that requires to wrap the whole command
        # in quotes. On Linux, this might create errors when starting the command.
        quoted_command = '"' + command + '"'
    # This could be simplified by using subprocess.run, but this requires Python 3.5.

    if os.system(quoted_command) != 0:
        print('docbook2epub fails')
        shutil.rmtree(output_dir, ignore_errors=True)
        sys.exit(1)

    print('Generated ePub contents.')


def get_images_from_package_opf(package_opf):
    images = []

    # Example in the OPF file:
    #     <item id="d436e1" href="D:/LyX/lib/images/buffer-view.svgz" media-type="image/SVGZ"/>
    # The XHTML files are also <item> tags:
    #     <item id="id-d0e2" href="index.xhtml" media-type="application/xhtml+xml"/>
    try:
        with open(package_opf, 'r') as f:
            for line in f.readlines():
                if '<item' in line and 'media-type="image' in line:
                    images.append(line.split('href="')[1].split('"')[0])
    except FileNotFoundError:
        print('The package.opf file was not found, probably due to a DocBook error. The ePub file will be corrupt.')

    return images


def change_image_paths(file, renamed):
    # This could be optimised, as the same operation is performed a zillion times on many files:
    # https://www.oreilly.com/library/view/python-cookbook/0596001673/ch03s15.html
    with open(file, 'r', encoding='utf8') as f:
        contents = list(f)

    with open(file, 'w', encoding='utf8') as f:
        for line in contents:
            for (old, new) in renamed.items():
                line = line.replace(old, new)
            f.write(line)


def copy_images(output_dir):
    # Copy the assets to the OEBPS/images/. All paths are available in OEBPS/package.opf, but they must also be changed
    # in the XHTML files. Typically, the current paths are absolute.

    # First, get the mapping old file => file in the ePub archive.
    original_images = get_images_from_package_opf(output_dir + '/OEBPS/package.opf')
    renamed = {img: 'images/' + os.path.basename(img) for img in original_images}

    # Then, transform all paths (both OPF and XHTML files).
    change_image_paths(output_dir + '/OEBPS/package.opf', renamed)
    for file in glob.glob(output_dir + '/OEBPS/*.xhtml'):
        change_image_paths(file, renamed)

    # Ensure that the destination path exists.
    if not os.path.exists(output_dir + '/OEBPS/images/'):
        os.mkdir(output_dir + '/OEBPS/images/')

    # Finally, actually copy the image files.
    for (old, new) in renamed.items():
        shutil.copyfile(old, output_dir + '/OEBPS/' + new)


def create_zip_archive(output, output_dir):
    with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as zip:
        # Python 3.5 brings the `recursive` argument. For older versions, this trick is required...
        # for file in glob.glob(output_dir + '/**/*', recursive=True):
        for file in [os.path.join(dp, f) for dp, dn, filenames in os.walk(output_dir) for f in filenames]:
            zip.write(file, os.path.relpath(file, output_dir), compress_type=zipfile.ZIP_STORED)

    shutil.rmtree(output_dir)
    print('Generated ePub.')


if __name__ == '__main__':
    java_path, input, output, script_folder = parse_arguments()
    output_dir = create_temporary_folder()
    start_xslt_transformation(input, output_dir, script_folder, java_path)
    copy_images(output_dir)
    create_zip_archive(output, output_dir)
