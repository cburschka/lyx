# This file is part of lyx2lyx
# Copyright (C) 2003 JosÅÈ Matos <jamatos@fep.up.pt>
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
from parser_tools import find_tokens

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

        if lines[i][:16] == "\\begin_inset Box":
            # Skip box parameters
            i = i + 10
        else:
            # We are interested in the next line
            i = i + 1
        if (lines[i] == "status open"):
            lines[i] = "collapsed false"
        elif (lines[i] == "status collapsed" or
              lines[i] == "status inlined"):
            lines[i] = "collapsed true"
        else:
            sys.stderr.write("Malformed lyx file\n")

        i = i + 1

def convert(header, body):
    convert_collapsable(body)

if __name__ == "__main__":
    pass
