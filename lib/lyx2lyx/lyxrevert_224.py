# This file is part of lyx2lyx
# -*- coding: iso-8859-1 -*-
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

from string import split, join
from parser_tools import find_token, find_tokens

def convert_external(lines):
    external_header = "\\begin_inset External"
    i = 0
    while 1:
        i = find_token(lines, external_header, i)
        if i == -1:
            break

        template = split(lines[i+1])
        template.reverse()
        del lines[i+1]

        filename = split(lines[i+1])
        filename.reverse()
        del lines[i+1]

        params = split(lines[i+1])
        params.reverse()
        if lines[i+1]: del lines[i+1]

        lines[i] = lines[i] + " " + template[0]+ ', "' + filename[0] + '", " '+ join(params[1:]) + '"'
        i = i + 1

def convert_comment(lines):
    i = 0
    while 1:
        i = find_tokens(lines, ["\\begin_inset Comment", "\\begin_inset Greyedout"], i)

        if i == -1:
            return
        lines[i] = "\\begin_inset Note"
        i = i + 1

def convert(header, body):
    convert_external(body)
    convert_comment(body)

if __name__ == "__main__":
    pass
