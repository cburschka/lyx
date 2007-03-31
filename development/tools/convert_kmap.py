#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file convert_kmap.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Georg Baum

# Full author contact details are available in file CREDITS

# This script converts a kmap file from LaTeX commands to unicode characters
# The kmap file is read and written in utf8 encoding


import os, re, string, sys, unicodedata

def usage(prog_name):
    return ("Usage: %s unicodesymbolsfile inputfile outputfile\n" % prog_name +
            "or     %s unicodesymbolsfile <inputfile >outputfile" % prog_name)


def error(message):
    sys.stderr.write(message + '\n')
    sys.exit(1)


def trim_eol(line):
    " Remove end of line char(s)."
    if line[-2:-1] == '\r':
        return line[:-2]
    elif line[-1:] == '\r' or line[-1:] == '\n':
        return line[:-1]
    else:
        # file with no EOL in last line
        return line


def read(input):
    " Read input file and strip lineendings."
    lines = list()
    while 1:
        line = input.readline()
        if not line:
            break
        lines.append(trim_eol(line).decode('utf8'))
    return lines


def escape(word):
    " Escape a word for LyXLex."
    re_quote = re.compile(r'\s|,')
    retval = u''
    i = 0
    for c in word:
        if c == '\\' or c == '"' or c == '#':
            retval = retval + u'\\'
        retval = retval + c
    if re_quote.match(retval):
        return u'"%s"' % retval
    return retval


def unescape(word):
    " Unescape a LyXLex escaped word."
    if len(word) > 1 and word[0] == '"' and word[-1] == '"':
        start = 1
        stop = len(word) - 1
    else:
        start = 0
        stop = len(word)
    retval = u''
    i = start
    while i < stop:
        if word[i] == '\\' and i < stop - 1:
            i = i + 1
        retval = retval + word[i]
        i = i + 1
    return retval


def readsymbols(input):
    " Build the symbol list from the unicodesymbols file and add some hardcoded symbols."
    symbols = list()
    while 1:
        line = input.readline()
        if not line:
            break
        line = trim_eol(line)
        tokens = line.split()
        if len(tokens) > 0 and tokens[0][0] != '#':
            if len(tokens) > 1:
                tokens[1] = unescape(tokens[1])
            if tokens[0][0:2] == "0x":
                tokens[0] = int(tokens[0][2:], 16)
                symbols.append(tokens)
    # special cases from .cdef files (e.g. duplicates with different commands)
    symbols.append([0x00a1, '\\nobreakspace'])
    symbols.append([0x00a7, '\\S'])
    symbols.append([0x00a9, '\\copyright'])
    symbols.append([0x00b1, '$\\pm$'])
    symbols.append([0x00b5, '$\\mu$'])
    symbols.append([0x00b7, '$\\cdot$'])
    symbols.append([0x00b9, '$\\mathonesuperior$'])
    symbols.append([0x00d7, '$\\times$'])
    symbols.append([0x00d7, '\\times'])
    symbols.append([0x00f7, '\\div'])
    symbols.append([0x20ac, '\\euro'])
    # special caron, see lib/lyx2lyx/lyx_1_5.py for an explanation
    symbols.append([0x030c, '\\q', '', 'combining'])
    return symbols


def write(output, lines):
    " Write output file with native lineendings."
    for line in lines:
        output.write(line.encode('utf8') + os.linesep)


def translate_symbol(unicodesymbols, symbol, try_combining = True):
    " Translate a symbol from LaTeX to unicode."
    re_combining = re.compile(r'^[^a-zA-Z]')
    if len(symbol) == 1:
        return symbol
    for i in unicodesymbols:
        # Play safe and don't try combining symbols (not sure if this is
        # needed)
        if i[1] == symbol and (len(i) < 4 or i[3].find('combining') < 0):
            return unichr(i[0])
    if try_combining:
        # no direct match, see whether this is a combining sequence
        for i in unicodesymbols:
            if len(i) > 3 and i[3].find('combining') >= 0 and symbol.find(i[1]) == 0:
                # Test whether this is really a combining sequence, e.g.
                # \"o or \d{o}, and not a symbol like \dh that shares the
                # beginning with a combining symbol
                translated = symbol[len(i[1]):]
                if translated != '' and re_combining.match(translated):
                    # Really a combining sequence
                    if len(translated) > 1 and translated[0] == '{' and translated[-1] == '}':
                        # Strip braces from things like \d{o}
                        translated = translated[1:-1]
                    else:
                        # for some strange reason \\'\\i does not get
                        # correctly combined, so we try \\'\\i which has an
                        # entry in unicodesymbols
                        combined = translate_symbol(unicodesymbols, u'%s{%s}' % (i[1], translated))
                        if combined != '':
                            return combined
                    if len(translated) > 1:
                        # The base character may be a symbol itself, e.g \"{\i}
                        translated = translate_symbol(unicodesymbols, translated, False)
                    # Play safe and only translate combining sequences with
                    # one base character
                    if len(translated) == 1 and (i[1] != '\\q' or translated in ['t', 'd', 'l', 'L']):
                        return unicodedata.normalize("NFKC", translated + unichr(i[0]))
                    else:
                        # we founed a combining character, but could not convert the argument to a single character
                        return ''
    return ''


def convert(lines, unicodesymbols):
    " Translate all symbols in lines from LaTeX to unicode."
    # convert both commented and active entries
    re_kmap = re.compile(r'^(#?\s*\\kmap\s+\S+\s+)([^\s]+)(.*)$')
    re_kxmod = re.compile(r'^(#?\s*\\kxmod\s+\S+\s+\S+\s+)([^\s]+)(.*)$')
    for i in range(len(lines)):
        match = re_kmap.match(lines[i])
        if not match:
            match = re_kxmod.match(lines[i])
        if match:
            symbol = unescape(match.group(2))
            if len(symbol) > 2 and symbol[-2:] == '{}':
                # The unicodesymbols file does not include the trailing delimiter {}
                symbol = symbol[0:-2]
            translated = translate_symbol(unicodesymbols, symbol)
            if translated == '':
                lines[i] = u'%s%s%s' % (match.group(1), match.group(2), match.group(3))
            else:
                lines[i] = u'%s%s%s' % (match.group(1), escape(translated), match.group(3))
                continue


def main(argv):

    # Open files
    if len(argv) == 2:
        input = sys.stdin
        output = sys.stdout
    elif len(argv) == 4:
        input = open(argv[2], 'rb')
        output = open(argv[3], 'wb')
    else:
        error(usage(argv[0]))
    unicodesymbols = open(argv[1], 'rb')

    # Do the real work
    symbols = readsymbols(unicodesymbols)
    lines = read(input)
    convert(lines, symbols)
    write(output, lines)

    # Close files
    if len(argv) == 3:
        input.close()
        output.close()

    return 0


if __name__ == "__main__":
    main(sys.argv)
