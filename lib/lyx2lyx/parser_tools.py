# This file is part of lyx2lyx
# -*- coding: utf-8 -*-
# Copyright (C) 2002-2011 Dekel Tsur <dekel@lyx.org>, 
# José Matos <jamatos@lyx.org>, Richard Heck <rgheck@comcast.net>
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


''' 
This modules offer several free functions to help parse lines.
More documentaton is below, but here is a quick guide to what 
they do. Optional arguments are marked by brackets.

find_token(lines, token, start[, end[, ignorews]]):
  Returns the first line i, start <= i < end, on which
  token is found at the beginning. Returns -1 if not 
  found. 
  If ignorews is (given and) True, then differences
  in whitespace do not count, except that there must be no 
  extra whitespace following token itself.

find_token_exact(lines, token, start[, end]):
  As find_token, but with ignorews True.

find_tokens(lines, tokens, start[, end[, ignorews]]):
  Returns the first line i, start <= i < end, on which
  oen of the tokens in tokens is found at the beginning. 
  Returns -1 if not found. 
  If ignorews is (given and) True, then differences
  in whitespace do not count, except that there must be no 
  extra whitespace following token itself.

find_tokens_exact(lines, token, start[, end]):
  As find_tokens, but with ignorews True.
  
find_token_backwards(lines, token, start):
find_tokens_backwards(lines, tokens, start):
  As before, but look backwards.

find_re(lines, rexp, start[, end]):
  As find_token, but rexp is a regular expression object,
  so it has to be passed as e.g.: re.compile(r'...').

get_value(lines, token, start[, end[, default]):
  Similar to find_token, but it returns what follows the 
  token on the found line. Example:
    get_value(document.header, "\use_xetex", 0)
  will find a line like:
    \use_xetex true
  and, in that case, return "true". (Note that whitespace
  is stripped.) The final argument, default, defaults to "", 
  and is what is returned if we do not find anything. So you
  can use that to set a default.
  
get_quoted_value(lines, token, start[, end[, default]):
  Similar to get_value, but it will strip quotes off the
  value, if they are present. So use this one for cases
  where the value is normally quoted.

get_option_value(line, option):
  This assumes we have a line with something like:
      option="value"
  and returns value. Returns "" if not found.

del_token(lines, token, start[, end]):
  Like find_token, but deletes the line if it finds one.
  Returns True if a line got deleted, otherwise False.

find_beginning_of(lines, i, start_token, end_token):
  Here, start_token and end_token are meant to be a matching 
  pair, like "\begin_layout" and "\end_layout". We look for 
  the start_token that pairs with the end_token that occurs
  on or after line i. Returns -1 if not found.
  So, in the layout case, this would find the \begin_layout 
  for the layout line i is in. 
  Example:
    ec = find_token(document.body, "</cell", i)
    bc = find_beginning_of(document.body, ec, \
        "<cell", "</cell")
  Now, assuming no -1s, bc-ec wraps the cell for line i.

find_end_of(lines, i, start_token, end_token):
  Like find_beginning_of, but looking for the matching 
  end_token. This might look like:
    bc = find_token_(document.body, "<cell", i)
    ec = find_end_of(document.body, bc,  "<cell", "</cell")
  Now, assuming no -1s, bc-ec wrap the next cell.

find_end_of_inset(lines, i):
  Specialization of find_end_of for insets.

find_end_of_layout(lines, i):
  Specialization of find_end_of for layouts.

is_in_inset(lines, i, inset):
  Checks if line i is in an inset of the given type.
  If so, returns starting and ending lines. Otherwise, 
  returns False.
  Example:
    is_in_inset(document.body, i, "\\begin_inset Tabular")
  returns False unless i is within a table. If it is, then
  it returns the line on which the table begins and the one
  on which it ends. Note that this pair will evaulate to
  boolean True, so
    if is_in_inset(...):
  will do what you expect.

get_containing_inset(lines, i):
  Finds out what kind of inset line i is within. Returns a 
  list containing what follows \begin_inset on the the line 
  on which the inset begins, plus the starting and ending line.
  Returns False on any kind of error or if it isn't in an inset.
  So get_containing_inset(document.body, i) might return:
    ("CommandInset ref", 300, 306)
  if i is within an InsetRef beginning on line 300 and ending
  on line 306.

get_containing_layout(lines, i):
  As get_containing_inset, but for layout.


find_nonempty_line(lines, start[, end):
  Finds the next non-empty line.

check_token(line, token):
  Does line begin with token?

is_nonempty_line(line):
  Does line contain something besides whitespace?

'''

