# This file is part of lyx2lyx
# Copyright (C) 2003 JosÅÈ Matos <jamatos@fep.up.pt>
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

def rm_end_header(lines):
    i = find_token(lines, "\\end_header", 0)
    if i == -1:
        return
    del lines[i]

def convert_spaces(lines):
    for i in range(len(lines)):
        lines[i] = string.replace(lines[i],"\\InsetSpace ~", "\\SpecialChar ~")

def convert_bibtex(lines):
    for i in range(len(lines)):
        lines[i] = string.replace(lines[i], "\\begin_inset LatexCommand \\bibtex",
                                  "\\begin_inset LatexCommand \\BibTeX")

def rm_tracking_changes(lines):
    i = find_token(lines, "\\tracking_changes", 0)
    if i == -1:
        return
    del lines[i]

#FIXME
def rm_body_changes(lines):
    pass

def convert(header, body):
    rm_end_header(header)
    convert_spaces(body)
    convert_bibtex(body)
    rm_tracking_changes(header)
    rm_body_changes(body)

if __name__ == "__main__":
    pass
