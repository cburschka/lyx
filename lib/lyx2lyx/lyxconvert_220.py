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


import sys,string,re
from parser_tools import *

def change_insetgraphics(lines):
    i = 0
    while 1:
	i = find_token(lines, "\\begin_inset Graphics", i)
	if i == -1:
	    break
	j = find_end_of_inset(lines, i)

	if get_value(lines, "display", i, j) == "default":
	    j = del_token(lines, "display", i, j)
	if get_value(lines, "rotateOrigin", i, j) == "leftBaseline":
	    j = del_token(lines, "rotateOrigin", i, j)

	k = find_token2(lines, "rotate", i, j)
	if k != -1:
	    del lines[k]
	    j = j-1
	else:
	    j = del_token(lines, "rotateAngle", i, j)

	k = find_token2(lines, "size_type", i, j)
	if k == -1:
	    k = find_token2(lines, "size_kind", i, j)
	size_type = string.split(lines[k])[1]
	del lines[k]
	j = j-1
	if size_type in ["0", "original"]:
	    j = del_token(lines, "width", i, j)
	    j = del_token(lines, "height", i, j)
	    j = del_token(lines, "scale", i, j)
	elif size_type in ["2", "scale"]:
	    j = del_token(lines, "width", i, j)
	    j = del_token(lines, "height", i, j)
	    if get_value(lines, "scale", i, j) == "100":
		j = del_token(lines, "scale", i, j)
	else:
	    j = del_token(lines, "scale", i, j)

	k = find_token2(lines, "lyxsize_type", i, j)
	if k == -1:
	    k = find_token2(lines, "lyxsize_kind", i, j)
	lyxsize_type = string.split(lines[k])[1]
	del lines[k]
	j = j-1
	j = del_token(lines, "lyxwidth", i, j)
	j = del_token(lines, "lyxheight", i, j)
	if lyxsize_type not in ["2", "scale"] or \
	   get_value(lines, "lyxscale", i, j) == "100":
	    j = del_token(lines, "lyxscale", i, j)
	
	i = i+1


def convert(header, body):
    change_insetgraphics(body)

if __name__ == "__main__":
    pass
