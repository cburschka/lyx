# This file is part of lyx2lyx
# Copyright (C) 2002 Dekel Tsur <dekel@lyx.org>
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

import string
import re
from parser_tools import find_token

def convert_external(lines):
    external_rexp = re.compile(r'\\begin_inset External ([^,]*),"([^"]*)",')
    external_header = "\\begin_inset External"
    i = 0
    while 1:
        i = find_token(lines, external_header, i)
        if i == -1:
            break
        look = external_rexp.search(lines[i])
        args = ['','']
        if look:
            args[0] = look.group(1)
            args[1] = look.group(2)
        #FIXME: if the previous search fails then warn

        if args[0] == "RasterImage":
            # Convert a RasterImage External Inset to a Graphics Inset.
            top = "\\begin_inset Graphics"
            if args[1]:
                filename = "\tfilename " + args[1]
            lines[i:i+1] = [top, filename]
            i = i + 1
        else:
            # Convert the old External Inset format to the new.
            top = external_header
            template = "\ttemplate " + args[0]
            if args[1]:
                filename = "\tfilename " + args[1]
                lines[i:i+1] = [top, template, filename]
                i = i + 2
            else:
                lines[i:i+1] = [top, template]
                i = i + 1

def convert(header, body):
    convert_external(body)

if __name__ == "__main__":
    pass
