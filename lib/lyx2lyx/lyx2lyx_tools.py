# This file is part of lyx2lyx
# -*- coding: utf-8 -*-
# Copyright (C) 2011 The LyX team
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

'''
This modules offer several free functions to help with lyx2lyx'ing. 
More documentaton is below, but here is a quick guide to what 
they do. Optional arguments are marked by brackets.

add_to_preamble(document, text):
  Here, text can be either a single line or a list of lines. It
  is bad practice to pass something with embedded newlines, but
  we will handle that properly.
  The routine checks to see whether the provided material is
  already in the preamble. If not, it adds it.
  Prepends a comment "% Added by lyx2lyx" to text.

insert_to_preamble(document, text[, index]):
  Here, text can be either a single line or a list of lines. It
  is bad practice to pass something with embedded newlines, but
  we will handle that properly.
  The routine inserts text at document.preamble[index], where by
  default index is 0, so the material is inserted at the beginning.
  Prepends a comment "% Added by lyx2lyx" to text.

put_cmd_in_ert(arg):
  Here arg should be a list of strings (lines), which we want to
  wrap in ERT. Returns a list of strings so wrapped.
  A call to this routine will often go something like this:
    i = find_token('\\begin_inset FunkyInset', ...)
    j = find_end_of_inset(document.body, i)
    content = lyx2latex(document[i:j + 1])
    ert = put_cmd_in_ert(content)
    document.body[i:j+1] = ert

lyx2latex(document, lines):
  Here, lines is a list of lines of LyX material we want to convert 
  to LaTeX. We do the best we can and return a string containing
  the translated material.

latex_length(slen):
    Convert lengths (in LyX form) to their LaTeX representation. Returns 
    (bool, length), where the bool tells us if it was a percentage, and 
    the length is the LaTeX representation.

'''

import string
from parser_tools import find_token
from unicode_symbols import unicode_reps


# This will accept either a list of lines or a single line.
# It is bad practice to pass something with embedded newlines,
# though we will handle that.
def add_to_preamble(document, text):
    " Add text to the preamble if it is not already there. "

    if not type(text) is list:
      # split on \n just in case
      # it'll give us the one element list we want
      # if there's no \n, too
      text = text.split('\n')

    i = 0
    prelen = len(document.preamble)
    while True:
      i = find_token(document.preamble, text[0], i)
      if i == -1:
        break
      # we need a perfect match
      matched = True
      for line in text:
        if i >= prelen or line != document.preamble[i]:
          matched = False
          break
        i += 1
      if matched:
        return

    document.preamble.extend(["% Added by lyx2lyx"])
    document.preamble.extend(text)


# Note that text can be either a list of lines or a single line.
# It should really be a list.
def insert_to_preamble(document, text, index = 0):
    """ Insert text to the preamble at a given line"""
    
    if not type(text) is list:
      # split on \n just in case
      # it'll give us the one element list we want
      # if there's no \n, too
      text = text.split('\n')
    
    text.insert(0, "% Added by lyx2lyx")
    document.preamble[index:index] = text


def put_cmd_in_ert(arg):
    '''
    arg should be a list of lines we want to wrap in ERT.
    Returns a list of strings, with the lines so wrapped.
    '''
    
    ret = ["\\begin_inset ERT", "status collapsed", "\\begin_layout Plain Layout", ""]
    # It will be faster for us to work with a single string internally. 
    # That way, we only go through the unicode_reps loop once.
    if type(arg) is list:
      s = "\n".join(arg)
    else:
      s = arg
    for rep in unicode_reps:
      s = s.replace(rep[1], rep[0].replace('\\\\', '\\'))
    s = s.replace('\\', "\\backslash\n")
    ret += s.splitlines()
    ret += ["\\end_layout", "\\end_inset"]
    return ret

            
