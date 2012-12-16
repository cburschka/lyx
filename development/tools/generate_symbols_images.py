#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file generate_symbols_images.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Georg Baum

# Full author contact details are available in file CREDITS

# This script generates a toolbar image for each missing math symbol
# It needs the template document generate_symbols_images.lyx, which must
# contain the placeholder formula '$a$' for generating the png image via
# preview.sty and dvipng.
# The created images are not always optimal, therefore the existing manually
# created images should never be replaced by automatically created ones.


import os, re, string, sys, subprocess, tempfile, shutil
import Image

def usage(prog_name):
    return ("Usage: %s lyxexe outputpath\n" % prog_name)


def error(message):
    sys.stderr.write(message + '\n')
    sys.exit(1)


def getlist(lyxexe, lyxfile):
    """ Call LyX and get a list of symbols from mathed debug output.
        This way, we can re-use the symbols file parser of LyX, and do not
        need to reimplement it in python. """

    # The debug is only generated if lyxfile contains a formula
    cmd = "%s %s -dbg mathed -x lyx-quit" % (lyxexe, lyxfile)
    proc = subprocess.Popen(cmd, shell=True, stderr=subprocess.PIPE)
    (stdout, stderr) = proc.communicate()
    regexp = re.compile(r'.*: read symbol \'(\S+)\s+inset:\s+(\S+)')
    # These insets are more complex than simply symbols, so the images need to
    # be created manually
    skipinsets = ['big', 'font', 'matrix', 'mbox', 'oldfont', 'ref', 'space']
    symbols = []
    for line in stderr.split('\n'):
        m = regexp.match(line)
        if m:
            inset = m.group(2)
            if not inset in skipinsets:
                symbols.append(m.group(1))
    return symbols


def createimage(name, path, template, lyxexe, tempdir):
    """ Create the image file for symbol name in path. """

    if name == '|':
        filename = 'vert'
    elif name == '/':
        filename = 'slash'
    elif name == '\\':
        filename = 'backslash'
    elif name == '*':
        filename = 'ast'
    elif name.startswith('lyx'):
        print 'Skipping ' + name
        return
    else:
        skipchars = ['|', '/', '\\', '*', '!', '?', ':', ';', '^', '<', '>']
        for i in skipchars:
            if name.find(i) >= 0:
                print 'Skipping ' + name
                return
        filename = name
    pngname = os.path.join(path, filename + '.png')
    if os.path.exists(pngname):
        print 'Skipping ' + name
        return
    print 'Generating ' + name
    lyxname = os.path.join(tempdir, filename)
    lyxfile = open(lyxname + '.lyx', 'wt')
    lyxfile.write(template.replace('$a$', '$\\' + name + '$'))
    lyxfile.close()
    cmd = "%s %s.lyx -e dvi" % (lyxexe, lyxname)
    proc = subprocess.Popen(cmd, shell=True)
    proc.wait()
    if proc.returncode != 0:
        print 'Error in DVI creation for ' + name
        return
    # The magnifaction factor is calculated such that we get an image of
    # height 18 px for most symbols and document font size 11. Then we can
    # add a small border to get the standard math image height of 20 px.
    cmd = "dvipng %s.dvi -bg Transparent -D 115 -o %s" % (lyxname, pngname)
    proc = subprocess.Popen(cmd, shell=True)
    proc.wait()
    if proc.returncode != 0:
        print 'Error in PNG creation for ' + name
        return
    image = Image.open(pngname)
    (width, height) = image.size
    if width < 20 and height < 20:
        if width == 19 and height == 19:
            padded = Image.new('RGBA', (width+1, height+1), (0, 0, 0, 0))
            padded.paste(image, (0, 0))
        elif width == 19:
            padded = Image.new('RGBA', (width+1, height+2), (0, 0, 0, 0))
            padded.paste(image, (0, 1))
        elif height == 19:
            padded = Image.new('RGBA', (width+2, height+1), (0, 0, 0, 0))
            padded.paste(image, (1, 0))
        else:
            padded = Image.new('RGBA', (width+2, height+2), (0, 0, 0, 0))
            padded.paste(image, (1, 1))
        padded.convert(image.mode)
        padded.save(pngname, "PNG")


def main(argv):

    if len(argv) == 3:
        (base, ext) = os.path.splitext(argv[0])
        symbols = getlist(argv[1], base)
        lyxtemplate = base + '.lyx'
        templatefile = open(base + '.lyx', 'rt')
        template = templatefile.read()
        templatefile.close()
        tempdir = tempfile.mkdtemp()
        for i in symbols:
            createimage(i, argv[2], template, argv[1], tempdir)
        shutil.rmtree(tempdir)
    else:
        error(usage(argv[0]))

    return 0


if __name__ == "__main__":
    main(sys.argv)
