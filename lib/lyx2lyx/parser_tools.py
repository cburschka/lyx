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
def get_paragraph(lines, i, format):
    if format < 225:
        begin_layout = "\\layout"
    else:
        begin_layout = "\\begin_layout"
    while i != -1:
	i = find_tokens_backwards(lines, ["\\end_inset", begin_layout], i)
        if i == -1: return -1
	if check_token(lines[i], begin_layout):
	    return i
	i = find_beginning_of_inset(lines, i)
    return -1


# Finds the paragraph after the paragraph that contains line i.
def get_next_paragraph(lines, i, format):
    if format < 225:
        tokens = ["\\begin_inset", "\\layout", "\\end_float", "\\the_end"]
    elif format < 236:
        tokens = ["\\begin_inset", "\\begin_layout", "\\end_float", "\\end_document"]
    else:
        tokens = ["\\begin_inset", "\\begin_layout", "\\end_float", "\\end_body", "\\end_document"]
    while i != -1:
	i = find_tokens(lines, tokens, i)
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