def lyx2latex(document, lines):
    'Convert some LyX stuff into corresponding LaTeX stuff, as best we can.'

    content = ""
    ert_end = 0
    note_end = 0
    hspace = ""

    for curline in range(len(lines)):
      line = lines[curline]
      if line.startswith("\\begin_inset Note Note"):
          # We want to skip LyX notes, so remember where the inset ends
          note_end = find_end_of_inset(lines, curline + 1)
          continue
      elif note_end >= curline:
          # Skip LyX notes
          continue
      elif line.startswith("\\begin_inset ERT"):
          # We don't want to replace things inside ERT, so figure out
          # where the end of the inset is.
          ert_end = find_end_of_inset(lines, curline + 1)
          continue
      elif line.startswith("\\begin_inset Formula"):
          line = line[20:]
      elif line.startswith("\\begin_inset Quotes"):
          # For now, we do a very basic reversion. Someone who understands
          # quotes is welcome to fix it up.
          qtype = line[20:].strip()
          # lang = qtype[0]
          side = qtype[1]
          dbls = qtype[2]
          if side == "l":
              if dbls == "d":
                  line = "``"
              else:
                  line = "`"
          else:
              if dbls == "d":
                  line = "''"
              else:
                  line = "'"
      elif line.startswith("\\begin_inset space"):
          line = line[18:].strip()
          if line.startswith("\\hspace"):
              # Account for both \hspace and \hspace*
              hspace = line[:-2]
              continue
          elif line == "\\space{}":
              line = "\\ "
          elif line == "\\thinspace{}":
              line = "\\,"
      elif hspace != "":
          # The LyX length is in line[8:], after the \length keyword
          length = latex_length(line[8:])[1]
          line = hspace + "{" + length + "}"
          hspace = ""
      elif line.isspace() or \
            line.startswith("\\begin_layout") or \
            line.startswith("\\end_layout") or \
            line.startswith("\\begin_inset") or \
            line.startswith("\\end_inset") or \
            line.startswith("\\lang") or \
            line.strip() == "status collapsed" or \
            line.strip() == "status open":
          #skip all that stuff
          continue

      # this needs to be added to the preamble because of cases like
      # \textmu, \textbackslash, etc.
      add_to_preamble(document, ['% added by lyx2lyx for converted index entries',
                                 '\\@ifundefined{textmu}',
                                 ' {\\usepackage{textcomp}}{}'])
      # a lossless reversion is not possible
      # try at least to handle some common insets and settings
      if ert_end >= curline:
          line = line.replace(r'\backslash', '\\')
      else:
          # No need to add "{}" after single-nonletter macros
          line = line.replace('&', '\\&')
          line = line.replace('#', '\\#')
          line = line.replace('^', '\\textasciicircum{}')
          line = line.replace('%', '\\%')
          line = line.replace('_', '\\_')
          line = line.replace('$', '\\$')

          # Do the LyX text --> LaTeX conversion
          for rep in unicode_reps:
            line = line.replace(rep[1], rep[0] + "{}")
          line = line.replace(r'\backslash', r'\textbackslash{}')
          line = line.replace(r'\series bold', r'\bfseries{}').replace(r'\series default', r'\mdseries{}')
          line = line.replace(r'\shape italic', r'\itshape{}').replace(r'\shape smallcaps', r'\scshape{}')
          line = line.replace(r'\shape slanted', r'\slshape{}').replace(r'\shape default', r'\upshape{}')
          line = line.replace(r'\emph on', r'\em{}').replace(r'\emph default', r'\em{}')
          line = line.replace(r'\noun on', r'\scshape{}').replace(r'\noun default', r'\upshape{}')
          line = line.replace(r'\bar under', r'\underbar{').replace(r'\bar default', r'}')
          line = line.replace(r'\family sans', r'\sffamily{}').replace(r'\family default', r'\normalfont{}')
          line = line.replace(r'\family typewriter', r'\ttfamily{}').replace(r'\family roman', r'\rmfamily{}')
          line = line.replace(r'\InsetSpace ', r'').replace(r'\SpecialChar ', r'')
      content += line
    return content


def latex_length(slen):
    ''' 
    Convert lengths to their LaTeX representation. Returns (bool, length),
    where the bool tells us if it was a percentage, and the length is the
    LaTeX representation.
    '''
    i = 0
    percent = False
    # the slen has the form
    # ValueUnit+ValueUnit-ValueUnit or
    # ValueUnit+-ValueUnit
    # the + and - (glue lengths) are optional
    # the + always precedes the -

    # Convert relative lengths to LaTeX units
    units = {"text%":"\\textwidth", "col%":"\\columnwidth",
             "page%":"\\paperwidth", "line%":"\\linewidth",
             "theight%":"\\textheight", "pheight%":"\\paperheight"}
    for unit in units.keys():
        i = slen.find(unit)
        if i == -1:
            continue
        percent = True
        minus = slen.rfind("-", 1, i)
        plus = slen.rfind("+", 0, i)
        latex_unit = units[unit]
        if plus == -1 and minus == -1:
            value = slen[:i]
            value = str(float(value)/100)
            end = slen[i + len(unit):]
            slen = value + latex_unit + end
        if plus > minus:
            value = slen[plus + 1:i]
            value = str(float(value)/100)
            begin = slen[:plus + 1]
            end = slen[i+len(unit):]
            slen = begin + value + latex_unit + end
        if plus < minus:
            value = slen[minus + 1:i]
            value = str(float(value)/100)
            begin = slen[:minus + 1]
            slen = begin + value + latex_unit

    # replace + and -, but only if the - is not the first character
    slen = slen[0] + slen[1:].replace("+", " plus ").replace("-", " minus ")
    # handle the case where "+-1mm" was used, because LaTeX only understands
    # "plus 1mm minus 1mm"
    if slen.find("plus  minus"):
        lastvaluepos = slen.rfind(" ")
        lastvalue = slen[lastvaluepos:]
        slen = slen.replace("  ", lastvalue + " ")
    return (percent, slen)


