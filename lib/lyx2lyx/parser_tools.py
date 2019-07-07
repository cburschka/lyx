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


"""
This module offers several free functions to help parse lines.
More documentaton is below, but here is a quick guide to what
they do. Optional arguments are marked by brackets.

find_token(lines, token[, start[, end[, ignorews]]]):
  Returns the first line i, start <= i < end, on which
  token is found at the beginning. Returns -1 if not
  found.
  If ignorews is (given and) True, then differences
  in whitespace do not count, except that there must be no
  extra whitespace following token itself.

find_token_exact(lines, token[, start[, end]]]):
  As find_token, but with ignorews set to True.

find_tokens(lines, tokens[, start[, end[, ignorews]]]):
  Returns the first line i, start <= i < end, on which
  one of the tokens in tokens is found at the beginning.
  Returns -1 if not found.
  If ignorews is (given and) True, then differences
  in whitespace do not count, except that there must be no
  extra whitespace following token itself.

find_tokens_exact(lines, token[, start[, end]]):
  As find_tokens, but with ignorews True.

find_token_backwards(lines, token, start):
find_tokens_backwards(lines, tokens, start):
  As before, but look backwards.

find_substring(lines, sub[, start[, end]]) -> int
  As find_token, but sub may be anywhere in the line.

find_re(lines, rexp, start[, end]):
  As find_token, but rexp is a regular expression object,
  so it has to be passed as e.g.: re.compile(r'...').

get_value(lines, token[, start[, end[, default[, delete]]]]):
  Similar to find_token, but it returns what follows the
  token on the found line. Example:
    get_value(document.header, "\\use_xetex", 0)
  will find a line like:
    \\use_xetex true
  and, in that case, return "true". (Note that whitespace
  is stripped.) The final argument, default, defaults to "",
  and is what is returned if we do not find anything. So you
  can use that to set a default.

get_quoted_value(lines, token[, start[, end[, default[, delete]]]]):
  Similar to get_value, but it will strip quotes off the
  value, if they are present. So use this one for cases
  where the value is normally quoted.

get_option_value(line, option):
  This assumes we have a line with something like:
      option="value"
  and returns value. Returns "" if not found.

get_bool_value(lines, token[, start[, end[, default, delete]]]]):
  Like get_value, but returns a boolean.

set_bool_value(lines, token, value[, start[, end]]):
  Find `token` in `lines[start:end]` and set to boolean value bool(`value`).
  Return old value. Raise ValueError if token is not in lines.

del_token(lines, token[, start[, end]]):
  Like find_token, but deletes the line if it finds one.
  Returns True if a line got deleted, otherwise False.

  Use get_* with the optional argument "delete=True", if you want to
  get and delete a token.

find_beginning_of(lines, i, start_token, end_token):
  Here, start_token and end_token are meant to be a matching
  pair, like "\\begin_layout" and "\\end_layout". We look for
  the start_token that pairs with the end_token that occurs
  on or after line i. Returns -1 if not found.
  So, in the layout case, this would find the \\begin_layout
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

find_end_of_sequence(lines, i):
  Find the end of the sequence of layouts of the same kind.
  Considers nesting. If the last paragraph in sequence is nested,
  the position of the last \end_deeper is returned, else
  the position of the last \end_layout.

is_in_inset(lines, i, inset, default=(-1,-1)):
  Check if line i is in an inset of the given type.
  If so, returns starting and ending lines. Otherwise,
  return default.
  Example:
    is_in_inset(document.body, i, "\\begin_inset Tabular")
  returns (-1,-1) unless i is within a table. If it is, then
  it returns the line on which the table begins and the one
  on which it ends. Note that this pair will evaulate to
  boolean True, so
    if is_in_inset(..., default=False):
  will do what you expect.

get_containing_inset(lines, i):
  Finds out what kind of inset line i is within. Returns a
  list containing what follows \begin_inset on the line
  on which the inset begins, plus the starting and ending line.
  Returns False on any kind of error or if it isn't in an inset.
  So get_containing_inset(document.body, i) might return:
    ("CommandInset ref", 300, 306)
  if i is within an InsetRef beginning on line 300 and ending
  on line 306.

get_containing_layout(lines, i):
  As get_containing_inset, but for layout. Additionally returns the
  position of real paragraph start (after par params) as 4th value.

find_nonempty_line(lines, start[, end):
  Finds the next non-empty line.

check_token(line, token):
  Does line begin with token?

is_nonempty_line(line):
  Does line contain something besides whitespace?

count_pars_in_inset(lines, i):
  Counts the paragraphs inside an inset.

"""

