# This file is part of lyx2lyx
# -*- coding: iso-8859-1 -*-
# Copyright (C) 2003-2004 José Matos <jamatos@lyx.org>
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
import string
from parser_tools import find_token, find_re, check_token


def space_before_layout(file):
    lines = file.body
    i = 2 # skip first layout
    while 1:
        i = find_token(lines, '\\layout', i)
        if i == -1:
            break

        if lines[i - 1] == '' and string.find(lines[i-2],'\\protected_separator') == -1:
            del lines[i-1]
        i = i + 1


def formula_inset_space_eat(file):
    lines = file.body
    i=0
    while 1:
        i = find_token(lines, "\\begin_inset Formula", i)
        if i == -1: break

        if len(lines[i]) > 22 and lines[i][21] == ' ':
            lines[i] = lines[i][:20] + lines[i][21:]
        i = i + 1


# Update from tabular format 2 to 4
def update_tabular(file):
    lines = file.body
    lyxtable_re = re.compile(r".*\\LyXTable$")
    i=0
    while 1:
        i = find_re(lines, lyxtable_re, i)
        if i == -1:
            break
        i = i + 1
        format = lines[i][8]

        lines[i]='multicol4'
        i = i + 1
        rows = int(string.split(lines[i])[0])
        columns = int(string.split(lines[i])[1])

        lines[i] = lines[i] + ' 0 0 -1 -1 -1 -1'
        i = i + 1

        for j in range(rows):
            lines[i] = lines[i] + ' 0 0'
            i = i + 1

        for j in range(columns):
            lines[i] = lines[i] + ' '
            i = i + 1

        while lines[i]:
            lines[i] = lines[i] + ' 0 0 0'
            i = i + 1


def final_dot(file):
    lines = file.body
    i = 0
    while i < len(lines):
        if lines[i][-1:] == '.' and lines[i+1][:1] != '\\' and  lines[i+1][:1] != ' ' and len(lines[i]) + len(lines[i+1])<= 72 and lines[i+1] != '':
            lines[i] = lines[i] + lines[i+1]
            del lines[i+1]
        else:
            i = i + 1


def update_inset_label(file):
    lines = file.body
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset Label', i)
        if i == -1:
            return
        lines[i] = '\\begin_inset LatexCommand \label{' + lines[i][19:] + '}'
        i = i + 1


def update_latexdel(file):
    lines = file.body
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset LatexDel', i)
        if i == -1:
            return
        lines[i] = string.replace(lines[i],'\\begin_inset LatexDel', '\\begin_inset LatexCommand')
        i = i + 1


def update_vfill(file):
    lines = file.body
    for i in range(len(lines)):
        lines[i] = string.replace(lines[i],'\\fill_top','\\added_space_top vfill')
        lines[i] = string.replace(lines[i],'\\fill_bottom','\\added_space_bottom vfill')


def update_space_units(file):
    lines = file.body
    added_space_bottom = re.compile(r'\\added_space_bottom ([^ ]*)')
    added_space_top    = re.compile(r'\\added_space_top ([^ ]*)')
    for i in range(len(lines)):
        result = added_space_bottom.search(lines[i])
        if result:
            old = '\\added_space_bottom ' + result.group(1)
            new = '\\added_space_bottom ' + str(float(result.group(1))) + 'cm'
            lines[i] = string.replace(lines[i], old, new)

        result = added_space_top.search(lines[i])
        if result:
            old = '\\added_space_top ' + result.group(1)
            new = '\\added_space_top ' + str(float(result.group(1))) + 'cm'
            lines[i] = string.replace(lines[i], old, new)


def update_inset_accent(file):
    lines = file.body
    pass


def remove_cursor(file):
    lines = file.body
    i = 0
    cursor_re = re.compile(r'.*(\\cursor \d*)')
    while 1:
        i = find_re(lines, cursor_re, i)
        if i == -1:
            break
        cursor = cursor_re.search(lines[i]).group(1)
        lines[i]= string.replace(lines[i], cursor, '')
        i = i + 1


def remove_empty_insets(file):
    lines = file.body
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset ',i)
        if i == -1:
            break
        if lines[i] == '\\begin_inset ' and lines[i+1] == '\\end_inset ':
            del lines[i]
            del lines[i]
        i = i + 1


def remove_formula_latex(file):
    lines = file.body
    i = 0
    while 1:
        i = find_token(lines, '\\latex formula_latex ', i)
        if i == -1:
            break
        del lines[i]

        i = find_token(lines, '\\latex default', i)
        if i == -1:
            break
        del lines[i]


def add_end_document(file):
    lines = file.body
    i = find_token(lines, '\\the_end', 0)
    if i == -1:
        lines.append('\\the_end')


def header_update(file):
    lines = file.header
    i = 0
    l = len(lines)
    while i < l:
        if check_token(lines[i], '\\begin_preamble'):
            i = find_token(lines, '\\end_preamble', i)
            if i == -1:
                file.error('Unfinished preamble')
            i = i + 1
            continue

        if lines[i][-1:] == ' ':
            lines[i] = lines[i][:-1]

        if check_token(lines[i], '\\epsfig'):
            lines[i] = string.replace(lines[i], '\\epsfig', '\\graphics')
            i = i + 1
            continue

        if check_token(lines[i], '\\papersize'):
            size = string.split(lines[i])[1]
            new_size = size
            paperpackage = ""

            if size == 'usletter':
                new_size = 'letterpaper'
            if size == 'a4wide':
                new_size = 'Default'
                paperpackage = "widemarginsa4"

            lines[i] = '\\papersize ' + new_size
            i = i + 1
            if paperpackage:
                lines.insert(i, '\\paperpackage ' + paperpackage)
                i = i + 1

            lines.insert(i,'\\use_geometry 0')
            lines.insert(i + 1,'\\use_amsmath 0')
            i = i + 2
            continue


        if check_token(lines[i], '\\baselinestretch'):
            size = string.split(lines[i])[1]
            if size == '1.00':
                name = 'single'
            elif size == '1.50':
                name = 'onehalf'
            elif size == '2.00':
                name = 'double'
            else:
                name = 'other ' + size
            lines[i] = '\\spacing %s ' % name
            i = i + 1
            continue

        i = i + 1


def update_latexaccents(file):
    body = file.body
    i = 1
    while 1:
        i = find_token(body, '\\i ', i)
        if i == -1:
            return

        contents = string.strip(body[i][2:])

        if string.find(contents, '{') != -1 and string.find(contents, '}') != -1:
            i = i + 1
            continue

        if len(contents) == 2:
            contents = contents + '{}'
        elif len(contents) == 3:
            contents = contents[:2] + '{' + contents[2] + '}'
        elif len(contents) == 4:
            if contents[2] == ' ':
                contents = contents[:2] + '{' + contents[3] + '}'
            elif contents[2:4] == '\\i' or contents[2:4] == '\\j':
                contents = contents[:2] + '{' + contents[2:] + '}'

        body[i] = '\\i ' + contents
        i = i + 1


def convert(file):
    table = [header_update, add_end_document, remove_cursor,
             final_dot, update_inset_label, update_latexdel,
             update_space_units, update_inset_accent,
             space_before_layout, formula_inset_space_eat,
             update_tabular, update_vfill, remove_empty_insets,
             remove_formula_latex, update_latexaccents]

    for conv in table:
        conv(file)

    file.format = 215


def revert(file):
    file.error("The convertion to an older format (%s) is not implemented." % file.format)


if __name__ == "__main__":
    pass
