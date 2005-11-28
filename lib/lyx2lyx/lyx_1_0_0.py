# This file is part of lyx2lyx
# -*- coding: iso-8859-1 -*-
# Copyright (C) 2004 José Matos <jamatos@lyx.org>
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
from parser_tools import find_token, find_re

def obsolete_latex_title(file):
    body = file.body
    i = 0
    while 1:
        i = find_token(body, '\\layout', i)
        if i == -1:
            return

        if string.find(string.lower(body[i]),'latex title') != -1:
            body[i] = '\\layout Title'

        i = i + 1


# Update from tabular format 3 to 4 if necessary
def update_tabular(file):
    lines = file.body
    lyxtable_re = re.compile(r".*\\LyXTable$")
    i=0
    while 1:
        i = find_re(lines, lyxtable_re, i)
        if i == -1:
            break
        i = i + 1
        format = lines[i][8:]

        if format != '3':
            continue

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

        while string.strip(lines[i]):
            lines[i] = lines[i] + ' 0 0 0'
            i = i + 1

        lines[i] = string.strip(lines[i])


convert = [[215, [obsolete_latex_title, update_tabular]]]
revert  = []


if __name__ == "__main__":
    pass