def revert_flex_inset(lines, name, LaTeXname):
  " Convert flex insets to TeX code "
  i = 0
  while True:
    i = find_token(lines, '\\begin_inset Flex ' + name, i)
    if i == -1:
      return
    z = find_end_of_inset(lines, i)
    if z == -1:
      document.warning("Can't find end of Flex " + name + " inset.")
      i += 1
      continue
    # remove the \end_inset
    lines[z - 2:z + 1] = put_cmd_in_ert("}")
    # we need to reset character layouts if necessary
    j = find_token(lines, '\\emph on', i, z)
    k = find_token(lines, '\\noun on', i, z)
    l = find_token(lines, '\\series', i, z)
    m = find_token(lines, '\\family', i, z)
    n = find_token(lines, '\\shape', i, z)
    o = find_token(lines, '\\color', i, z)
    p = find_token(lines, '\\size', i, z)
    q = find_token(lines, '\\bar under', i, z)
    r = find_token(lines, '\\uuline on', i, z)
    s = find_token(lines, '\\uwave on', i, z)
    t = find_token(lines, '\\strikeout on', i, z)
    if j != -1:
      lines.insert(z - 2, "\\emph default")
    if k != -1:
      lines.insert(z - 2, "\\noun default")
    if l != -1:
      lines.insert(z - 2, "\\series default")
    if m != -1:
      lines.insert(z - 2, "\\family default")
    if n != -1:
      lines.insert(z - 2, "\\shape default")
    if o != -1:
      lines.insert(z - 2, "\\color inherit")
    if p != -1:
      lines.insert(z - 2, "\\size default")
    if q != -1:
      lines.insert(z - 2, "\\bar default")
    if r != -1:
      lines.insert(z - 2, "\\uuline default")
    if s != -1:
      lines.insert(z - 2, "\\uwave default")
    if t != -1:
      lines.insert(z - 2, "\\strikeout default")
    lines[i:i + 4] = put_cmd_in_ert(LaTeXname + "{")
    i += 1


def revert_font_attrs(lines, name, LaTeXname):
  " Reverts font changes to TeX code "
  i = 0
  changed = False
  while True:
    i = find_token(lines, name + ' on', i)
    if i == -1:
      return changed
    j = find_token(lines, name + ' default', i)
    k = find_token(lines, name + ' on', i + 1)
    # if there is no default set, the style ends with the layout
    # assure hereby that we found the correct layout end
    if j != -1 and (j < k or k == -1):
      lines[j:j + 1] = put_cmd_in_ert("}")
    else:
      j = find_token(lines, '\\end_layout', i)
      lines[j:j] = put_cmd_in_ert("}")
    lines[i:i + 1] = put_cmd_in_ert(LaTeXname + "{")
    changed = True
    i += 1


def revert_layout_command(lines, name, LaTeXname):
  " Reverts a command from a layout to TeX code "
  i = 0
  while True:
    i = find_token(lines, '\\begin_layout ' + name, i)
    if i == -1:
      return
    k = -1
    # find the next layout
    j = i + 1
    while k == -1:
      j = find_token(lines, '\\begin_layout', j)
      l = len(lines)
      # if nothing was found it was the last layout of the document
      if j == -1:
        lines[l - 4:l - 4] = put_cmd_in_ert("}")
        k = 0
      # exclude plain layout because this can be TeX code or another inset
      elif lines[j] != '\\begin_layout Plain Layout':
        lines[j - 2:j - 2] = put_cmd_in_ert("}")
        k = 0
      else:
        j += 1
    lines[i] = '\\begin_layout Standard'
    lines[i + 1:i + 1] = put_cmd_in_ert(LaTeXname + "{")
    i += 1


def hex2ratio(s):
  " Converts an RRGGBB-type hexadecimal string to a float in [0.0,1.0] "
  try:
    val = int(s, 16)
  except:
    val = 0
  if val != 0:
    val += 1
  return str(val / 256.0)


def str2bool(s):
  "'true' goes to True, case-insensitively, and we strip whitespace."
  s = s.strip().lower()
  return s == "true"
