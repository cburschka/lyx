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

import sys
from parser_tools import find_token, find_tokens

def convert_collapsable(lines):
    i = 0
    while 1:
        i = find_tokens(lines, ["\\begin_inset Box",
                                "\\begin_inset Branch",
                                "\\begin_inset CharStyle",
                                "\\begin_inset Float",
                                "\\begin_inset Foot",
                                "\\begin_inset Marginal",
                                "\\begin_inset Note",
                                "\\begin_inset OptArg",
                                "\\begin_inset Wrap"], i)
        if i == -1:
            break

        # Seach for a line starting 'status'
        # If, however, we find a line starting '\begin_layout'
        # (_always_ present) then break with a warning message
        i = i + 1
        while 1:
            if (lines[i] == "status open"):
                lines[i] = "collapsed false"
                break
            elif (lines[i] == "status collapsed" or
                  lines[i] == "status inlined"):
                lines[i] = "collapsed true"
                break
            elif (lines[i][:13] == "\\begin_layout"):
                sys.stderr.write("Malformed lyx file\n")
                break
            i = i + 1

        i = i + 1

def convert_ert(lines):
    i = 0
    while 1:
        i = find_token(lines, "\\begin_inset ERT", i)
        if i == -1:
            break

        # Seach for a line starting 'status'
        # If, however, we find a line starting '\begin_layout'
        # (_always_ present) then break with a warning message
        i = i + 1
        while 1:
            if (lines[i] == "status open"):
                lines[i] = "status Open"
                break
            elif (lines[i] == "status collapsed"):
                lines[i] = "status Collapsed"
                break
            elif (lines[i] == "status inlined"):
                lines[i] = "status Inlined"
                break
            elif (lines[i][:13] == "\\begin_layout"):
                sys.stderr.write("Malformed lyx file\n")
                break
            i = i + 1

        i = i + 1

def convert(header, body):
    convert_collapsable(body)
    convert_ert(body)

if __name__ == "__main__":
    pass
