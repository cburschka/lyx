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

floats = {
    "footnote": ["\\begin_inset Foot",
		 "collapsed true"],
    "margin":   ["\\begin_inset Marginal",
		 "collapsed true"],
    "fig":      ["\\begin_inset Float figure",
		 "wide false",
		 "collapsed false"],
    "tab":      ["\\begin_inset Float table",
		 "wide false",
		 "collapsed false"],
    "alg":      ["\\begin_inset Float algorithm",
		 "wide false",
		 "collapsed false"],
    "wide-fig": ["\\begin_inset Float figure",
		 "wide true",
		 "collapsed false"],
    "wide-tab": ["\\begin_inset Float table",
		 "wide true",
		 "collapsed false"]
}

font_tokens = ["\\family", "\\series", "\\shape", "\\size", "\\emph",
	       "\\bar", "\\noun", "\\color", "\\lang", "\\latex"]

#
# Change \begin_float .. \end_float into \begin_inset Float .. \end_inset
#

pextra_type3_rexp = re.compile(r".*\\pextra_type\s+3")
pextra_rexp = re.compile(r"\\pextra_type\s+(\S+)"+\
			 r"(\s+\\pextra_alignment\s+(\S+))?"+\
			 r"(\s+\\pextra_hfill\s+(\S+))?"+\
			 r"(\s+\\pextra_start_minipage\s+(\S+))?"+\
			 r"(\s+(\\pextra_widthp?)\s+(\S*))?")

def get_width(mo):
    if mo.group(10):
	if mo.group(9) == "\\pextra_widthp":
	    return mo.group(10)+"col%"
	else:
	    return mo.group(10)
    else:
	return "100col%"

def remove_oldfloat(lines, language):
    i = 0
    while 1:
	i = find_token(lines, "\\begin_float", i)
	if i == -1:
	    break
	# There are no nested floats, so finding the end of the float is simple
	j = find_token(lines, "\\end_float", i+1)

	floattype = string.split(lines[i])[1]
	if not floats.has_key(floattype):
	    sys.stderr.write("Error! Unknown float type "+floattype+"\n")
	    floattype = "fig"

	# skip \end_deeper tokens
	i2 = i+1
	while check_token(lines[i2], "\\end_deeper"):
	    i2 = i2+1
	if i2 > i+1:
	    j2 = get_next_paragraph(lines, j+1)
	    lines[j2:j2] = ["\\end_deeper "]*(i2-(i+1))

	new = floats[floattype]+[""]

	# Check if the float is floatingfigure
	k = find_re(lines, pextra_type3_rexp, i, j)
	if k != -1:
	    mo = pextra_rexp.search(lines[k])
	    width = get_width(mo)
	    lines[k] = re.sub(pextra_rexp, "", lines[k])
	    new = ["\\begin_inset Wrap figure",
		   'width "%s"' % width,
		   "collapsed false",
		   ""]

	new = new+lines[i2:j]+["\\end_inset ", ""]

	# After a float, all font attributes are reseted.
	# We need to output '\foo default' for every attribute foo
	# whose value is not default before the float.
	# The check here is not accurate, but it doesn't matter
	# as extra '\foo default' commands are ignored.
	# In fact, it might be safer to output '\foo default' for all 
	# font attributes.
	k = get_paragraph(lines, i)
	flag = 0
	for token in font_tokens:
	    if find_token(lines, token, k, i) != -1:
		if not flag:
		    # This is not necessary, but we want the output to be
		    # as similar as posible to the lyx format
		    flag = 1
		    new.append("")
		if token == "\\lang":
		    new.append(token+" "+language)
		else:
		    new.append(token+" default ")

	lines[i:j+1] = new
	i = i+1

pextra_type2_rexp = re.compile(r".*\\pextra_type\s+2")
pextra_type2_rexp2 = re.compile(r".*(\\layout|\\pextra_type\s+2)")

def remove_oldminipage(lines):
    i = 0
    flag = 0
    while 1:
	i = find_re(lines, pextra_type2_rexp, i)
	if i == -1:
	    break
	
	mo = pextra_rexp.search(lines[i])
	position = mo.group(3)
	hfill = mo.group(5)
	width = get_width(mo)
	lines[i] = re.sub(pextra_rexp, "", lines[i])

	start = ["\\begin_inset Minipage",
		 "position " + position,
		 "inner_position 0",
		 'height "0pt"',
		 'width "%s"' % width,
		 "collapsed false"
		 ]
	if flag:
	    flag = 0
	    if hfill:
		start = ["","\hfill",""]+start
	else:
	    start = ["\\layout Standard"] + start

	j = find_token_backwards(lines,"\\layout", i-1)
	j0 = j

	j = find_tokens(lines, ["\\layout", "\\end_float"], i+1)
	# j can be -1

	count = 0
	while 1:
	    # collect more paragraphs to the minipage
	    count = count+1
	    if j == -1 or not check_token(lines[j], "\\layout"):
		break
	    i = find_re(lines, pextra_type2_rexp2, j+1)
	    if i == -1:
		break
	    mo = pextra_rexp.search(lines[i])
	    if not mo:
		break
	    if mo.group(7) == "1":
		flag = 1
		break
	    lines[i] = re.sub(pextra_rexp, "", lines[i])
	    j = find_tokens(lines, ["\\layout", "\\end_float"], i+1)

	mid = lines[j0:j]
	end = ["\\end_inset "]

	lines[j0:j] = start+mid+end
	i = i+1

