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
from string import replace, split, find, replace, strip, join

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

##
# Convert line and page breaks
# Old:
#\layout Standard
#\line_top \line_bottom \pagebreak_top \pagebreak_bottom \added_space_top xxx \added_space_bottom yyy
#0
#
# New:
#\begin layout Standard
#
#\newpage 
#
#\lyxline
#\begin_inset VSpace xxx
#\end_inset
#
#\end_layout
#\begin_layout Standard
#
#0
#\end_layout
#\begin_layout Standard
#
#\begin_inset VSpace xxx
#\end_inset
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
        line_top   = find(lines[i],"\\line_top")
        line_bot   = find(lines[i],"\\line_bottom")
        pb_top     = find(lines[i],"\\pagebreak_top")
        pb_bot     = find(lines[i],"\\pagebreak_bottom")
        vspace_top = find(lines[i],"\\added_space_top")
        vspace_bot = find(lines[i],"\\added_space_bottom")

        if line_top == -1 and line_bot == -1 and pb_bot == -1 and pb_top == -1 and vspace_top == -1 and vspace_bot == -1:
            continue

        for tag in "\\line_top", "\\line_bottom", "\\pagebreak_top", "\\pagebreak_bottom":
            lines[i] = replace(lines[i], tag, "")

        if vspace_top != -1:
            # the position could be change because of the removal of other
            # paragraph properties above
            vspace_top = find(lines[i],"\\added_space_top")
            tmp_list = split(lines[i][vspace_top:])
            vspace_top_value = tmp_list[1]
            lines[i] = lines[i][:vspace_top] + join(tmp_list[2:])

        if vspace_bot != -1:
            # the position could be change because of the removal of other
            # paragraph properties above
            vspace_bot = find(lines[i],"\\added_space_bottom")
            tmp_list = split(lines[i][vspace_bot:])
            vspace_bot_value = tmp_list[1]
            lines[i] = lines[i][:vspace_bot] + join(tmp_list[2:])

        lines[i] = strip(lines[i])
        i = i + 1

        #  Create an empty paragraph for line and page break that belong
        # above the paragraph
        if pb_top !=-1 or line_top != -1 or vspace_bot != -1:
            
            paragraph_above = ['','\\begin_layout Standard','','']

            if pb_top != -1:
                paragraph_above.extend(['\\newpage ',''])

            if vspace_top != -1:
                paragraph_above.extend(['\\begin_inset VSpace ' + vspace_top_value,'\\end_inset ','',''])

            if line_top != -1:
                paragraph_above.extend(['\\lyxline ',''])

            paragraph_above.extend(['\\end_layout',''])

            #inset new paragraph above the current paragraph
            lines[i-2:i-2] = paragraph_above
            i = i + len(paragraph_above)

        # Ensure that nested style are converted later.
        k = find_end_of(lines, i, "\\begin_layout", "\\end_layout")

        if k == -1:
            return

        if pb_top !=-1 or line_top != -1 or vspace_bot != -1:
            
            paragraph_bellow = ['','\\begin_layout Standard','','']

            if line_bot != -1:
                paragraph_bellow.extend(['\\lyxline ',''])

            if vspace_bot != -1:
                paragraph_bellow.extend(['\\begin_inset VSpace ' + vspace_bot_value,'\\end_inset ','',''])

            if pb_bot != -1:
                paragraph_bellow.extend(['\\newpage ',''])

            paragraph_bellow.extend(['\\end_layout',''])

            #inset new paragraph above the current paragraph
            lines[k + 1: k + 1] = paragraph_bellow

def convert(header, body):
    add_end_layout(body)
    layout2begin_layout(body)
    end_document(body)
    table_valignment_middle(body)
    convert_breaks(body)

if __name__ == "__main__":
    pass
