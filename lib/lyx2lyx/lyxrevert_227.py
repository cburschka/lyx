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

from parser_tools import find_token, find_end_of_inset

def convert_box(lines):
    box_header = "\\begin_inset Box "
    i = 0
    while 1:
        i = find_token(lines, box_header, i)
        if i == -1:
            break

        lines[i] = "\\begin_inset " + lines[i][len(box_header):]
        i = i + 1

def convert_external(lines):
    draft_token = '\tdraft'
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset External', i)
        if i == -1:
            break
        j = find_end_of_inset(lines, i + 1)
        if j == -1:
            #this should not happen
            break
        k = find_token(lines, draft_token, i+1, j-1)
        if (k != -1 and len(draft_token) == len(lines[k])):
            del lines[k]
        i = j + 1

def convert(header, body):
    convert_box(body)
    convert_external(body)

if __name__ == "__main__":
    pass
