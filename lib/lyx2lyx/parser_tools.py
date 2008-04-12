# This file is part of lyx2lyx
# -*- coding: utf-8 -*-
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

" This modules offer several free functions to help parse lines."

# Utilities for one line
def check_token(line, token):
    """ check_token(line, token) -> bool

    Return True if token is present in line and is the first element
    else returns False."""

    return line[:len(token)] == token


def is_nonempty_line(line):
    """ is_nonempty_line(line) -> bool

    Return False if line is either empty or it has only whitespaces,
    else return True."""
    return line != " "*len(line)


# Utilities for a list of lines
def find_token(lines, token, start, end = 0, exact = False):
    """ find_token(lines, token, start[[, end], exact]) -> int

    Return the lowest line where token is found, and is the first
    element, in lines[start, end].

    Return -1 on failure."""

    if end == 0:
        end = len(lines)
    m = len(token)
    for i in xrange(start, end):
        if exact:
            x = lines[i].split()
            y = token.split()
            if len(x) < len(y):
                continue
            if x[:len(y)] == y:
                return i
        else:
            if lines[i][:m] == token:
                return i
    return -1


def find_token_exact(lines, token, start, end = 0):
    return find_token(lines, token, start, end, True)


def find_tokens(lines, tokens, start, end = 0, exact = False):
    """ find_tokens(lines, tokens, start[[, end], exact]) -> int

    Return the lowest line where one token in tokens is found, and is
    the first element, in lines[start, end].

    Return -1 on failure."""
    if end == 0:
        end = len(lines)

    for i in xrange(start, end):
        for token in tokens:
            if exact:
                x = lines[i].split()
                y = token.split()
                if len(x) < len(y):
                    continue
                if x[:len(y)] == y:
                    return i
            else:
                if lines[i][:len(token)] == token:
                    return i
    return -1


def find_tokens_exact(lines, tokens, start, end = 0):
    return find_tokens(lines, tokens, start, end, True)


def find_re(lines, rexp, start, end = 0):
    """ find_token_re(lines, rexp, start[, end]) -> int

    Return the lowest line where rexp, a regular expression, is found
    in lines[start, end].

    Return -1 on failure."""

    if end == 0:
        end = len(lines)
    for i in xrange(start, end):
        if rexp.match(lines[i]):
                return i
    return -1


def find_token_backwards(lines, token, start):
    """ find_token_backwards(lines, token, start) -> int

    Return the highest line where token is found, and is the first
    element, in lines[start, end].

    Return -1 on failure."""
    m = len(token)
    for i in xrange(start, -1, -1):
        line = lines[i]
        if line[:m] == token:
            return i
    return -1


def find_tokens_backwards(lines, tokens, start):
    """ find_tokens_backwards(lines, token, start) -> int

    Return the highest line where token is found, and is the first
    element, in lines[end, start].

    Return -1 on failure."""
    for i in xrange(start, -1, -1):
        line = lines[i]
        for token in tokens:
            if line[:len(token)] == token:
                return i
    return -1


def get_value(lines, token, start, end = 0, default = ""):
    """ get_value(lines, token, start[[, end], default]) -> list of strings

    Return tokens after token for the first line, in lines, where
    token is the first element."""

    i = find_token_exact(lines, token, start, end)
    if i == -1:
        return default
    if len(lines[i].split()) > 1:
        return lines[i].split()[1]
    else:
        return default


def get_value_string(lines, token, start, end = 0, trim = False, default = ""):
    """ get_value_string(lines, token, start[[, end], trim, default]) -> string

    Return tokens after token as string, in lines, where
    token is the first element."""

    i = find_token_exact(lines, token, start, end)
    if i == -1:
        return default
    if len(lines[i].split()) > 1:
        for k in range (0, len(lines[i])):
            if lines[i][k] == ' ':
                if trim ==False:
                    return lines[i][k+1:len(lines[i])]
                else:
                    return lines[i][k+2:len(lines[i])-1]
    else:
        return default


def del_token(lines, token, start, end):
    """ del_token(lines, token, start, end) -> int

    Find the lower line in lines where token is the first element and
    delete that line.

    Returns the number of lines remaining."""

    k = find_token_exact(lines, token, start, end)
    if k == -1:
        return end
    else:
        del lines[k]
        return end - 1


def find_beginning_of(lines, i, start_token, end_token):
    count = 1
    while i > 0:
        i = find_tokens_backwards(lines, [start_token, end_token], i-1)
        if i == -1:
            return -1
        if check_token(lines[i], end_token):
            count = count+1
        else:
            count = count-1
        if count == 0:
            return i
    return -1


def find_end_of(lines, i, start_token, end_token):
    count = 1
    n = len(lines)
    while i < n:
        i = find_tokens(lines, [end_token, start_token], i+1)
        if i == -1:
            return -1
        if check_token(lines[i], start_token):
            count = count+1
        else:
            count = count-1
        if count == 0:
            return i
    return -1


def find_nonempty_line(lines, start, end = 0):
    if end == 0:
        end = len(lines)
    for i in xrange(start, end):
        if is_nonempty_line(lines[i]):
            return i
    return -1