import re

# Utilities for one line
def check_token(line, token):
    """ check_token(line, token) -> bool

    Return True if token is present in line and is the first element
    else returns False.

    Deprecated. Use line.startswith(token).
    """
    return line.startswith(token)


def is_nonempty_line(line):
    """ is_nonempty_line(line) -> bool

    Return False if line is either empty or it has only whitespaces,
    else return True."""
    return bool(line.strip())


# Utilities for a list of lines
def find_token(lines, token, start=0, end=0, ignorews=False):
    """ find_token(lines, token, start[[, end], ignorews]) -> int

    Return the lowest line where token is found, and is the first
    element, in lines[start, end].

    If ignorews is True (default is False), then differences in
    whitespace are ignored, but there must be whitespace following
    token itself.

    Use find_substring(lines, sub) to find a substring anywhere in `lines`.

    Return -1 on failure."""

    if end == 0 or end > len(lines):
        end = len(lines)
    if ignorews:
        y = token.split()
    for i in range(start, end):
        if ignorews:
            x = lines[i].split()
            if len(x) < len(y):
                continue
            if x[:len(y)] == y:
                return i
        else:
            if lines[i].startswith(token):
                return i
    return -1


def find_token_exact(lines, token, start=0, end=0):
    return find_token(lines, token, start, end, True)


def find_tokens(lines, tokens, start=0, end=0, ignorews=False):
    """ find_tokens(lines, tokens, start[[, end], ignorews]) -> int

    Return the lowest line where one token in tokens is found, and is
    the first element, in lines[start, end].

    Return -1 on failure."""

    if end == 0 or end > len(lines):
        end = len(lines)

    for i in range(start, end):
        for token in tokens:
            if ignorews:
                x = lines[i].split()
                y = token.split()
                if len(x) < len(y):
                    continue
                if x[:len(y)] == y:
                    return i
            else:
                if lines[i].startswith(token):
                    return i
    return -1


def find_tokens_exact(lines, tokens, start=0, end=0):
    return find_tokens(lines, tokens, start, end, True)


def find_substring(lines, sub, start=0, end=0):
    """ find_substring(lines, sub[, start[, end]]) -> int

    Return the lowest line number `i` in [start, end] where
    `sub` is a substring of line[i].

    Return -1 on failure."""

    if end == 0 or end > len(lines):
        end = len(lines)
    for i in range(start, end):
        if sub in lines[i]:
                return i
    return -1


def find_re(lines, rexp, start=0, end=0):
    """ find_re(lines, rexp[, start[, end]]) -> int

    Return the lowest line number `i` in [start, end] where the regular
    expression object `rexp` matches at the beginning of line[i].
    Return -1 on failure.

    Start your pattern with the wildcard ".*" to find a match anywhere in a
    line. Use find_substring() to find a substring anywhere in the lines.
    """
    if end == 0 or end > len(lines):
        end = len(lines)
    for i in range(start, end):
        if rexp.match(lines[i]):
                return i
    return -1


def find_token_backwards(lines, token, start):
    """ find_token_backwards(lines, token, start) -> int

    Return the highest line where token is found, and is the first
    element, in lines[start, end].

    Return -1 on failure."""
    for i in range(start, -1, -1):
        if lines[i].startswith(token):
            return i
    return -1


