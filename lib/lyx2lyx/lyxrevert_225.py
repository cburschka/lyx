# This file is part of lyx2lyx
# Copyright (C) 2003 José Matos <jamatos@fep.up.pt>
# Copyright (C) 2003 Georg Baum <Georg.Baum@post.rwth-aachen.de>
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

import re
from parser_tools import find_token, find_end_of_inset, get_next_paragraph, \
                         get_paragraph, get_value, del_token, is_nonempty_line
from string import find, replace, split
import sys

def rm_end_layout(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\end_layout', i)

        if i == -1:
            return

        del lines[i]

def begin_layout2layout(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\begin_layout', i)
        if i == -1:
            return

        lines[i] = replace(lines[i], '\\begin_layout', '\\layout')
        i = i + 1

def table_valignment_middle(lines, start, end):
    for i in range(start, end):
        if re.search('^<(column|cell) .*valignment="middle".*>$', lines[i]):
            lines[i] = replace(lines[i], 'valignment="middle"', 'valignment="center"')

def valignment_middle(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset  Tabular', i)
        if i == -1:
            return
        j = find_end_of_inset(lines, i + 1)
        if j == -1:
            #this should not happen
            valignment_middle(lines, i + 1, len(lines))
            return
        valignment_middle(lines, i + 1, j)
        i = j + 1

def end_document(lines):
    i = find_token(lines, "\\end_document", 0)
    if i == -1:
        lines.append("\\the_end")
        return
    lines[i] = "\\the_end"

# Convert backslashes into valid ERT code, append the converted text to
# lines[i] and return the (maybe incremented) line index i
def convert_ertbackslash(lines, i, ert):
    for c in ert:
	if c == '\\':
	    lines[i] = lines[i] + '\\backslash '
	    lines.insert(i, '')
	    i = i + 1
	else:
	    lines[i] = lines[i] + c
    return i

def convert_vspace(header, lines):

    # Get default spaceamount
    i = find_token(header, '\\defskip', 0)
    if i == -1:
	defskipamount = 'medskip'
    else:
	defskipamount = split(header[i])[1]

    # Convert the insets
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset VSpace', i)
        if i == -1:
            return
	spaceamount = split(lines[i])[2]

	# Are we at the beginning or end of a paragraph?
	paragraph_start = 1
	start = get_paragraph(lines, i) + 1
	for k in range(start, i):
	    if is_nonempty_line(lines[k]):
		paragraph_start = 0
		break
	paragraph_end = 1
	j = find_end_of_inset(lines, i)
	if j == -1:
	    sys.stderr.write("Malformed lyx file: Missing '\\end_inset'\n")
	    i = i + 1
	    continue
	end = get_next_paragraph(lines, i)
	for k in range(j + 1, end):
	    if is_nonempty_line(lines[k]):
		paragraph_end = 0
		break

	# Convert to paragraph formatting if we are at the beginning or end
	# of a paragraph and the resulting paragraph would not be empty
	if ((paragraph_start and not paragraph_end) or
	    (paragraph_end   and not paragraph_start)):
	    # The order is important: del and insert invalidate some indices
	    del lines[j]
	    del lines[i]
	    if paragraph_start:
		lines.insert(start, '\\added_space_top ' + spaceamount + ' ')
	    else:
		lines.insert(start, '\\added_space_bottom ' + spaceamount + ' ')
	    continue

	# Convert to ERT
	lines[i:i+1] = ['\\begin_inset ERT', 'status Collapsed', '',
	                '\\layout Standard', '', '\\backslash ']
	i = i + 6
	if spaceamount[-1] == '*':
	    spaceamount = spaceamount[:-1]
	    keep = 1
	else:
	    keep = 0

	# Replace defskip by the actual value
	if spaceamount == 'defskip':
	    spaceamount = defskipamount

	# LaTeX does not know \\smallskip* etc
	if keep:
	    if spaceamount == 'smallskip':
		spaceamount = '\\smallskipamount'
	    elif spaceamount == 'medskip':
		spaceamount = '\\medskipamount'
	    elif spaceamount == 'bigskip':
		spaceamount = '\\bigskipamount'
	    elif spaceamount == 'vfill':
		spaceamount = '\\fill'

	# Finally output the LaTeX code
	if (spaceamount == 'smallskip' or spaceamount == 'medskip' or
	    spaceamount == 'bigskip'   or spaceamount == 'vfill'):
	    lines.insert(i, spaceamount)
	else :
	    if keep:
		lines.insert(i, 'vspace*{')
	    else:
		lines.insert(i, 'vspace{')
	    i = convert_ertbackslash(lines, i, spaceamount)
            lines[i] =  lines[i] + '}'
        i = i + 1

# Convert a LyX length into valid ERT code and append it to lines[i]
# Return the (maybe incremented) line index i
def convert_ertlen(lines, i, len, special):
    units = {"text%":"\\textwidth", "col%":"\\columnwidth",
             "page%":"\\pagewidth", "line%":"\\linewidth",
             "theight%":"\\textheight", "pheight%":"\\pageheight"}

    # Convert special lengths
    if special != 'none':
	len = '%f\\' % len2value(len) + special

    # Convert LyX units to LaTeX units
    for unit in units.keys():
	if find(len, unit) != -1:
	    len = '%f' % (len2value(len) / 100) + units[unit]
	    break

    # Convert backslashes and insert the converted length into lines
    return convert_ertbackslash(lines, i, len)

# Return the value of len without the unit in numerical form
def len2value(len):
    result = re.search('([+-]?[0-9.]+)', len)
    if result:
	return float(result.group(1))
    # No number means 1.0
    return 1.0

def convert_frameless_box(lines):
    pos = ['t', 'c', 'b']
    inner_pos = ['c', 't', 'b', 's']
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset Frameless', i)
        if i == -1:
            return
	j = find_end_of_inset(lines, i)
	if j == -1:
	    sys.stderr.write("Malformed lyx file: Missing '\\end_inset'\n")
	    i = i + 1
	    continue
	del lines[i]

	# Gather parameters
	params = {'position':'0', 'hor_pos':'c', 'has_inner_box':'1',
                  'inner_pos':'1', 'use_parbox':'0', 'width':'100col%',
	          'special':'none', 'height':'1in',
	          'height_special':'totalheight', 'collapsed':'false'}
	for key in params.keys():
	    value = replace(get_value(lines, key, i, j), '"', '')
	    if value != "":
		if key == 'position':
		    # convert new to old position: 'position "t"' -> 0
		    value = find_token(pos, value, 0)
		    if value != -1:
			params[key] = value
		elif key == 'inner_pos':
		    # convert inner position
		    value = find_token(inner_pos, value, 0)
		    if value != -1:
			params[key] = value
		else:
		    params[key] = value
		j = del_token(lines, key, i, j)
	i = i + 1

	# Convert to minipage or ERT?
	# Note that the inner_position and height parameters of a minipage
	# inset are ignored and not accessible for the user, although they
	# are present in the file format and correctly read in and written.
	# Therefore we convert to ERT if they do not have their LaTeX
	# defaults. These are:
	# - the value of "position" for "inner_pos"
	# - "\totalheight"          for "height"
	if (params['use_parbox'] != '0' or
	    params['has_inner_box'] != '1' or
	    params['special'] != 'none' or
	    inner_pos[params['inner_pos']] != pos[params['position']] or
	    params['height_special'] != 'totalheight' or
	    len2value(params['height']) != 1.0):

	    # Convert to ERT
	    if params['collapsed'] == 'true':
		params['collapsed'] = 'Collapsed'
	    else:
		params['collapsed'] = 'Open'
	    lines[i : i] = ['\\begin_inset ERT', 'status ' + params['collapsed'],
	                    '', '\\layout Standard', '', '\\backslash ']
	    i = i + 6
	    if params['use_parbox'] == '1':
		lines.insert(i, 'parbox')
	    else:
		lines.insert(i, 'begin{minipage}')
	    lines[i] = lines[i] + '[' + pos[params['position']] + ']['
	    i = convert_ertlen(lines, i, params['height'], params['height_special'])
	    lines[i] = lines[i] + '][' + inner_pos[params['inner_pos']] + ']{'
	    i = convert_ertlen(lines, i, params['width'], params['special'])
	    lines[i] = lines[i] + '}{'
	    i = i + 1
	    lines[i:i] = ['', '\\end_inset ']
	    i = i + 2
	    j = find_end_of_inset(lines, i)
	    if j == -1:
		sys.stderr.write("Malformed lyx file: Missing '\\end_inset'\n")
		break
	    lines[j-1:j-1] += ['\\begin_inset ERT', 'status ' + params['collapsed'],
	                       '', '\\layout Standard', '']
	    j = j + 4
	    if params['use_parbox'] == '1':
		lines.insert(j, '}')
	    else:
		lines[j:j] = ['\\backslash ', 'end{minipage}']
	else:

	    # Convert to minipage
	    lines[i:i] = ['\\begin_inset Minipage',
	                  'position %d' % params['position'],
			  'inner_position %d' % params['inner_pos'],
			  'height "' + params['height'] + '"',
	                  'width "' + params['width'] + '"',
	                  'collapsed ' + params['collapsed']]
	    i = i + 6

def convert(header, body):
    rm_end_layout(body)
    begin_layout2layout(body)
    end_document(body)
    valignment_middle(body)
    convert_vspace(header, body)
    convert_frameless_box(body)

if __name__ == "__main__":
    pass