import re

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
def find_token(lines, token, start, end = 0, ignorews = False):
    """ find_token(lines, token, start[[, end], ignorews]) -> int

    Return the lowest line where token is found, and is the first
    element, in lines[start, end].
    
    If ignorews is True (default is False), then differences in
    whitespace are ignored, except that there must be no extra
    whitespace following token itself.

    Return -1 on failure."""

    if end == 0 or end > len(lines):
        end = len(lines)
    m = len(token)
    for i in xrange(start, end):
        if ignorews:
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


def find_tokens(lines, tokens, start, end = 0, ignorews = False):
    """ find_tokens(lines, tokens, start[[, end], ignorews]) -> int

    Return the lowest line where one token in tokens is found, and is
    the first element, in lines[start, end].

    Return -1 on failure."""
    if end == 0 or end > len(lines):
        end = len(lines)

    for i in xrange(start, end):
        for token in tokens:
            if ignorews:
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

    if end == 0 or end > len(lines):
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
    """ get_value(lines, token, start[[, end], default]) -> string

    Find the next line that looks like:
      token followed by other stuff
    Returns "followed by other stuff" with leading and trailing
    whitespace removed.
    """

    i = find_token_exact(lines, token, start, end)
    if i == -1:
        return default
    l = lines[i].split(None, 1)
    if len(l) > 1:
        return l[1].strip()
    return default


def get_quoted_value(lines, token, start, end = 0, default = ""):
    """ get_quoted_value(lines, token, start[[, end], default]) -> string

    Find the next line that looks like:
      token "followed by other stuff"
    Returns "followed by other stuff" with leading and trailing
    whitespace and quotes removed. If there are no quotes, that is OK too.
    So use get_value to preserve possible quotes, this one to remove them,
    if they are there.
    Note that we will NOT strip quotes from default!
    """
    val = get_value(lines, token, start, end, "")
    if not val:
      return default
    return val.strip('"')


def get_option_value(line, option):
    rx = option + '\s*=\s*"([^"+])"'
    rx = re.compile(rx)
    m = rx.search(line)
    if not m:
      return ""
    return m.group(1)


def del_token(lines, token, start, end = 0):
    """ del_token(lines, token, start, end) -> int

    Find the first line in lines where token is the first element 
    and delete that line. Returns True if we deleted a line, False
    if we did not."""

    k = find_token_exact(lines, token, start, end)
    if k == -1:
        return False
    del lines[k]
    return True


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


def find_end_of_inset(lines, i):
    " Find end of inset, where lines[i] is included."
    return find_end_of(lines, i, "\\begin_inset", "\\end_inset")


def find_end_of_layout(lines, i):
    " Find end of layout, where lines[i] is included."
    return find_end_of(lines, i, "\\begin_layout", "\\end_layout")


def is_in_inset(lines, i, inset):
    '''
    Checks if line i is in an inset of the given type.
    If so, returns starting and ending lines.
    Otherwise, returns False.
    Example:
      is_in_inset(document.body, i, "\\begin_inset Tabular")
    returns False unless i is within a table. If it is, then
    it returns the line on which the table begins and the one
    on which it ends. Note that this pair will evaulate to
    boolean True, so
      if is_in_inset(...):
    will do what you expect.
    '''
    defval = (-1, -1)
    stins = find_token_backwards(lines, inset, i)
    if stins == -1:
      return defval
    endins = find_end_of_inset(lines, stins)
    # note that this includes the notfound case.
    if endins < i:
      return defval
    return (stins, endins)


def get_containing_inset(lines, i):
  ''' 
  Finds out what kind of inset line i is within. Returns a 
  list containing (i) what follows \begin_inset on the the line 
  on which the inset begins, plus the starting and ending line.
  Returns False on any kind of error or if it isn't in an inset.
  '''
  stins = find_token_backwards(lines, i, "\\begin_inset")
  if stins == -1:
      return False
  endins = find_end_of_inset(lines, stins)
  if endins < i:
      return False
  inset = get_value(lines, "\\begin_inset", stins)
  if inset == "":
      # shouldn't happen
      return False
  return (inset, stins, endins)


def get_containing_layout(lines, i):
  ''' 
  Finds out what kind of layout line i is within. Returns a 
  list containing (i) what follows \begin_layout on the the line 
  on which the layout begins, plus the starting and ending line.
  Returns False on any kind of error.
  '''
  stins = find_token_backwards(lines, i, "\\begin_layout")
  if stins == -1:
      return False
  endins = find_end_of_layout(lines, stins)
  if endins < i:
      return False
  lay = get_value(lines, "\\begin_layout", stins)
  if lay == "":
      # shouldn't happen
      return False
  return (lay, stins, endins)
