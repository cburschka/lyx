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


import sys,string
from parser_tools import *

floats = {
    "footnote": ["\\begin_inset Foot\n",
		 "collapsed true\n"],
    "margin":   ["\\begin_inset Marginal\n",
		 "collapsed true\n"],
    "fig":      ["\\begin_inset Float figure\n",
		 "placement htbp\n",
		 "wide false\n",
		 "collapsed false\n"],
    "tab":      ["\\begin_inset Float table\n",
		 "placement htbp\n",
		 "wide false\n",
		 "collapsed false\n"],
    "alg":      ["\\begin_inset Float algorithm\n",
		 "placement htbp\n",
		 "wide false\n",
		 "collapsed false\n"],
    "wide-fig": ["\\begin_inset Float figure\n",
		 "placement htbp\n",
		 "wide true\n",
		 "collapsed false\n"],
    "wide-tab": ["\\begin_inset Float table\n",
		 "placement htbp\n",
		 "wide true\n",
		 "collapsed false\n"]
}

def remove_oldfloat(lines):
    i = 0
    while 1:
	i = find_token(lines, "\\begin_float", i)
	if i == -1:
	    break
	j = find_token(lines, "\\end_float", i+1)
	floattype = string.split(lines[i])[1]
	#print floattype
	start = floats[floattype]+["\n"]
	mid = lines[i+1:j]
	lines[i:j+1]= start+mid+["\\end_inset \n"]
	i = i+1

def remove_oldminipage(lines):
    i = 0
    flag = 0
    while 1:
	i = find_token(lines, "\\pextra_type 2", i)
	if i == -1:
	    break
	hfill = 0
	line = string.split(lines[i])
	if line[4] == "\\pextra_hfill":
	    line[4:6] = []
	    hfill = 1
	if line[4] == "\\pextra_start_minipage":
	    line[4:6] = []
	width = line[5]
	if line[4] == "\\pextra_widthp":
	    width = line[5]+"col%"

	start = ["\\begin_inset Minipage\n",
		 "position %s\n" % line[3],
		 "inner_position 0\n",
		 'height "0pt"\n',
		 'width "%s"\n' % width,
		 "collapsed false\n"
		 ]
	if flag:
	    flag = 0
	    if hfill:
		start = ["\n","\hfill\n","\n"]+start
	else:
	    start = ["\\layout Standard\n"] + start

	j = find_token_backwards(lines,"\\layout", i-1)
	j0 = j
	mid = lines[j:i]

	j = find_token2(lines,"\\layout", "\\end_float", i+1)
	mid = mid+lines[i+1:j]

	while 1:
	    i = find_token2(lines,"\\layout", "\\pextra_type", j+1)
	    if i == -1 or not check_token(lines,  "\\pextra_type", i):
		break
	    line = string.split(lines[i])
	    if line[4] == "\\pextra_hfill":
		line[4:6] = []
	    if line[4] == "\\pextra_start_minipage" and line[5] == "1":
		flag = 1
		break
	    j = find_token_backwards(lines,"\\layout", i-1)
	    mid = mid+lines[j:i]
	    j = find_token2(lines,"\\layout", "\\end_float", i+1)
	    mid = mid+lines[i+1:j]

	end = ["\\end_inset \n"]

	lines[j0:j] = start+mid+end
	i = i+1

def convert(header,body):
    remove_oldminipage(body)
    remove_oldfloat(body)

if __name__ == "__main__":
    pass

