# This file is part of lyx2lyx
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

def add_end(header):
    header.append("\\end_header");

def convert_spaces(lines):
    for i in range(len(lines)):
        lines[i] = string.replace(lines[i],"\\SpecialChar ~","\\InsetSpace ~")

def convert_bibtex(lines):
    for i in range(len(lines)):
        lines[i] = string.replace(lines[i],"\\begin_inset LatexCommand \\BibTeX",
                                  "\\begin_inset LatexCommand \\bibtex")

def convert(header, body):
    add_end(header)
    convert_spaces(body)
    convert_bibtex(body)

if __name__ == "__main__":
    pass
