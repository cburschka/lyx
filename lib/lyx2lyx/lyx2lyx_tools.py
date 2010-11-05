# This file is part of lyx2lyx
# -*- coding: utf-8 -*-
# Copyright (C) 2010 The LyX team
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

" This modules offer several free functions to help with lyx2lyx'ing. "

import string
from parser_tools import find_token
from unicode_symbols import unicode_reps

# This will accept either a list of lines or a single line.
# But it is bad practice to pass something with embedded
# newlines, though we will handle that.
def add_to_preamble(document, text):
    """ Add text to the preamble if it is not already there.
    Only the first line is checked!"""

    if not type(text) is list:
      # split on \n just in case
      # it'll give us the one element list we want
      # if there's no \n, too
      text = text.split('\n')

    if find_token(document.preamble, text[0], 0) != -1:
        return

    document.preamble.extend(text)


# Note that text can be either a list of lines or a single line.
# It should really be a list.
def insert_to_preamble(index, document, text):
    """ Insert text to the preamble at a given line"""
    
    if not type(text) is list:
      # split on \n just in case
      # it'll give us the one element list we want
      # if there's no \n, too
      text = text.split('\n')

    document.preamble[index:index] = text


# This routine wraps some content in an ERT inset. 
#
# NOTE: The function accepts either a single string or a LIST of strings as
# argument. But it returns a LIST of strings, split on \n, so that it does 
# not have embedded newlines.
# 
# This is how lyx2lyx represents a LyX document: as a list of strings, 
# each representing a line of a LyX file. Embedded newlines confuse 
# lyx2lyx very much.
#
# A call to this routine will often go something like this:
#   i = find_token('\\begin_inset FunkyInset', ...)
#   ...
#   j = find_end_of_inset(document.body, i)
#   content = ...extract content from insets
#   # that could be as simple as: 
#   # content = lyx2latex(document[i:j + 1])
#   ert = put_cmd_in_ert(content)
#   document.body[i:j] = ert
# Now, before we continue, we need to reset i appropriately. Normally,
# this would be: 
#   i += len(ert)
# That puts us right after the ERT we just inserted.
#
def put_cmd_in_ert(arg):
    ret = ["\\begin_inset ERT", "status collapsed", "\\begin_layout Plain Layout", ""]
    # Despite the warnings just given, it will be faster for us to work
    # with a single string internally. That way, we only go through the
    # unicode_reps loop once.
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
    # clean up multiline stuff
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


def revert_flex_inset(document, name, LaTeXname, position):
  " Convert flex insets to TeX code "
  i = position
  while True:
    i = find_token(document.body, '\\begin_inset Flex ' + name, i)
    if i == -1:
      return
    z = find_end_of_inset(document.body, i)
    if z == -1:
      document.warning("Malformed LyX document: Can't find end of Flex " + name + " inset.")
      return
    # remove the \end_inset
    document.body[z - 2:z + 1] = put_cmd_in_ert("}")
    # we need to reset character layouts if necessary
    j = find_token(document.body, '\\emph on', i, z)
    k = find_token(document.body, '\\noun on', i, z)
    l = find_token(document.body, '\\series', i, z)
    m = find_token(document.body, '\\family', i, z)
    n = find_token(document.body, '\\shape', i, z)
    o = find_token(document.body, '\\color', i, z)
    p = find_token(document.body, '\\size', i, z)
    q = find_token(document.body, '\\bar under', i, z)
    r = find_token(document.body, '\\uuline on', i, z)
    s = find_token(document.body, '\\uwave on', i, z)
    t = find_token(document.body, '\\strikeout on', i, z)
    if j != -1:
      document.body.insert(z - 2, "\\emph default")
    if k != -1:
      document.body.insert(z - 2, "\\noun default")
    if l != -1:
      document.body.insert(z - 2, "\\series default")
    if m != -1:
      document.body.insert(z - 2, "\\family default")
    if n != -1:
      document.body.insert(z - 2, "\\shape default")
    if o != -1:
      document.body.insert(z - 2, "\\color inherit")
    if p != -1:
      document.body.insert(z - 2, "\\size default")
    if q != -1:
      document.body.insert(z - 2, "\\bar default")
    if r != -1:
      document.body.insert(z - 2, "\\uuline default")
    if s != -1:
      document.body.insert(z - 2, "\\uwave default")
    if t != -1:
      document.body.insert(z - 2, "\\strikeout default")
    document.body[i:i + 4] = put_cmd_in_ert(LaTeXname + "{")
    i += 1


def revert_font_attrs(document, name, LaTeXname):
  " Reverts font changes to TeX code "
  i = 0
  changed = False
  while True:
    i = find_token(document.body, name + ' on', i)
    if i == -1:
      return changed
    j = find_token(document.body, name + ' default', i)
    k = find_token(document.body, name + ' on', i + 1)
    # if there is no default set, the style ends with the layout
    # assure hereby that we found the correct layout end
    if j != -1 and (j < k or k == -1):
      document.body[j:j + 1] = put_cmd_in_ert("}")
    else:
      j = find_token(document.body, '\\end_layout', i)
      document.body[j:j] = put_cmd_in_ert("}")
    document.body[i:i + 1] = put_cmd_in_ert(LaTeXname + "{")
    changed = True
    i += 1


def revert_layout_command(document, name, LaTeXname, position):
  " Reverts a command from a layout to TeX code "
  i = position
  while True:
    i = find_token(document.body, '\\begin_layout ' + name, i)
    if i == -1:
      return
    k = -1
    # find the next layout
    j = i + 1
    while k == -1:
      j = find_token(document.body, '\\begin_layout', j)
      l = len(document.body)
      # if nothing was found it was the last layout of the document
      if j == -1:
        document.body[l - 4:l - 4] = put_cmd_in_ert("}")
        k = 0
      # exclude plain layout because this can be TeX code or another inset
      elif document.body[j] != '\\begin_layout Plain Layout':
        document.body[j - 2:j - 2] = put_cmd_in_ert("}")
        k = 0
      else:
        j += 1
    document.body[i] = '\\begin_layout Standard'
    document.body[i + 1:i + 1] = put_cmd_in_ert(LaTeXname + "{")
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
