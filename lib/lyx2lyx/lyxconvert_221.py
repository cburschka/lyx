# This file is part of lyx2lyx
# -*- coding: iso-8859-1 -*-
# Copyright (C) 2002 Dekel Tsur <dekel@lyx.org>
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
from parser_tools import find_token

def add_end(header):
    header.append("\\end_header");

def convert_spaces(lines):
    for i in range(len(lines)):
        lines[i] = string.replace(lines[i],"\\SpecialChar ~","\\InsetSpace ~")

def convert_bibtex(lines):
    for i in range(len(lines)):
        lines[i] = string.replace(lines[i],"\\begin_inset LatexCommand \\BibTeX",
                                  "\\begin_inset LatexCommand \\bibtex")

def remove_insetparent(lines):
    i = 0
    while 1:
        i = find_token(lines, "\\begin_inset LatexCommand \\lyxparent", i)
        if i == -1:
            break
        del lines[i:i+3]

def convert(header, body):
    add_end(header)
    convert_spaces(body)
    convert_bibtex(body)
    remove_insetparent(body)

if __name__ == "__main__":
    pass
