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
from parser_tools import find_token, find_tokens, find_end_of_inset
from sys import stderr
from string import replace, split, strip
import re

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

def convert_bibtex(lines):
    bibtex_header = "\\begin_inset LatexCommand \\bibtex"
    i = 0
    while 1:
        i = find_token(lines, bibtex_header, i)
        if i == -1:
            break
        # We've found a bibtex inset.
        # I'd like to strip bibtex_header from the front of lines[i]
        lines[i] = replace(lines[i], bibtex_header, "")

        # Trim any space at extremes
        lines[i] = strip(lines[i])

        # Does the thing have an opt arg?
        optarg_rexp = re.compile(r'^\[([^]]*)\]')
        optarg = optarg_rexp.search(lines[i])
        optarg_contents = ''
        if optarg:
                optarg_contents = optarg.group(1)
                # strip [<optarg_contents>] from the front of lines[i]
                lines[i] = replace (lines[i], '[' + optarg.group(0) + ']', '')

        # lines[i] should now contain "{<list of databases>}"
        mainarg_rexp = re.compile(r'{([^}]*)}')
        mainarg = mainarg_rexp.search(lines[i])
        mainarg_contents = ''
        if mainarg:
                mainarg_contents = mainarg.group(1)
        else:
                # complain about a mal-formed lyx file.
                stderr.write("Mal-formed bibitem\n")

        # optarg will contain either
        #       "bibtotoc,<style>"
        # or
        #       "<style>"
        # ie, these are a comma-separated list of arguments.
        optarg_list = split(optarg_contents, ',')
        if len(optarg_list) == 0:
            bibtotoc, style = '',''
        elif len(optarg_list) == 1:
            bibtotoc, style = '',optarg_list[0]
        else:
            bibtotoc, style = 'true',optarg_list[1]
        
        # mainarg will contain a comma-separated list of files.
        mainarg_list = split( mainarg_contents, ',')

        new_syntax = ['\\begin_inset Bibtex']
        for file in mainarg_list:
            new_syntax.append('\t' + 'filename ' + file)

        if style:
            new_syntax.append('\t' + 'style ' + style)

        if bibtotoc == 'true':
            new_syntax.append('\t' + 'bibtotoc ' + bibtotoc)

        # Replace old syntax with new
        lines[i:i+1] = new_syntax

        i = i + len(new_syntax) + 1

        
def convert(header, body):
    add_end_layout(body)
    layout2begin_layout(body)
    end_document(body)
    table_valignment_middle(body)
    convert_bibtex(body)

if __name__ == "__main__":
    pass