def find_tokens_backwards(lines, tokens, start):
    """ find_tokens_backwards(lines, token, start) -> int

    Return the highest line where token is found, and is the first
    element, in lines[end, start].

    Return -1 on failure."""
    for i in range(start, -1, -1):
        line = lines[i]
        for token in tokens:
            if line.startswith(token):
                return i
    return -1


def find_complete_lines(lines, sublines, start=0, end=0):
    """Find first occurence of sequence `sublines` in list `lines`.
    Return index of first line or -1 on failure.

    Efficient search for a sub-list in a large list. Works for any values.

    >>> find_complete_lines([1, 2, 3, 1, 1, 2], [1, 2])
    0

    The `start` and `end` arguments work similar to list.index()

    >>> find_complete_lines([1, 2, 3, 1, 1 ,2], [1, 2], start=1)
    4
    >>> find_complete_lines([1, 2, 3, 1, 1 ,2], [1, 2], start=1, end=4)
    -1

    The return value can be used to substitute the sub-list.
    Take care to check before use:

    >>> l = [1, 1, 2]
    >>> s = find_complete_lines(l, [1, 2])
    >>> if s != -1:
    ...     l[s:s+2] = [3]; l
    [1, 3]

    See also del_complete_lines().
    """
    if not sublines:
        return start
    end = end or len(lines)
    N = len(sublines)
    try:
        while True:
            for j, value in enumerate(sublines):
                i = lines.index(value, start, end)
                if j and i != start:
                    start = i-j
                    break
                start = i + 1
            else:
                return i +1 - N
    except ValueError: # `sublines` not found
        return -1


def find_across_lines(lines, sub, start=0, end=0):
    sublines = sub.splitlines()
    if len(sublines) > 2:
        # at least 3 lines: the middle one(s) are complete -> use index search
        i = find_complete_lines(lines, sublines[1:-1], start+1, end-1)
        if i < start+1:
            return -1
        try:
            if (lines[i-1].endswith(sublines[0]) and
                lines[i+len(sublines)].startswith(sublines[-1])):
                return i-1
        except IndexError:
            pass
    elif len(sublines) > 1:
        # last subline must start a line
        i = find_token(lines, sublines[-1], start, end)
        if i < start + 1:
            return -1
        if lines[i-1].endswith(sublines[0]):
            return i-1
    else: # no line-break, may be in the middle of a line
        if end == 0 or end > len(lines):
            end = len(lines)
        for i in range(start, end):
            if sub in lines[i]:
                return i
    return -1


def get_value(lines, token, start=0, end=0, default="", delete=False):
    """Find `token` in `lines` and return part of line that follows it.

    Find the next line that looks like:
      token followed by other stuff

    If `delete` is True, delete the line (if found).

    Return "followed by other stuff" with leading and trailing
    whitespace removed.
    """
    i = find_token_exact(lines, token, start, end)
    if i == -1:
        return default
    # TODO: establish desired behaviour, eventually change to
    #  return lines.pop(i)[len(token):].strip() # or default
    # see test_parser_tools.py
    l = lines[i].split(None, 1)
    if delete:
        del(lines[i])
    if len(l) > 1:
        return l[1].strip()
    return default


def get_quoted_value(lines, token, start=0, end=0, default="", delete=False):
    """ get_quoted_value(lines, token, start[[, end], default]) -> string

    Find the next line that looks like:
      token "followed by other stuff"
    Returns "followed by other stuff" with leading and trailing
    whitespace and quotes removed. If there are no quotes, that is OK too.
    So use get_value to preserve possible quotes, this one to remove them,
    if they are there.
    Note that we will NOT strip quotes from default!
    """
    val = get_value(lines, token, start, end, "", delete)
    if not val:
      return default
    return val.strip('"')


