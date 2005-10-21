#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-

# file layout2layout.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Georg Baum

# Full author contact details are available in file CREDITS

# This script will update a .layout file to format 2


import os, re, string, sys


def usage(prog_name):
    return ("Usage: %s inputfile outputfile\n" % prog_name +
            "or     %s <inputfile >outputfile" % prog_name)


def error(message):
    sys.stderr.write(message + '\n')
    sys.exit(1)


def trim_eol(line):
    " Remove end of line char(s)."
    if line[-2:-1] == '\r':
        return line[:-2]
    elif line[-1:-1] == '\r' or line[-1:-1] == '\n':
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
        lines.append(trim_eol(line))
    return lines


def write(output, lines):
    " Write output file with native lineendings."
    for line in lines:
        output.write(line + os.linesep)


def convert(lines):
    " Convert to new format."
    re_Comment = re.compile(r'^(\s*)#')
    re_Empty = re.compile(r'^(\s*)$')
    re_Format = re.compile(r'^(\s*)(Format)(\s+)(\S+)', re.IGNORECASE)
    re_Preamble = re.compile(r'^(\s*)Preamble', re.IGNORECASE)
    re_EndPreamble = re.compile(r'^(\s*)EndPreamble', re.IGNORECASE)
    re_MaxCounter = re.compile(r'^\s*MaxCounter', re.IGNORECASE)
    re_LabelType = re.compile(r'^(\s*)(LabelType)(\s+)(\S+)', re.IGNORECASE)

    i = 0
    only_comment = 1
    while i < len(lines):

        # Skip comments and empty lines
        if re_Comment.match(lines[i]) or re_Empty.match(lines[i]):
            i = i + 1
            continue

	# insert file format if not already there
        if (only_comment):
                match = re_Format.match(lines[i])
                if match:
                        format = match.group(4)
                        if format == '2':
                                # nothing to do
                                return
                        error('Cannot convert file format %s' % format)
                else:
	                lines.insert(i, "Format 2")
                        only_comment = 0
                        continue

        # Don't get confused by LaTeX code
        if re_Preamble.match(lines[i]):
            i = i + 1
            while i < len(lines) and not re_EndPreamble.match(lines[i]):
                i = i + 1
            continue

        # Delete MaxCounter
        if re_MaxCounter.match(lines[i]):
            del lines[i]
            continue

        # Replace line
        #
        # LabelType Counter_EnumI
        #
        # with two lines
        #
        # LabelType Counter
        # LabelCounter EnumI
        #
        match = re_LabelType.match(lines[i])
        if match:
            label = match.group(4)
            if string.lower(label[:8]) == "counter_":
                counter = label[8:]
                lines[i] = re_LabelType.sub(r'\1\2\3Counter', lines[i])
                # use the same indentation
                space1 = match.group(1)
                lines.insert(i + 1, "%sLabelCounter %s" % (space1, counter))

        i = i + 1


def main(argv):

    # Open files
    if len(argv) == 1:
        input = sys.stdin
        output = sys.stdout
    elif len(argv) == 3:
        input = open(argv[1], 'rb')
        output = open(argv[2], 'wb')
    else:
        error(usage(argv[0]))

    # Do the real work
    lines = read(input)
    convert(lines)
    write(output, lines)

    # Close files
    if len(argv) == 3:
        input.close()
        output.close()

    return 0


if __name__ == "__main__":
    main(sys.argv)
