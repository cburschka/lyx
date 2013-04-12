#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file html2latexwrapper.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Georg Baum

# Full author contact details are available in file CREDITS

# Usage:
# html2latexwrapper.py <converter> <from file> <to file>

# This script will call <converter> -s <from file> > <to file>
# and add a \usepackage{inputenc} line if needed.


import os, string, sys, re

from lyxpreview_tools import error, run_command


def usage(prog_name):
    return "Usage: %s <converter> <from file> <to file>" % prog_name


def get_encoding(from_file_name):
    '''Read the encoding from a HTML or XHTML file'''
    try:
        from_file = open(from_file_name, 'rt')
        regexpxml = re.compile(r'^\s?<\?xml\s+.*?encoding\s*=\s*"([^"]+)"', re.IGNORECASE)
        regexptype = re.compile(r'^\s?<meta\s+.*?charset\s*=\s*"([^"]+)"', re.IGNORECASE)
        for line in from_file.readlines():
            m = regexpxml.match(line)
            if not m:
                m = regexptype.match(line)
            if m:
                from_file.close()
                return m.group(1).lower()
        from_file.close()
    except:
        pass
    return ''


def main(argv):
    # Parse and manipulate the command line arguments.
    if len(argv) != 4:
        error(usage(argv[0]))

    converter = argv[1]
    from_file_name = argv[2]
    to_file_name = argv[3]

    # Run gnuhtml2latex
    cmd = '%s -s %s' % (converter, from_file_name)
    (ret, output) = run_command(cmd, False)

    # Determine encoding of HTML file
    enc = get_encoding(from_file_name).replace('iso_8859', 'iso-8859')
    # The HTML encodings were taken from http://www.iana.org/assignments/character-sets/character-sets.xml.
    # Only those with inputenc support were added, and only thge most important aliases.
    # List of encodings that have the same name in HTML (may be as an alias) and inputenc
    same_enc = ['cp437', 'cp850', 'cp852', 'cp855', 'cp858', 'cp862', 'cp865', 'cp866', \
                'cp1250', 'cp1251', 'cp1252', 'cp1255', 'cp1256', 'cp1257', \
                'koi8-r', 'koi8-u', 'pt154', 'pt254', \
                'latin1', 'latin2', 'latin3', 'latin4', 'latin5', 'latin9', 'latin10']
    # Translation table from HTML encoding names to inputenc encoding names
    encodings = {'utf-8' : 'utf8', 'csutf8' : 'utf8', \
                 'iso-8859-1' : 'latin1', 'cp819' : 'latin1', \
                 'iso-8859-2' : 'latin2', \
                 'iso-8859-3' : 'latin3', \
                 'iso-8859-4' : 'latin4', \
                 'iso-8859-5' : 'iso88595', 'cyrillic' : 'iso88595', \
                 'iso-8859-6' : '8859-6', 'arabic' : '8859-6', \
                 'iso-8859-7' : 'iso-8859-7', 'greek' : 'iso-8859-7', \
                 'iso-8859-8' : '8859-8', 'hebrew' : '8859-8', \
                 'iso-8859-9' : 'latin5', \
                 'iso-8859-13' : 'l7xenc', \
                 'iso-8859-15' : 'latin9', \
                 'iso-8859-16' : 'latin10', \
                 'ibm437' : 'cp437', \
                 'ibm850' : 'cp850', \
                 'ibm852' : 'cp852', \
                 'ibm855' : 'cp855', \
                 'ibm858' : 'cp858', \
                 'ibm862' : 'cp862', \
                 'ibm865' : 'cp865', \
                 'ibm866' : 'cp866', \
                 'ibm1250' : 'cp1250', \
                 'ibm1251' : 'cp1251', \
                 'ibm1255' : 'cp1255', \
                 'ibm1256' : 'cp1256', \
                 'ibm1257' : 'cp1257', \
                 'macintosh' : 'applemac', 'mac' : 'applemac', 'csmacintosh' : 'applemac'}
    if enc != '':
        if enc in encodings.keys():
            enc = encodings[enc]
        elif enc not in same_enc:
            enc = ''

    # Read conversion result
    lines = output.split('\n')

    # Do not add the inputenc call if inputenc or CJK is already loaded
    add_inputenc = (enc != '')
    if add_inputenc:
        regexp = re.compile(r'^\s?\\usepackage\s?(\[[^]+]\])?\s?{(inputenc)|(CJK)|(CJKutf8)}')
        for line in lines:
            if regexp.match(line):
                add_inputenc = False
                break

    # Write output file and insert inputenc call if needed
    to_file = open(to_file_name, 'wt')
    for line in lines:
        to_file.write(line + '\n')
        if add_inputenc and line.find('\\documentclass') == 0:
            to_file.write('\\usepackage[%s]{inputenc}\n' % enc)
    to_file.close()

    return ret


if __name__ == "__main__":
    main(sys.argv)
