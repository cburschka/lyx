# This file is part of lyx2lyx
# Copyright (C) 2003 José Matos <jamatos@fep.up.pt>
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
from parser_tools import find_token, find_tokens, find_end_of_inset, find_end_of
from sys import stderr
from string import replace, split, find

def add_end_layout(lines):
    i = find_token(lines, '\\layout', 0)

    if i == -1:
        return

    i = i + 1
    struct_stack = ["\\layout"]
    while 1:
        i = find_tokens(lines, ["\\begin_inset", "\\end_inset", "\\layout",
                                "\\begin_deeper", "\\end_deeper", "\\the_end"], i)

        token = split(lines[i])[0]

        if token == "\\begin_inset":
            struct_stack.append(token)
            i = i + 1
            continue

        if token == "\\end_inset":
            tail = struct_stack.pop()
            if tail == "\\layout":
                lines.insert(i,"\\end_layout")
                i = i + 1
                #Check if it is the correct tag
                struct_stack.pop()
            i = i + 1
            continue

        if token == "\\layout":
            tail = struct_stack.pop()
            if tail == token:
                lines.insert(i,"\\end_layout")
                i = i + 2
            else:
                struct_stack.append(tail)
                i = i + 1
            struct_stack.append(token)
            continue

        if token == "\\begin_deeper" or token == "\\end_deeper":
            lines.insert(i,"\\end_layout")
            i = i + 2
            continue

        #case \end_document
        lines.insert(i, "\\end_layout")
        return

def layout2begin_layout(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\layout', i)
        if i == -1:
            return

        lines[i] = replace(lines[i], '\\layout', '\\begin_layout')
        i = i + 1

def valignment_middle(lines, start, end):
    for i in range(start, end):
        if re.search('^<(column|cell) .*valignment="center".*>$', lines[i]):
            lines[i] = replace(lines[i], 'valignment="center"', 'valignment="middle"')

def table_valignment_middle(lines):
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
    i = find_token(lines, "\\the_end", 0)
    if i == -1:
        lines.append("\\end_document")
        return
    lines[i] = "\\end_document"


def convert_minipage(lines):
    """ Convert minipages to the box inset.
    We try to use the same order of arguments as lyx does.
    """
    pos = ["t","c","b"]
    inner_pos = ["c","t","b","s"]

    i = 0
    while 1:
        i = find_token(lines, "\\begin_inset Minipage", i)
        if i == -1:
            return

        lines[i] = "\\begin_inset Frameless"
        i = i + 1

        # convert old to new position using the pos list
        if lines[i][:8] == "position":
            lines[i] = 'position "%s"' % pos[int(lines[i][9])]
        else:
            lines.insert(i, 'position "%s"' % pos[0])
        i = i + 1

        lines.insert(i, 'hor_pos "c"')
        i = i + 1
        lines.insert(i, 'has_inner_box 1')
        i = i + 1

        # convert the inner_position
        if lines[i][:14] == "inner_position":
            lines[i] = 'inner_pos "%s"' %  inner_pos[int(lines[i][15])]
        else:
            lines.insert('inner_pos "%s"' % inner_pos[0])
        i = i + 1

        # We need this since the new file format has a height and width
        # in a different order.
        if lines[i][:6] == "height":
            height = lines[i][6:]
            del lines[i]
        else:
            height = ' "0"'

        if lines[i][:5] == "width":
            width = lines[i][5:]
            del lines[i]
        else:
            width = ' "0"'

        lines.insert(i, 'use_parbox 0')
        i = i + 1
        lines.insert(i, 'width' + width)
        i = i + 1
        lines.insert(i, 'special "none"')
        i = i + 1
        lines.insert(i, 'height' + height)
        i = i + 1
        lines.insert(i, 'height_special "totalheight"')
        i = i + 1

##
# Convert line and page breaks
# Old:
#\layout Standard
#\line_top \line_bottom \pagebreak_top \pagebreak_bottom 
#0
#
# New:
#\begin_layout Standard
#\newpage 
#
#\lyxline 
#0
#\lyxline 
#
#\newpage 
#
#\end_layout

def convert_breaks(lines):    
    i = 0
    while 1:
        i = find_token(lines, "\\begin_layout", i)
        if i == -1:
            return
        i = i + 1
        line_top = find(lines[i],"\\line_top")
        line_bot = find(lines[i],"\\line_bottom")
        pb_top = find(lines[i],"\\pagebreak_top")
        pb_bot = find(lines[i],"\\pagebreak_bottom")

        if line_top == -1 and line_bot == -1 and pb_bot == -1 and pb_top == -1:
            continue

        lines[i] = ""
        i = i + 1

        #  Create an empty paragraph for line and page break that belong
        # above the paragraph
        #  To simplify the code, and maintain the same insertion point,
        # I inserted by reverse order. It looks funny. :-)
        if pb_top !=-1 or line_top != -1:
            k = i - 3
            lines.insert(k, '')
            lines.insert(k, '\\end_layout')

            if line_top != -1:
                lines.insert(k, '')
                lines.insert(k, "\\lyxline ")
                i = i + 2

            if pb_top != -1:
                lines.insert(k, '')
                lines.insert(k, "\\newpage ")
                i = i + 2

            lines.insert(k, '')
            lines.insert(k, '')
            lines.insert(k, '\\begin_layout Standard')
            i = i + 5

        # Ensure that nested style are converted later.
        k = find_end_of(lines, i, "\\begin_layout", "\\end_layout")

        if k == -1:
            return

        if line_bot != -1:
            lines.insert(k, "\\lyxline ")
            k = k + 1

        if pb_bot != -1:
            lines.insert(k, "\\newpage ")
            k = k + 1

def convert(header, body):
    add_end_layout(body)
    layout2begin_layout(body)
    end_document(body)
    table_valignment_middle(body)
    convert_minipage(body)
    convert_breaks(body)

if __name__ == "__main__":
    pass
