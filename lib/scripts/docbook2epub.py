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


class DocBookToEpub:
    def __init__(self, args=None):
        if args is None:
            args = sys.argv

        if len(args) != 7:
            print('Seven arguments are expected, only %s found: %s.' % (len(args), args))
            sys.exit(1)

        self.own_path = sys.argv[0]
        self.java_path = sys.argv[1] if sys.argv[1] != '' and sys.argv[1] != 'none' else None
        self.saxon_path = sys.argv[2] if sys.argv[2] != '' and sys.argv[2] != 'none' else None
        self.xsltproc_path = sys.argv[3] if sys.argv[3] != '' and sys.argv[3] != 'none' else None
        self.xslt_path = sys.argv[4] if sys.argv[4] != '' and sys.argv[4] != 'none' else None
        self.input = sys.argv[5]
        self.output = sys.argv[6]
        self.script_folder = os.path.dirname(self.own_path) + '/../'

        print('Generating ePub with the following parameters:')
        print(self.own_path)
        print(self.java_path)
        print(self.saxon_path)
        print(self.xsltproc_path)
        print(self.xslt_path)
        print(self.input)
        print(self.output)

        # Precompute paths that will be used later.
        self.output_dir = tempfile.mkdtemp().replace('\\', '/')
        self.package_opf = self.output_dir + '/OEBPS/package.opf'  # Does not exist yet,
        print('Temporary output directory: %s' % self.output_dir)

        if self.xslt_path is None:
            self.xslt = self.script_folder + 'docbook/epub3/chunk.xsl'
        else:
            self.xslt = self.xslt_path + '/epub3/chunk.xsl'
        print('XSLT style sheet to use:')
        print(self.xslt)

        if self.saxon_path is None:
            self.saxon_path = self.script_folder + 'scripts/saxon6.5.5.jar'

        # These will be filled during the execution of the script.
        self.renamed = None

    def gracefully_fail(self, reason):
        print('docbook2epub fails: %s' % reason)
        shutil.rmtree(self.output_dir, ignore_errors=True)
        sys.exit(1)

    def start_xslt_transformation(self):
        command = None
        if self.xsltproc_path is not None:
            command = self.start_xslt_transformation_xsltproc()
        elif self.java_path is not None:
            command = self.start_xslt_transformation_saxon6()

        if command is None:
            self.gracefully_fail('no XSLT processor available')

        print('Command to execute:')
        print(command)

        quoted_command = command
        if os.name == 'nt':
            # On Windows, it is typical to have spaces in folder names, and that requires to wrap the whole command
            # in quotes. On Linux, this might create errors when starting the command.
            quoted_command = '"' + command + '"'
        # This could be simplified by using subprocess.run, but this requires Python 3.5.

        if os.system(quoted_command) != 0:
            self.gracefully_fail('error from the XSLT processor')

        print('Generated ePub contents.')

    def start_xslt_transformation_xsltproc(self):
        params = '-stringparam base.dir "' + self.output_dir + '"'
        return '"' + self.xsltproc_path + '" ' + params + ' "' + self.xslt + '" "' + self.input + '"'

    def start_xslt_transformation_saxon6(self):
        params = 'base.dir=%s' % self.output_dir
        executable = '"' + self.java_path + '" -jar "' + self.saxon_path + '"'
        return executable + ' "' + self.input + '" "' + self.xslt + '" "' + params + '"'

    def get_images_from_package_opf(self):
        images = []

        # Example in the OPF file:
        #     <item id="d436e1" href="D:/LyX/lib/images/buffer-view.svgz" media-type="image/SVGZ"/>
        # The XHTML files are also <item> tags:
        #     <item id="id-d0e2" href="index.xhtml" media-type="application/xhtml+xml"/>
        try:
            with open(self.package_opf, 'r') as f:
                for line in f.readlines():
                    if '<item' in line and 'media-type="image' in line:
                        images.append(line.split('href="')[1].split('"')[0])
        except FileNotFoundError:
            print('The package.opf file was not found, probably due to a DocBook error. The ePub file will be corrupt.')

        return images

    def change_image_paths(self, file):
        # This could be optimised, as the same operation is performed a zillion times on many files:
        # https://www.oreilly.com/library/view/python-cookbook/0596001673/ch03s15.html
        with open(file, 'r', encoding='utf8') as f:
            contents = list(f)

        with open(file, 'w', encoding='utf8') as f:
            for line in contents:
                for (old, new) in self.renamed.items():
                    line = line.replace(old, new)
                f.write(line)

    def copy_images(self):
        # Copy the assets to the OEBPS/images/. All paths are available in OEBPS/package.opf, but they must also be
        # changed in the XHTML files. Typically, the current paths are absolute.

        # First, get the mapping old file => file in the ePub archive.
        original_images = self.get_images_from_package_opf()
        self.renamed = {img: 'images/' + os.path.basename(img) for img in original_images}

        # Then, transform all paths (both OPF and XHTML files).
        self.change_image_paths(self.output_dir + '/OEBPS/package.opf')
        for file in glob.glob(self.output_dir + '/OEBPS/*.xhtml'):
            self.change_image_paths(file)

        # Ensure that the destination path exists. OEBPS exists due to the DocBook-to-ePub transformation.
        if not os.path.exists(self.output_dir + '/OEBPS/images/'):
            os.mkdir(self.output_dir + '/OEBPS/images/')

        # Finally, actually copy the image files.
        for (old, new) in self.renamed.items():
            shutil.copyfile(old, self.output_dir + '/OEBPS/' + new)

    def create_zip_archive(self):
        with zipfile.ZipFile(self.output, 'w', zipfile.ZIP_DEFLATED) as zip:
            # Python 3.5 brings the `recursive` argument. For older versions, this trick is required...
            # for file in glob.glob(output_dir + '/**/*', recursive=True):
            for file in [os.path.join(dp, f) for dp, dn, filenames in os.walk(self.output_dir) for f in filenames]:
                zip.write(file, os.path.relpath(file, self.output_dir), compress_type=zipfile.ZIP_STORED)

        shutil.rmtree(self.output_dir)
        print('Generated ePub.')

    def transform(self):
        self.start_xslt_transformation()
        self.copy_images()
        self.create_zip_archive()


if __name__ == '__main__':
    DocBookToEpub(sys.argv).transform()