def is_empty(lines):
    return filter(is_nonempty_line, lines) == []

move_rexp =  re.compile(r"\\(family|series|shape|size|emph|numeric|bar|noun|end_deeper)")
ert_rexp = re.compile(r"\\begin_inset|.*\\SpecialChar")
spchar_rexp = re.compile(r"(.*)(\\SpecialChar.*)")
ert_begin = ["\\begin_inset ERT",
	     "status Collapsed",
	     "",
	     "\\layout Standard"]

def remove_oldert(lines):
    i = 0
    while 1:
	i = find_tokens(lines, ["\\latex latex", "\\layout LaTeX"], i)
	if i == -1:
	    break
	j = i+1
	while 1:
	    j = find_tokens(lines, ["\\latex default", "\\begin_inset", "\\layout", "\\end_float", "\\the_end"],
			    j)
	    if check_token(lines[j], "\\begin_inset"):
		j = find_end_of_inset(lines, j)
	    else:
		break

	if check_token(lines[j], "\\layout"):
	    while j-1 >= 0 and check_token(lines[j-1], "\\begin_deeper"):
		j = j-1

	# We need to remove insets, special chars & font commands from ERT text
	new = []
	new2 = []
	if check_token(lines[i], "\\layout LaTeX"):
	    new = ["\layout Standard", "", ""]
	    # We have a problem with classes in which Standard is not the default layout!

	k = i+1
	while 1:
	    k2 = find_re(lines, ert_rexp, k, j)
	    inset = specialchar = 0
	    if k2 == -1:
		k2 = j
	    elif check_token(lines[k2], "\\begin_inset"):
		inset = 1
	    else:
		specialchar = 1
		mo = spchar_rexp.match(lines[k2])
		lines[k2] = mo.group(1)
		specialchar_str = mo.group(2)
		k2 = k2+1

	    tmp = []
	    for line in lines[k:k2]:
                # Move some lines outside the ERT inset:
		if move_rexp.match(line):
		    if new2 == []:
			# This is not necessary, but we want the output to be
			# as similar as posible to the lyx format
			new2 = [""]
		    new2.append(line)
		elif not check_token(line, "\\latex"):
		    tmp.append(line)

	    if is_empty(tmp):
		if filter(lambda x:x != "", tmp) != []:
		    if new == []:
			# This is not necessary, but we want the output to be
			# as similar as posible to the lyx format
			lines[i-1] = lines[i-1]+" "
		    else:
			new = new+[" "]
	    else:
		new = new+ert_begin+tmp+["\\end_inset ", ""]

	    if inset:
		k3 = find_end_of_inset(lines, k2)
		new = new+[""]+lines[k2:k3+1]+[""] # Put an empty line after \end_inset
		k = k3+1
		# Skip the empty line after \end_inset
		if not is_nonempty_line(lines[k]):
		    k = k+1
		    new.append("")
	    elif specialchar:
		if new == []:
		    # This is not necessary, but we want the output to be
		    # as similar as posible to the lyx format
		    lines[i-1] = lines[i-1]+specialchar_str
		    new = [""]
		else:
		    new = new+[specialchar_str, ""]
		k = k2
	    else:
		break

	new = new+new2
	if not check_token(lines[j], "\\latex "):
	    new = new+[""]+[lines[j]]
	lines[i:j+1] = new
	i = i+1

    # Delete remaining "\latex xxx" tokens
    i = 0
    while 1:
	i = find_token(lines, "\\latex ", i)
	if i == -1:
	    break
	del lines[i]


def remove_oldertinset(lines):
    i = 0
    while 1:
	i = find_token(lines, "\\begin_inset ERT", i)
	if i == -1:
	    break
	j = find_end_of_inset(lines, i)
	k = find_token(lines, "\\layout", i+1)
	l = get_paragraph(lines, i)
	if lines[k] == lines[l]: # same layout
	    k = k+1
	new = lines[k:j]
	lines[i:j+1] = new
	i = i+1

