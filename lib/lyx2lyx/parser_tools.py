# This file is part of lyx2lyx
# -*- coding: iso-8859-1 -*-
# Copyright (C) 2002-2004 Dekel Tsur <dekel@lyx.org>, José Matos <jamatos@lyx.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

import string
import re

def check_token(line, token):
    if line[:len(token)] == token:
	return 1
    return 0

# We need to check that the char after the token is space, but I think
# we can ignore this
def find_token(lines, token, start, end = 0):
    if end == 0:
	end = len(lines)
    m = len(token)
    for i in xrange(start, end):
	if lines[i][:m] == token:
	    return i
    return -1

def find_token2(lines, token, start, end = 0):
    if end == 0:
	end = len(lines)
    for i in xrange(start, end):
	x = string.split(lines[i])
	if len(x) > 0 and x[0] == token:
	    return i
    return -1

def find_tokens(lines, tokens, start, end = 0):
    if end == 0:
	end = len(lines)
    for i in xrange(start, end):
	line = lines[i]
	for token in tokens:
	    if line[:len(token)] == token:
		return i
    return -1

def find_re(lines, rexp, start, end = 0):
    if end == 0:
	end = len(lines)
    for i in xrange(start, end):
	if rexp.match(lines[i]):
		return i
    return -1

def find_token_backwards(lines, token, start):
    m = len(token)
    for i in xrange(start, -1, -1):
	line = lines[i]
	if line[:m] == token:
	    return i
    return -1

def find_tokens_backwards(lines, tokens, start):
    for i in xrange(start, -1, -1):
	line = lines[i]
	for token in tokens:
	    if line[:len(token)] == token:
		return i
    return -1

def get_value(lines, token, start, end = 0):
    i = find_token2(lines, token, start, end)
    if i == -1:
	return ""
    if len(string.split(lines[i])) > 1:
        return string.split(lines[i])[1]
    else:
        return ""

def del_token(lines, token, i, j):
    k = find_token2(lines, token, i, j)
    if k == -1:
	return j
    else:
	del lines[k]
	return j-1

# Finds the paragraph that contains line i.
def get_paragraph(lines, i):
    while i != -1:
	i = find_tokens_backwards(lines, ["\\end_inset", "\\layout"], i)
        if i == -1: return -1
	if check_token(lines[i], "\\layout"):
	    return i
	i = find_beginning_of_inset(lines, i)
    return -1

# Finds the paragraph after the paragraph that contains line i.
def get_next_paragraph(lines, i):
    while i != -1:
	i = find_tokens(lines, ["\\begin_inset", "\\layout", "\\end_float", "\\the_end"], i)
	if not check_token(lines[i], "\\begin_inset"):
	    return i
	i = find_end_of_inset(lines, i)
    return -1

def find_end_of(lines, i, start_token, end_token):
    count = 1
    n = len(lines)
    while i < n:
	i = find_tokens(lines, [end_token, start_token], i+1)
	if check_token(lines[i], start_token):
	    count = count+1
	else:
	    count = count-1
	if count == 0:
	    return i
    return -1

# Finds the matching \end_inset
def find_beginning_of(lines, i, start_token, end_token):
    count = 1
    while i > 0:
	i = find_tokens_backwards(lines, [start_token, end_token], i-1)
	if check_token(lines[i], end_token):
	    count = count+1
	else:
	    count = count-1
	if count == 0:
	    return i
    return -1

# Finds the matching \end_inset
def find_end_of_inset(lines, i):
    return find_end_of(lines, i, "\\begin_inset", "\\end_inset")

# Finds the matching \end_inset
def find_beginning_of_inset(lines, i):
    return find_beginning_of(lines, i, "\\begin_inset", "\\end_inset")

def find_end_of_tabular(lines, i):
    return find_end_of(lines, i, "<lyxtabular", "</lyxtabular")

def get_tabular_lines(lines, i):
    result = []
    i = i+1
    j = find_end_of_tabular(lines, i)
    if j == -1:
	return []

    while i <= j:
	if check_token(lines[i], "\\begin_inset"):
	    i = find_end_of_inset(lines, i)+1
	else:
	    result.append(i)
	    i = i+1
    return result

def is_nonempty_line(line):
    return line != " "*len(line)

def find_nonempty_line(lines, start, end = 0):
    if end == 0:
	end = len(lines)
    for i in xrange(start, end):
	if is_nonempty_line(lines[i]):
	    return i
    return -1

