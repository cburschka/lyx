# This file is part of lyx2lyx
# -*- coding: iso-8859-1 -*-
# Copyright (C) 2006 José Matos <jamatos@lyx.org>
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

##
#  Notes: Framed/Shaded
#

def revert_framed(file):
    i = 0
    while 1:
        i = find_tokens(file.body, ["\\begin_inset Note Framed", "\\begin_inset Note Shaded"], i)

        if i == -1:
            return
        file.body[i] = "\\begin_inset Note"
        i = i + 1

##
# Conversion hub
#

convert = [[246, []]]
revert  = [[245, [revert_framed]]]

if __name__ == "__main__":
    pass

