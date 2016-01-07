#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file unciodesymbols.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Georg Baum

# Full author contact details are available in file CREDITS

# This script reads a unicode symbol file and completes it in the given range


import os, re, string, sys, unicodedata

def usage(prog_name):
    return ("Usage: %s start stop inputfile outputfile\n" % prog_name +
            "or     %s start stop <inputfile >outputfile" % prog_name)


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
        line = trim_eol(line)
        tokens = line.split()
        char = -1
        if len(tokens) > 0:
            if tokens[0][0:2] == "0x":
                char = int(tokens[0][2:], 16)
            elif tokens[0][0:3] == "#0x":
                char = int(tokens[0][3:], 16)
        lines.append([char, line])
    return lines


def write(output, lines):
    " Write output file with native lineendings."
    for line in lines:
        output.write(line[1] + os.linesep)


def complete(lines, start, stop):
    l = 0
    for i in range(start, stop):
        # This catches both comments (lines[l][0] == -1) and code points less than i
        while l < len(lines) and lines[l][0] < i:
#            print lines[l]
            l = l + 1
            continue
        if l >= len(lines) or lines[l][0] != i:
            c = unichr(i)
            name = unicodedata.name(c, "")
            if name != "":
                if unicodedata.combining(c):
                    combining = "combining"
                else:
                    combining = ""
                line = [i, '#0x%04x ""                         "" "%s" "" "" # %s' % (i, combining, name)]
                lines.insert(l, line)
#                print lines[l]
                l = l + 1


def main(argv):

    # Open files
    if len(argv) == 3:
        input = sys.stdin
        output = sys.stdout
    elif len(argv) == 5:
        input = open(argv[3], 'rb')
        output = open(argv[4], 'wb')
    else:
        error(usage(argv[0]))
    if argv[1][:2] == "0x":
        start = int(argv[1][2:], 16)
    else:
        start = int(argv[1])
    if argv[2][:2] == "0x":
        stop = int(argv[2][2:], 16)
    else:
        stop = int(argv[2])

    # Do the real work
    lines = read(input)
    complete(lines, start, stop)
    write(output, lines)

    # Close files
    if len(argv) == 3:
        input.close()
        output.close()

    return 0


if __name__ == "__main__":
    main(sys.argv)
