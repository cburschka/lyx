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

from parser_tools import find_tokens

def convert_box(lines):
    i = 0
    while 1:
        i = find_tokens(lines, ["\\begin_inset Boxed",
                                "\\begin_inset Doublebox",
                                "\\begin_inset Frameless",
                                "\\begin_inset ovalbox",
                                "\\begin_inset Ovalbox",
                                "\\begin_inset Shadowbox"], i)
        if i == -1:
            break

        lines[i] = lines[i][0:13] + 'Box ' + lines[i][13:]
        i = i + 1

def convert(header, body):
    convert_box(body)

if __name__ == "__main__":
    pass
