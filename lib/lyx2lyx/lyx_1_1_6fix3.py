# This file is part of lyx2lyx
# -*- coding: iso-8859-1 -*-
# Copyright (C) 2002-2004 José Matos <jamatos@lyx.org>
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
from parser_tools import find_token

def bool_table(item):
    if item == "0":
        return "false"
    # should emit a warning if item != "1"
    return "true"


align_table = {"0": "top", "2": "left", "4": "right", "8": "center"}
use_table = {"0": "none", "1": "parbox"}
table_meta_re = re.compile(r'<LyXTabular version="?1"? rows="?(\d*)"? columns="?(\d*)"?>')

def update_tabular(file):
    lines = file.body
    i=0
    while 1:
        i = find_token(lines, '\\begin_inset  Tabular', i)
        if i == -1:
            break

        i = i +1

        # scan table header meta-info
        res = table_meta_re.match( lines[i] )
        if res:
            val = res.groups()
            lines[i] = '<lyxtabular version="2" rows="%s" columns="%s">' % val

        j = find_token(lines, '</LyXTabular>', i) + 1
        if j == 0:
            file.warning( "Error: Bad lyx format i=%d j=%d" % (i,j))
            break

        new_table = table_update(lines[i:j])
	lines[i:j] = new_table
        i = i + len(new_table)


col_re = re.compile(r'<column alignment="?(\d)"? valignment="?(\d)"? leftline="?(\d)"? rightline="?(\d)"? width="(.*)" special="(.*)">')
cell_re = re.compile(r'<cell multicolumn="?(\d)"? alignment="?(\d)"? valignment="?(\d)"? topline="?(\d)"? bottomline="?(\d)"? leftline="?(\d)"? rightline="?(\d)"? rotate="?(\d)"? usebox="?(\d)"? width="(.*)" special="(.*)">')
features_re = re.compile(r'<features rotate="?(\d)"? islongtable="?(\d)"? endhead="?(-?\d)"? endfirsthead="?(-?\d)"? endfoot="?(-?\d)"? endlastfoot="?(-?\d)"?>')
row_re = re.compile(r'<row topline="?(\d)"? bottomline="?(\d)"? newpage="?(\d)"?>')

def table_update(lines):
    lines[1] = string.replace(lines[1], '<Features', '<features')
    res = features_re.match( lines[1] )
    if res:
        val = res.groups()
        lines[1] = '<features rotate="%s" islongtable="%s" endhead="%s" endfirsthead="%s" endfoot="%s" endlastfoot="%s">' % (bool_table(val[0]), bool_table(val[1]), val[2], val[3], val[4], val[5])

    if lines[2]=="":
        del lines[2]
    i = 2
    col_info = []
    while i < len(lines):
        lines[i] = string.replace(lines[i], '<Cell', '<cell')
        lines[i] = string.replace(lines[i], '</Cell', '</cell')
        lines[i] = string.replace(lines[i], '<Row', '<row')
        lines[i] = string.replace(lines[i], '</Row', '</row')
        lines[i] = string.replace(lines[i], '<Column', '<column')
        lines[i] = string.replace(lines[i], '</Column', '</column')
        lines[i] = string.replace(lines[i], '</LyXTabular', '</lyxtabular')
        k = string.find (lines[i], '<column ')
        if k != -1:
            col_info.append(lines[i])
            del lines[i]
            continue

        if lines[i] == '</column>' or lines[i] == '<column>':
            del lines[i]
            continue

        res = cell_re.match(lines[i])
        if res:
            val = res.groups()
            lines[i] = '<cell multicolumn="%s" alignment="%s" valignment="%s" topline="%s" bottomline="%s" leftline="%s" rightline="%s" rotate="%s" usebox="%s" width="%s" special="%s">' % ( val[0], align_table[val[1]], align_table[val[2]], bool_table(val[3]), bool_table(val[4]), bool_table(val[5]), bool_table(val[6]), bool_table(val[7]), use_table[val[8]], val[9], val[10])

        res = row_re.match(lines[i])
        if res:
            val = res.groups()
            lines[i] = '<row topline="%s" bottomline="%s" newpage="%s">' % (bool_table(val[0]), bool_table(val[1]), bool_table(val[2]))

        i = i + 1

    j = len(col_info)
    for i in range(j):
        res = col_re.match(col_info[i])
        if res:
            val = res.groups()
            col_info[i] = '<column alignment="%s" valignment="%s" leftline="%s" rightline="%s" width="%s" special="%s">' \
                          % ( align_table[val[0]], align_table[val[1]], bool_table(val[2]), bool_table(val[3]), val[4],val[5])

    return lines[:2] + col_info + lines[2:]


def convert(file):
    table = [update_tabular]

    for conv in table:
        conv(file)

    file.format = 218


def revert(file):
    file.error("The convertion to an older format (%s) is not implemented." % file.format)


if __name__ == "__main__":
    pass