bool_values = {"true": True, "1": True,
               "false": False, "0": False}

def get_bool_value(lines, token, start=0, end=0, default=None, delete=False):
    """ get_bool_value(lines, token, start[[, end], default]) -> string

    Find the next line that looks like:
      `token` <bool_value>

    Return True if <bool_value> is 1 or "true", False if <bool_value>
    is 0 or "false", else `default`.
    """
    val = get_quoted_value(lines, token, start, end, default, delete)
    return bool_values.get(val, default)


def set_bool_value(lines, token, value, start=0, end=0):
    """Find `token` in `lines` and set to bool(`value`).

    Return previous value. Raise `ValueError` if `token` is not in lines.

    Cf. find_token(), get_bool_value().
    """
    i = find_token(lines, token, start, end)
    if i == -1:
        raise ValueError
    oldvalue = get_bool_value(lines, token, i, i+1)
    if oldvalue is value:
        return oldvalue
    # set to new value
    if get_quoted_value(lines, token, i, i+1) in ('0', '1'):
        lines[i] = "%s %d" % (token, value)
    else:
        lines[i] = "%s %s" % (token, str(value).lower())

    return oldvalue


def get_option_value(line, option):
    rx = option + '\s*=\s*"([^"]+)"'
    rx = re.compile(rx)
    m = rx.search(line)
    if not m:
      return ""
    return m.group(1)


def set_option_value(line, option, value):
    rx = '(' + option + '\s*=\s*")[^"]+"'
    rx = re.compile(rx)
    m = rx.search(line)
    if not m:
        return line
    return re.sub(rx, '\g<1>' + value + '"', line)


def del_token(lines, token, start=0, end=0):
    """ del_token(lines, token, start, end) -> int

    Find the first line in lines where token is the first element
    and delete that line. Returns True if we deleted a line, False
    if we did not."""

    k = find_token_exact(lines, token, start, end)
    if k == -1:
        return False
    del lines[k]
    return True

def del_complete_lines(lines, sublines, start=0, end=0):
    """Delete first occurence of `sublines` in list `lines`.

    Efficient deletion of a sub-list in a list. Works for any values.
    The `start` and `end` arguments work similar to list.index()

    Returns True if a deletion was done and False if not.

    >>> l = [1, 0, 1, 1, 1, 2]
    >>> del_complete_lines(l, [0, 1, 1])
    True
    >>> l
    [1, 1, 2]
    """
    i = find_complete_lines(lines, sublines, start, end)
    if i == -1:
        return False
    del(lines[i:i+len(sublines)])
    return True


def del_value(lines, token, start=0, end=0, default=None):
    """
    Find the next line that looks like:
      token followed by other stuff
    Delete that line and return "followed by other stuff"
    with leading and trailing whitespace removed.

    If token is not found, return `default`.
    """
    i = find_token_exact(lines, token, start, end)
    if i == -1:
        return default
    return lines.pop(i)[len(token):].strip()


def find_beginning_of(lines, i, start_token, end_token):
    count = 1
    while i > 0:
        i = find_tokens_backwards(lines, [start_token, end_token], i-1)
        if i == -1:
            return -1
        if lines[i].startswith(end_token):
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
        if lines[i].startswith(start_token):
            count = count+1
        else:
            count = count-1
        if count == 0:
            return i
    return -1


def find_nonempty_line(lines, start=0, end=0):
    if end == 0:
        end = len(lines)
    for i in range(start, end):
        if lines[i].strip():
            return i
    return -1


def find_end_of_inset(lines, i):
    " Find end of inset, where lines[i] is included."
    return find_end_of(lines, i, "\\begin_inset", "\\end_inset")


def find_end_of_layout(lines, i):
    " Find end of layout, where lines[i] is included."
    return find_end_of(lines, i, "\\begin_layout", "\\end_layout")