##
# Tools for file reading
#
def read_file(header, body, opt):
    """Reads a file into the header and body parts"""
    preamble = 0

    while 1:
        line = opt.input.readline()
        if not line:
            opt.error("Invalid LyX file.")

        line = line[:-1]
        if check_token(line, '\\begin_preamble'):
            preamble = 1
        if check_token(line, '\\end_preamble'):
            preamble = 0

        if not preamble:
            line = string.strip(line)

        if not line and not preamble:
            break

        header.append(line)

    while 1:
        line = opt.input.readline()
        if not line:
            break
        body.append(line[:-1])

def write_file(header, body, opt):
    for line in header:
        opt.output.write(line+"\n")
    opt.output.write("\n")
    for line in body:
        opt.output.write(line+"\n")

##
# lyx version
#
original_version = re.compile(r"\#LyX (\S*)")

def read_version(header):
    for line in header:
        if line[0] != "#":
            return None

        result = original_version.match(line)
        if result:
            return result.group(1)
    return None

def set_version(lines, version):
    lines[0] = "#LyX %s created this file. For more info see http://www.lyx.org/" % version
    if lines[1][0] == '#':
	del lines[1]

##
# file format version
#
format_re = re.compile(r"(\d)[\.,]?(\d\d)")
fileformat = re.compile(r"\\lyxformat\s*(\S*)")
lst_ft = [210, 215, 216, 217, 218, 220, 221, 223, 224, 225, 226, 227, 228, 229, 
          230, 231, 232]

format_relation = [("0_10",  [210], ["0.10.7","0.10"]),
                   ("0_12",  [215], ["0.12","0.12.1","0.12"]),
                   ("1_0_0", [215], ["1.0.0","1.0"]),
                   ("1_0_1", [215], ["1.0.1","1.0.2","1.0.3","1.0.4", "1.1.2","1.1"]),
                   ("1_1_4", [215], ["1.1.4","1.1"]),
                   ("1_1_5", [216], ["1.1.5","1.1.5fix1","1.1.5fix2","1.1"]),
                   ("1_1_6", [217], ["1.1.6","1.1.6fix1","1.1.6fix2","1.1"]),
                   ("1_1_6fix3", [218], ["1.1.6fix3","1.1.6fix4","1.1"]),
                   ("1_2", [220], ["1.2.0","1.2.1","1.2.3","1.2.4","1.2"]),
                   ("1_3", [221], ["1.3.0","1.3.1","1.3.2","1.3.3","1.3.4","1.3"]),
                   ("1_4", [223,224,225,226,227,228,229,230,231,232], ["1.4.0cvs","1.4"])]

def lyxformat(format, opt):
    result = format_re.match(format)
    if result:
        format = int(result.group(1) + result.group(2))
    else:
        opt.error(str(format) + ": " + "Invalid LyX file.")

    if format in lst_ft:
        return format

    opt.error(str(format) + ": " + "Format no supported.")
    return None

def read_format(header, opt):
    for line in header:
        result = fileformat.match(line)
        if result:
            return lyxformat(result.group(1), opt)
    else:
        opt.error("Invalid LyX File.")
    return None

def set_format(lines, number):
    if int(number) <= 217:
        number = float(number)/100
    i = find_token(lines, "\\lyxformat", 0)
    lines[i] = "\\lyxformat %s" % number

def get_end_format():
    return format_relation[-1:][0][1][-1:][0]

def chain(opt, initial_version):
    """ This is where all the decisions related with the convertion are taken"""

    format = opt.format
    if opt.start:
        if opt.start != format:
            opt.warning("%s: %s %s" % ("Proposed file format and input file formats do not match:", opt.start, format))
    else:
        opt.start = format

    if not opt.end:
        opt.end = get_end_format()

    correct_version = 0

    for rel in format_relation:
        if initial_version in rel[2]:
            if format in rel[1]:
                initial_step = rel[0]
                correct_version = 1
                break

    if not correct_version:
        if format <= 215:
            opt.warning("Version does not match file format, discarding it.")
        for rel in format_relation:
            if format in rel[1]:
                initial_step = rel[0]
                break
        else:
            # This should not happen, really.
            opt.error("Format not supported.")

    # Find the final step
    for rel in format_relation:
        if opt.end in rel[1]:
            final_step = rel[0]
            break
    else:
        opt.error("Format not supported.")
    
    # Convertion mode, back or forth
    steps = []
    if (initial_step, opt.start) < (final_step, opt.end):
        mode = "convert"
        first_step = 1
        for step in format_relation:
            if  initial_step <= step[0] <= final_step:
                if first_step and len(step[1]) == 1:
                    first_step = 0
                    continue
                steps.append(step[0])
    else:
        mode = "revert"
        relation_format = format_relation
        relation_format.reverse()
        last_step = None

        for step in relation_format:
            if  final_step <= step[0] <= initial_step:
                steps.append(step[0])
                last_step = step

        if last_step[1][-1] == opt.end:
            steps.pop()

    return mode, steps
