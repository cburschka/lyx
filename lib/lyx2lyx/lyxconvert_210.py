# This file is part of lyx2lyx
# -*- coding: iso-8859-1 -*-
# Copyright (C) 2003 José Matos <jamatos@lyx.org>
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
from parser_tools import *

def space_before_layout(lines):
    i = 2 # skip first layout
    while 1:
        i = find_token(lines, '\\layout', i)
        if i == -1:
            break

        if lines[i - 1] == '':
            del lines[i-1]
        i = i + 1

def formula_inset_space_eat(lines):
    i=0
    while 1:
        i = find_token(lines, "\\begin_inset Formula", i)
        if i == -1: break

        if len(lines[i]) > 22:
            lines[i] = lines[i][:20] + lines[i][21:]
        i = i + 1

# Update from tabular format 2 to 4
def update_tabular(lines):
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

def final_dot(lines):
    i = 0
    while i < len(lines):
        if lines[i][-1:] == '.' and lines[i+1][:1] != '\\' and  lines[i+1][:1] != ' ' and len(lines[i]) + len(lines[i+1])<= 72 and lines[i+1] != '':
            lines[i] = lines[i] + lines[i+1]
            del lines[i+1]
        else:
            i = i + 1

def update_inset_label(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset Label', i)
        if i == -1:
            return
        lines[i] = '\\begin_inset LatexCommand \label{' + lines[i][19:] + '}'
        i = i + 1

def update_latexdel(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset LatexDel', i)
        if i == -1:
            return
        lines[i] = string.replace(lines[i],'\\begin_inset LatexDel', '\\begin_inset LatexCommand')
        i = i + 1

def update_vfill(lines):
    for i in range(len(lines)):
        lines[i] = string.replace(lines[i],'\\fill_top','\\added_space_top vfill')
        lines[i] = string.replace(lines[i],'\\fill_bottom','\\added_space_bottom vfill')

def update_space_units(lines):
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

def update_inset_accent(lines):
    pass

def remove_cursor(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\cursor', i)
        if i == -1:
            break
        lines[i] = ''
        i = i + 1

def add_end_document(lines):
    lines.append('\\the_end')

def header_update(lines):
    i = 0
    l = len(lines)
    while i < l:
        if check_token(lines[i], '\\begin_preamble'):
            i = find_token(lines, '\\end_preamble', i)
            if i == -1:
                sys.stderr.write('Unfinished preamble')
                sys.exit(1)
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
            if size == 'usletter':
                lines[i] = '\\papersize letterpaper'
            i = i + 1
            continue

        if check_token(lines[i], '\\baselinestretch'):
            size = string.split(lines[i])[1]
            if size == '1.00':
                name = 'single'
            elif size == '1.50':
                name = 'onehalf'
            elif size == '2.00':
                name == 'double'
            lines[i] = '\\spacing %s ' % name
            i = i + 1
            continue

        i = i + 1

    lines.append('\\paperpackage a4')
    lines.append('\\use_geometry 0')
    lines.append('\\use_amsmath 0')
    lines.append('\\paperorientation portrait')


def convert(header,body):
    header_update(header)
    add_end_document(body)
    remove_cursor(body)
    final_dot(body)
    update_inset_label(body)
    update_latexdel(body)
    update_space_units(body)
    update_inset_accent(body)
    space_before_layout(body)
    formula_inset_space_eat(body)
    update_tabular(body)
    update_vfill(body)

if __name__ == "__main__":
    pass