def is_in_inset(lines, i, inset, default=(-1,-1)):
    """
    Check if line i is in an inset of the given type.
    If so, return starting and ending lines, otherwise `default`.
    Example:
      is_in_inset(document.body, i, "\\begin_inset Tabular")
    returns (-1,-1) if `i` is not within a "Tabular" inset (i.e. a table).
    If it is, then it returns the line on which the table begins and the one
    on which it ends.
    Note that this pair will evaulate to boolean True, so (with the optional
    default value set to False)
      if is_in_inset(..., default=False):
    will do what you expect.
    """
    start = find_token_backwards(lines, inset, i)
    if start == -1:
      return default
    end = find_end_of_inset(lines, start)
    if end < i: # this includes the notfound case.
      return default
    return (start, end)


def get_containing_inset(lines, i):
  '''
  Finds out what kind of inset line i is within. Returns a
  list containing (i) what follows \begin_inset on the line
  on which the inset begins, plus the starting and ending line.
  Returns False on any kind of error or if it isn't in an inset.
  '''
  j = i
  while True:
      stins = find_token_backwards(lines, "\\begin_inset", j)
      if stins == -1:
          return False
      endins = find_end_of_inset(lines, stins)
      if endins > j:
          break
      j = stins - 1

  if endins < i:
      return False

  inset = get_value(lines, "\\begin_inset", stins)
  if inset == "":
      # shouldn't happen
      return False
  return (inset, stins, endins)


def get_containing_layout(lines, i):
  '''
  Find out what kind of layout line `i` is within.
  Return a tuple
    (layoutname, layoutstart, layoutend, startofcontent)
  containing
    * layout style/name,
    * start line number,
    * end line number, and
    * number of first paragraph line (after all params).
  Return `False` on any kind of error.
  '''
  j = i
  while True:
      stlay = find_token_backwards(lines, "\\begin_layout", j)
      if stlay == -1:
          return False
      endlay = find_end_of_layout(lines, stlay)
      if endlay > i:
          break
      j = stlay - 1

  if endlay < i:
      return False

  layoutname = get_value(lines, "\\begin_layout", stlay)
  if layoutname == "": # layout style missing
      # TODO: What shall we do in this case?
      pass
      # layoutname == "Standard" # use same fallback as the LyX parser:
      # raise ValueError("Missing layout name on line %d"%stlay) # diagnosis
      # return False # generic error response
  par_params = ["\\noindent", "\\indent", "\\indent-toggle", "\\leftindent",
                "\\start_of_appendix", "\\paragraph_spacing", "\\align",
                "\\labelwidthstring"]
  stpar = stlay
  while True:
      stpar += 1
      if lines[stpar].split(' ', 1)[0] not in par_params:
          break
  return (layoutname, stlay, endlay, stpar)


def count_pars_in_inset(lines, i):
  '''
  Counts the paragraphs within this inset
  '''
  ins = get_containing_inset(lines, i)
  if ins == -1:
      return -1
  pars = 0
  for j in range(ins[1], ins[2]):
      m = re.match(r'\\begin_layout (.*)', lines[j])
      if m and get_containing_inset(lines, j)[0] == ins[0]:
          pars += 1

  return pars


def find_end_of_sequence(lines, i):
  '''
  Returns the end of a sequence of identical layouts.
  '''
  lay = get_containing_layout(lines, i)
  if lay == False:
      return -1
  layout = lay[0]
  endlay = lay[2]
  i = endlay
  while True:
      m = re.match(r'\\begin_layout (.*)', lines[i])
      if m and m.group(1) != layout:
          return endlay
      elif lines[i] == "\\begin_deeper":
          j = find_end_of(lines, i, "\\begin_deeper", "\\end_deeper")
          if j != -1:
              i = j
              endlay = j
              continue
      if m and m.group(1) == layout:
          endlay = find_end_of_layout(lines, i)
          i = endlay
          continue
      if i == len(lines) - 1:
          break
      i = i + 1

  return endlay
