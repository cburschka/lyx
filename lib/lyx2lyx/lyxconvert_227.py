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

        # Seach for a line starting 'collapsed'
        # If, however, we find a line starting '\layout' (_always_ present)
        # then break with a warning message
        i = i + 1
        while 1:
            if (lines[i] == "collapsed false"):
                lines[i] = "status open"
                break
            elif (lines[i] == "collapsed true"):
                lines[i] = "status collapsed"
                break
            elif (lines[i][:7] == "\\layout"):
                sys.stderr.write("Malformed lyx file\n")
                break
            i = i + 1

        i = i + 1


def convert_minipage(lines):
    """ Convert minipages to the box inset.
    We try to use the same order of arguments as lyx does.
    """
    pos = ["t","c","b"]
    inner_pos = ["c","t","b","s"]

    i = 0
    while 1:
        i = find_token(lines, "\\begin_inset Minipage", i)
        if i == -1:
            return

        lines[i] = "\\begin_inset Box Frameless"
        i = i + 1

        # convert old to new position using the pos list
        if lines[i][:8] == "position":
            lines[i] = 'position "%s"' % pos[int(lines[i][9])]
        else:
            lines.insert(i, 'position "%s"' % pos[0])
        i = i + 1

        lines.insert(i, 'hor_pos "c"')
        i = i + 1
        lines.insert(i, 'has_inner_box 1')
        i = i + 1

        # convert the inner_position
        if lines[i][:14] == "inner_position":
            lines[i] = 'inner_pos "%s"' %  inner_pos[int(lines[i][15])]
        else:
            lines.insert('inner_pos "%s"' % inner_pos[0])
        i = i + 1

        # We need this since the new file format has a height and width
        # in a different order.
        if lines[i][:6] == "height":
            height = lines[i][6:]
            # test for default value of 221 and convert it accordingly
            if height == ' "0pt"':
                height = ' "1pt"'
            del lines[i]
        else:
            height = ' "1pt"'

        if lines[i][:5] == "width":
            width = lines[i][5:]
            del lines[i]
        else:
            width = ' "0"'

        lines.insert(i, 'use_parbox 0')
        i = i + 1
        lines.insert(i, 'width' + width)
        i = i + 1
        lines.insert(i, 'special "none"')
        i = i + 1
        lines.insert(i, 'height' + height)
        i = i + 1
        lines.insert(i, 'height_special "totalheight"')
        i = i + 1

def convert(header, body):
    convert_collapsable(body)
    convert_minipage(body)

if __name__ == "__main__":
    pass
