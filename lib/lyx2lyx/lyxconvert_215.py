# This file is part of lyx2lyx
# Copyright (C) 2002 José Matos <jamatos@lyx.org>
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

import re
from parser_tools import *

layout_exp = re.compile(r"\\layout (\S*)")

math_env = ["\\[","\\begin{eqnarray*}","\\begin{eqnarray}","\\begin{equation}"]

def replace_protected_separator(lines):
    i=0
    while 1:
        i = find_token(lines, "\\protected_separator", i)
        if i == -1:
            break
        j = find_token_backwards(lines, "\\layout", i)
        #if j == -1: print error
        layout = layout_exp.match(lines[j]).group(1)

        if layout == "LyX-Code":
            result = ""
            while lines[i] == "\\protected_separator ":
                result = result + " "
                del lines[i]

            lines[i-1] = lines[i-1] + result + lines[i]
        else:
            lines[i-1] = lines[i-1]+ "\\SpecialChar ~"

        del lines[i]

def merge_formula_inset(lines):
    i=0
    while 1:
        i = find_token(lines, "\\begin_inset Formula", i)
        if i == -1: break
        if lines[i+1] in math_env:
            lines[i] = lines[i] + lines[i+1]
            del lines[i+1]
        i = i + 1

def convert(header,body):
    replace_protected_separator(body)
    merge_formula_inset(body)

if __name__ == "__main__":
    pass

