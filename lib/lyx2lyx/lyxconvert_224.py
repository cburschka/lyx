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

from parser_tools import find_token, find_tokens
from sys import stderr

def add_end_layout(lines):
    begin_layout = "\\layout"

    i = find_token(lines, begin_layout, 0)
    if i == -1:
        write(stderr, "lyx2lyx: File with no paragraphs. ")
        lines[0:len(lines)] = []
        return
    i = i + 1

    while 1:
        i = find_tokens(lines, ["\\begin_inset", "\\end_inset", "\\layout",
                                "\\begin_deeper", "\\end_deeper", "\\the_end"], i)

        lines[i:i] = ["\\end_layout"]
        i = i + 1
        i = find_token(lines, begin_layout, i)
        if i == -1:
            return
        i = i + 1
        
def convert(header, body):
    add_end_layout(body)

if __name__ == "__main__":
    pass
