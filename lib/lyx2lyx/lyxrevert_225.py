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

import re
from parser_tools import find_token, find_end_of_inset
from string import replace

def rm_end_layout(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\end_layout', i)

        if i == -1:
            return

        del lines[i]

def begin_layout2layout(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\begin_layout', i)
        if i == -1:
            return

        lines[i] = replace(lines[i], '\\begin_layout', '\\layout')
        i = i + 1

def table_valignment_middle(lines, start, end):
    for i in range(start, end):
        if re.search('^<(column|cell) .*valignment="middle".*>$', lines[i]):
            lines[i] = replace(lines[i], 'valignment="middle"', 'valignment="center"')

def valignment_middle(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset  Tabular', i)
        if i == -1:
            return
        j = find_end_of_inset(lines, i + 1)
        if j == -1:
            #this should not happen
            valignment_middle(lines, i + 1, len(lines))
            return
        valignment_middle(lines, i + 1, j)
        i = j + 1

def end_document(lines):
    i = find_token(lines, "\\end_document", 0)
    if i == -1:
        lines.append("\\the_end")
        return
    lines[i] = "\\the_end"

def convert(header, body):
    rm_end_layout(body)
    begin_layout2layout(body)
    end_document(body)
    valignment_middle(body)

if __name__ == "__main__":
    pass