def is_ert_paragraph(lines, i):
    i = find_nonempty_line(lines, i+1)
    if not check_token(lines[i], "\\begin_inset ERT"):
	return 0
    j = find_end_of_inset(lines, i)
    k = find_nonempty_line(lines, j+1)
    return check_token(lines[k], "\\layout")

def combine_ert(lines):
    i = 0
    while 1:
	i = find_token(lines, "\\begin_inset ERT", i)
	if i == -1:
	    break
	j = find_token_backwards(lines,"\\layout", i-1)
	count = 0
	text = []
	while is_ert_paragraph(lines, j):

	    count = count+1
	    i2 = find_token(lines, "\\layout", j+1)
	    k = find_token(lines, "\\end_inset", i2+1)
	    text = text+lines[i2:k]
	    j = find_token(lines, "\\layout", k+1)
	    if j == -1:
		break

	if count >= 2:
	    j = find_token(lines, "\\layout", i+1)
	    lines[j:k] = text

	i = i+1
	
oldunits = ["pt", "cm", "in", "text%", "col%"]

def get_length(lines, name, start, end):
    i = find_token(lines, name, start, end)
    if i == -1:
	return ""
    x = string.split(lines[i])
    return x[2]+oldunits[int(x[1])]

def write_attribute(x, token, value):
    if value != "":
	x.append("\t"+token+" "+value)

def remove_figinset(lines):
    i = 0
    while 1:
	i = find_token(lines, "\\begin_inset Figure", i)
	if i == -1:
	    break
	j = find_end_of_inset(lines, i)

	lyxwidth = string.split(lines[i])[3]+"pt"
	lyxheight = string.split(lines[i])[4]+"pt"

	filename = get_value(lines, "file", i+1, j)

	width = get_length(lines, "width", i+1, j)
	# what does width=5 mean ?
	height = get_length(lines, "height", i+1, j)
	rotateAngle = get_value(lines, "angle", i+1, j)
	if width == "" and height == "":
	    size_type = "0"
	else:
	    size_type = "1"

	flags = get_value(lines, "flags", i+1, j)
	x = int(flags)%4
	if x == 1:
	    display = "monochrome"
	elif x == 2:
	    display = "gray"
	else:
	    display = "color"

	subcaptionText = get_value(lines, "subcaption", i+1, j)
	if subcaptionText != "":
	    subcaptionText = '"'+subcaptionText+'"'
	k = find_token(lines, "subfigure", i+1,j)
	if k == -1:
	    subcaption = 0
	else:
	    subcaption = 1

	new = ["\\begin_inset Graphics FormatVersion 1"]
	write_attribute(new, "filename", filename)
	write_attribute(new, "display", display)
	if subcaption:
	    new.append("\tsubcaption")
	write_attribute(new, "subcaptionText", subcaptionText)
	write_attribute(new, "size_type", size_type)
	write_attribute(new, "width", width)
	write_attribute(new, "height", height)
	if rotateAngle != "":
	    new.append("\trotate")
	    write_attribute(new, "rotateAngle", rotateAngle)
	write_attribute(new, "rotateOrigin", "leftBaseline")
	write_attribute(new, "lyxsize_type", "1")
	write_attribute(new, "lyxwidth", lyxwidth)
	write_attribute(new, "lyxheight", lyxheight)
	new = new + ["\end_inset"]
	lines[i:j+1] = new

attr_re = re.compile(r' \w*="(false|0|)"')
line_re = re.compile(r'<(features|column|row|cell)')

def update_tabular(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset  Tabular', i)
        if i == -1:
            break

	for k in get_tabular_lines(lines, i):
	    if check_token(lines[k], "<lyxtabular"):
		lines[k] = string.replace(lines[k], 'version="2"', 'version="3"')
	    elif check_token(lines[k], "<column"):
		lines[k] = string.replace(lines[k], 'width=""', 'width="0pt"')

	    if line_re.match(lines[k]):
		lines[k] = re.sub(attr_re, "", lines[k])

	i = i+1

def change_preamble(lines):
    i = find_token(lines, "\\use_amsmath", 0)
    if i == -1:
	return
    lines[i+1:i+1] = ["\\use_natbib 0",
		      "\use_numerical_citations 0"]

def convert(header, body):
    language = get_value(header, "\\language", 0)
    if language == "":
	language = "english"

    change_preamble(header)
    update_tabular(body)
    remove_oldminipage(body)
    remove_oldfloat(body, language)
    remove_figinset(body)
    remove_oldertinset(body)
    remove_oldert(body)
    combine_ert(body)

if __name__ == "__main__":
    pass
