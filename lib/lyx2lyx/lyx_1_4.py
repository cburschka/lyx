# This file is part of lyx2lyx
# -*- coding: iso-8859-1 -*-
# Copyright (C) 2002 Dekel Tsur <dekel@lyx.org>
# Copyright (C) 2002-2004 José Matos <jamatos@lyx.org>
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
from os import access, F_OK
import os.path
from parser_tools import find_token, find_end_of_inset, get_next_paragraph, \
                         get_paragraph, get_value, del_token, is_nonempty_line,\
			 find_tokens, find_end_of, find_token2
from sys import stdin
from string import replace, split, find, strip, join

##
# Remove \color default
#
def remove_color_default(lines):
    i = 0
    while 1:
        i = find_token(lines, "\\color default", i)
        if i == -1:
            return
        lines[i] = replace(lines[i], "\\color default",
                           "\\color inherit")


##
# Add \end_header
#
def add_end_header(header):
    header.append("\\end_header");


def rm_end_header(lines):
    i = find_token(lines, "\\end_header", 0)
    if i == -1:
        return
    del lines[i]


##
# \SpecialChar ~ -> \InsetSpace ~
#
def convert_spaces(lines):
    for i in range(len(lines)):
        lines[i] = replace(lines[i],"\\SpecialChar ~","\\InsetSpace ~")


def revert_spaces(lines):
    for i in range(len(lines)):
        lines[i] = replace(lines[i],"\\InsetSpace ~", "\\SpecialChar ~")


##
# BibTeX changes
#
def convert_bibtex(lines):
    for i in range(len(lines)):
        lines[i] = replace(lines[i],"\\begin_inset LatexCommand \\BibTeX",
                                  "\\begin_inset LatexCommand \\bibtex")


def revert_bibtex(lines):
    for i in range(len(lines)):
        lines[i] = replace(lines[i], "\\begin_inset LatexCommand \\bibtex",
                                  "\\begin_inset LatexCommand \\BibTeX")


##
# Remove \lyxparent
#
def remove_insetparent(lines):
    i = 0
    while 1:
        i = find_token(lines, "\\begin_inset LatexCommand \\lyxparent", i)
        if i == -1:
            break
        del lines[i:i+3]


##
#  Inset External
#
def convert_external(lines):
    external_rexp = re.compile(r'\\begin_inset External ([^,]*),"([^"]*)",')
    external_header = "\\begin_inset External"
    i = 0
    while 1:
        i = find_token(lines, external_header, i)
        if i == -1:
            break
        look = external_rexp.search(lines[i])
        args = ['','']
        if look:
            args[0] = look.group(1)
            args[1] = look.group(2)
        #FIXME: if the previous search fails then warn

        if args[0] == "RasterImage":
            # Convert a RasterImage External Inset to a Graphics Inset.
            top = "\\begin_inset Graphics"
            if args[1]:
                filename = "\tfilename " + args[1]
            lines[i:i+1] = [top, filename]
            i = i + 1
        else:
            # Convert the old External Inset format to the new.
            top = external_header
            template = "\ttemplate " + args[0]
            if args[1]:
                filename = "\tfilename " + args[1]
                lines[i:i+1] = [top, template, filename]
                i = i + 2
            else:
                lines[i:i+1] = [top, template]
                i = i + 1


def revert_external_1(lines):
    external_header = "\\begin_inset External"
    i = 0
    while 1:
        i = find_token(lines, external_header, i)
        if i == -1:
            break

        template = split(lines[i+1])
        template.reverse()
        del lines[i+1]

        filename = split(lines[i+1])
        filename.reverse()
        del lines[i+1]

        params = split(lines[i+1])
        params.reverse()
        if lines[i+1]: del lines[i+1]

        lines[i] = lines[i] + " " + template[0]+ ', "' + filename[0] + '", " '+ join(params[1:]) + '"'
        i = i + 1


def revert_external_2(lines):
    draft_token = '\tdraft'
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset External', i)
        if i == -1:
            break
        j = find_end_of_inset(lines, i + 1)
        if j == -1:
            #this should not happen
            break
        k = find_token(lines, draft_token, i+1, j-1)
        if (k != -1 and len(draft_token) == len(lines[k])):
            del lines[k]
        i = j + 1


##
# Comment
#
def convert_comment(lines):
    i = 0
    comment = "\\layout Comment"
    while 1:
        i = find_token(lines, comment, i)
        if i == -1:
            return

        lines[i:i+1] = ["\\layout Standard","","",
                        "\\begin_inset Comment",
                        "collapsed true","",
                        "\\layout Standard"]
        i = i + 7

        while 1:
                old_i = i
            	i = find_token(lines, "\\layout", i)
                if i == -1:
                    i = len(lines) - 1
                    lines[i:i] = ["\\end_inset","",""]
                    return

                j = find_token(lines, '\\begin_deeper', old_i, i)
                if j == -1: j = i + 1
                k = find_token(lines, '\\begin_inset', old_i, i)
                if k == -1: k = i + 1

                if j < i and j < k:
                    i = j
                    del lines[i]
                    i = find_end_of( lines, i, "\\begin_deeper","\\end_deeper")
                    if i == -1:
                        #This case should not happen
                        #but if this happens deal with it greacefully adding
                        #the missing \end_deeper.
                        i = len(lines) - 1
                        lines[i:i] = ["\end_deeper",""]
                        return
                    else:
                        del lines[i]
                        continue

                if k < i:
                    i = k
                    i = find_end_of( lines, i, "\\begin_inset","\\end_inset")
                    if i == -1:
                        #This case should not happen
                        #but if this happens deal with it greacefully adding
                        #the missing \end_inset.
                        i = len(lines) - 1
                        lines[i:i] = ["\\end_inset","","","\\end_inset","",""]
                        return
                    else:
                        i = i + 1
                        continue

                if find(lines[i], comment) == -1:
                    lines[i:i] = ["\\end_inset"]
                    i = i + 1
                    break
                lines[i:i+1] = ["\\layout Standard"]
                i = i + 1


def revert_comment(lines):
    i = 0
    while 1:
        i = find_tokens(lines, ["\\begin_inset Comment", "\\begin_inset Greyedout"], i)

        if i == -1:
            return
        lines[i] = "\\begin_inset Note"
        i = i + 1


##
# Add \end_layout
#
def add_end_layout(lines):
    i = find_token(lines, '\\layout', 0)

    if i == -1:
        return

    i = i + 1
    struct_stack = ["\\layout"]

    while 1:
        i = find_tokens(lines, ["\\begin_inset", "\\end_inset", "\\layout",
                                "\\begin_deeper", "\\end_deeper", "\\the_end"], i)

        token = split(lines[i])[0]

        if token == "\\begin_inset":
            struct_stack.append(token)
            i = i + 1
            continue

        if token == "\\end_inset":
            tail = struct_stack.pop()
            if tail == "\\layout":
                lines.insert(i,"")
                lines.insert(i,"\\end_layout")
                i = i + 2
                #Check if it is the correct tag
                struct_stack.pop()
            i = i + 1
            continue

        if token == "\\layout":
            tail = struct_stack.pop()
            if tail == token:
                lines.insert(i,"")
                lines.insert(i,"\\end_layout")
                i = i + 3
            else:
                struct_stack.append(tail)
                i = i + 1
            struct_stack.append(token)
            continue

        if token == "\\begin_deeper":
            lines.insert(i,"")
            lines.insert(i,"\\end_layout")
            i = i + 3
            struct_stack.append(token)
            continue

        if token == "\\end_deeper":
            if struct_stack[-1] == '\\layout':
                lines.insert(i, '\\end_layout')
                i = i + 1
                struct_stack.pop()
            i = i + 1
            continue

        #case \end_document
        lines.insert(i, "")
        lines.insert(i, "\\end_layout")
        return


def rm_end_layout(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\end_layout', i)

        if i == -1:
            return

        del lines[i]


##
# Handle change tracking keywords
#
def insert_tracking_changes(lines):
    i = find_token(lines, "\\tracking_changes", 0)
    if i == -1:
        lines.append("\\tracking_changes 0")

def rm_tracking_changes(lines):
    i = find_token(lines, "\\author", 0)
    if i != -1:
        del lines[i]

    i = find_token(lines, "\\tracking_changes", 0)
    if i == -1:
        return
    del lines[i]


def rm_body_changes(lines):
    i = 0
    while 1:
        i = find_token(lines, "\\change_", i)
        if i == -1:
            return

        del lines[i]


##
# \layout -> \begin_layout
#
def layout2begin_layout(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\layout', i)
        if i == -1:
            return

        lines[i] = replace(lines[i], '\\layout', '\\begin_layout')
        i = i + 1


def begin_layout2layout(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\begin_layout', i)
        if i == -1:
            return

        lines[i] = replace(lines[i], '\\begin_layout', '\\layout')
        i = i + 1


##
# valignment="center" -> valignment="middle"
#
def convert_valignment_middle(lines, start, end):
    for i in range(start, end):
        if re.search('^<(column|cell) .*valignment="center".*>$', lines[i]):
            lines[i] = replace(lines[i], 'valignment="center"', 'valignment="middle"')


def convert_table_valignment_middle(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset  Tabular', i)
        if i == -1:
            return
        j = find_end_of_inset(lines, i + 1)
        if j == -1:
            #this should not happen
            convert_valignment_middle(lines, i + 1, len(lines))
            return
        convert_valignment_middle(lines, i + 1, j)
        i = j + 1


def revert_table_valignment_middle(lines, start, end):
    for i in range(start, end):
        if re.search('^<(column|cell) .*valignment="middle".*>$', lines[i]):
            lines[i] = replace(lines[i], 'valignment="middle"', 'valignment="center"')


def revert_valignment_middle(lines):
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset  Tabular', i)
        if i == -1:
            return
        j = find_end_of_inset(lines, i + 1)
        if j == -1:
            #this should not happen
            revert_table_valignment_middle(lines, i + 1, len(lines))
            return
        revert_table_valignment_middle(lines, i + 1, j)
        i = j + 1


##
#  \the_end -> \end_document
#
def convert_end_document(lines):
    i = find_token(lines, "\\the_end", 0)
    if i == -1:
        lines.append("\\end_document")
        return
    lines[i] = "\\end_document"


def revert_end_document(lines):
    i = find_token(lines, "\\end_document", 0)
    if i == -1:
        lines.append("\\the_end")
        return
    lines[i] = "\\the_end"


##
# Convert line and page breaks
# Old:
#\layout Standard
#\line_top \line_bottom \pagebreak_top \pagebreak_bottom \added_space_top xxx \added_space_bottom yyy
#0
#
# New:
#\begin layout Standard
#
#\newpage 
#
#\lyxline
#\begin_inset VSpace xxx
#\end_inset
#
#\end_layout
#\begin_layout Standard
#
#0
#\end_layout
#\begin_layout Standard
#
#\begin_inset VSpace xxx
#\end_inset
#\lyxline 
#
#\newpage
#
#\end_layout
def convert_breaks(lines):    
    i = 0
    while 1:
        i = find_token(lines, "\\begin_layout", i)
        if i == -1:
            return
        i = i + 1
        line_top   = find(lines[i],"\\line_top")
        line_bot   = find(lines[i],"\\line_bottom")
        pb_top     = find(lines[i],"\\pagebreak_top")
        pb_bot     = find(lines[i],"\\pagebreak_bottom")
        vspace_top = find(lines[i],"\\added_space_top")
        vspace_bot = find(lines[i],"\\added_space_bottom")

        if line_top == -1 and line_bot == -1 and pb_bot == -1 and pb_top == -1 and vspace_top == -1 and vspace_bot == -1:
            continue

        for tag in "\\line_top", "\\line_bottom", "\\pagebreak_top", "\\pagebreak_bottom":
            lines[i] = replace(lines[i], tag, "")

        if vspace_top != -1:
            # the position could be change because of the removal of other
            # paragraph properties above
            vspace_top = find(lines[i],"\\added_space_top")
            tmp_list = split(lines[i][vspace_top:])
            vspace_top_value = tmp_list[1]
            lines[i] = lines[i][:vspace_top] + join(tmp_list[2:])

        if vspace_bot != -1:
            # the position could be change because of the removal of other
            # paragraph properties above
            vspace_bot = find(lines[i],"\\added_space_bottom")
            tmp_list = split(lines[i][vspace_bot:])
            vspace_bot_value = tmp_list[1]
            lines[i] = lines[i][:vspace_bot] + join(tmp_list[2:])

        lines[i] = strip(lines[i])
        i = i + 1

        #  Create an empty paragraph for line and page break that belong
        # above the paragraph
        if pb_top !=-1 or line_top != -1 or vspace_bot != -1:
            
            paragraph_above = ['','\\begin_layout Standard','','']

            if pb_top != -1:
                paragraph_above.extend(['\\newpage ',''])

            if vspace_top != -1:
                paragraph_above.extend(['\\begin_inset VSpace ' + vspace_top_value,'\\end_inset','',''])

            if line_top != -1:
                paragraph_above.extend(['\\lyxline ',''])

            paragraph_above.extend(['\\end_layout',''])

            #inset new paragraph above the current paragraph
            lines[i-2:i-2] = paragraph_above
            i = i + len(paragraph_above)

        # Ensure that nested style are converted later.
        k = find_end_of(lines, i, "\\begin_layout", "\\end_layout")

        if k == -1:
            return

        if pb_top !=-1 or line_top != -1 or vspace_bot != -1:
            
            paragraph_bellow = ['','\\begin_layout Standard','','']

            if line_bot != -1:
                paragraph_bellow.extend(['\\lyxline ',''])

            if vspace_bot != -1:
                paragraph_bellow.extend(['\\begin_inset VSpace ' + vspace_bot_value,'\\end_inset','',''])

            if pb_bot != -1:
                paragraph_bellow.extend(['\\newpage ',''])

            paragraph_bellow.extend(['\\end_layout',''])

            #inset new paragraph above the current paragraph
            lines[k + 1: k + 1] = paragraph_bellow


##
#  Notes
#
def convert_note(lines):
    i = 0
    while 1:
        i = find_tokens(lines, ["\\begin_inset Note",
                                "\\begin_inset Comment",
                                "\\begin_inset Greyedout"], i)
        if i == -1:
            break

        lines[i] = lines[i][0:13] + 'Note ' + lines[i][13:]
        i = i + 1


def revert_note(lines):
    note_header = "\\begin_inset Note "
    i = 0
    while 1:
        i = find_token(lines, note_header, i)
        if i == -1:
            break

        lines[i] = "\\begin_inset " + lines[i][len(note_header):]
        i = i + 1


##
# Box
#
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


def revert_box(lines):
    box_header = "\\begin_inset Box "
    i = 0
    while 1:
        i = find_token(lines, box_header, i)
        if i == -1:
            break

        lines[i] = "\\begin_inset " + lines[i][len(box_header):]
        i = i + 1


##
# Collapse
#
def convert_collapsable(lines, opt):
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
        # If, however, we find a line starting '\begin_layout'
        # (_always_ present) then break with a warning message
        i = i + 1
        while 1:
            if (lines[i] == "collapsed false"):
                lines[i] = "status open"
                break
            elif (lines[i] == "collapsed true"):
                lines[i] = "status collapsed"
                break
            elif (lines[i][:13] == "\\begin_layout"):
                opt.warning("Malformed LyX file.")
                break
            i = i + 1

        i = i + 1


def revert_collapsable(lines, opt):
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
                opt.warning("Malformed LyX file.")
                break
            i = i + 1

        i = i + 1


##
#  ERT
#
def convert_ert(lines, opt):
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
            if (lines[i] == "status Open"):
                lines[i] = "status open"
                break
            elif (lines[i] == "status Collapsed"):
                lines[i] = "status collapsed"
                break
            elif (lines[i] == "status Inlined"):
                lines[i] = "status inlined"
                break
            elif (lines[i][:13] == "\\begin_layout"):
                opt.warning("Malformed LyX file.")
                break
            i = i + 1

        i = i + 1


def revert_ert(lines, opt):
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
                opt.warning("Malformed LyX file.")
                break
            i = i + 1

        i = i + 1


##
# Minipages
#
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

        if lines[i][:9] == "collapsed":
            if lines[i][9:] == "true":
		status = "collapsed"
            else:
		status = "open"
            del lines[i]
        else:
	    status = "collapsed"

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
        lines.insert(i, 'status ' + status)
        i = i + 1


# -------------------------------------------------------------------------------------------
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


def convert_vspace(header, lines, opt):

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
	    opt.warning("Malformed LyX file: Missing '\\end_inset'.")
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


def convert_frameless_box(lines, opt):
    pos = ['t', 'c', 'b']
    inner_pos = ['c', 't', 'b', 's']
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset Frameless', i)
        if i == -1:
            return
	j = find_end_of_inset(lines, i)
	if j == -1:
	    opt.warning("Malformed LyX file: Missing '\\end_inset'.")
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
            if params['use_parbox'] == '1':
                lines[i] = lines[i] + '}{'
            else:
                lines[i] = lines[i] + '}'
	    i = i + 1
	    lines[i:i] = ['', '\\end_inset']
	    i = i + 2
	    j = find_end_of_inset(lines, i)
	    if j == -1:
		opt.warning("Malformed LyX file: Missing '\\end_inset'.")
		break
            lines[j-1:j-1] = ['\\begin_inset ERT', 'status ' + params['collapsed'],
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

##
# Convert jurabib
#

def convert_jurabib(header, opt):
    i = find_token(header, '\\use_numerical_citations', 0)
    if i == -1:
        opt.warning("Malformed lyx file: Missing '\\use_numerical_citations'.")
        return
    header.insert(i + 1, '\\use_jurabib 0')


def revert_jurabib(header, opt):
    i = find_token(header, '\\use_jurabib', 0)
    if i == -1:
        opt.warning("Malformed lyx file: Missing '\\use_jurabib'.")
        return
    if get_value(header, '\\use_jurabib', 0) != "0":
        opt.warning("Conversion of '\\use_jurabib = 1' not yet implemented.")
        # Don't remove '\\use_jurabib' so that people will get warnings by lyx
        return
    del header[i]

##
# Convert bibtopic
#

def convert_bibtopic(header, opt):
    i = find_token(header, '\\use_jurabib', 0)
    if i == -1:
        opt.warning("Malformed lyx file: Missing '\\use_jurabib'.")
        return
    header.insert(i + 1, '\\use_bibtopic 0')


def revert_bibtopic(header, opt):
    i = find_token(header, '\\use_bibtopic', 0)
    if i == -1:
        opt.warning("Malformed lyx file: Missing '\\use_bibtopic'.")
        return
    if get_value(header, '\\use_bibtopic', 0) != "0":
        opt.warning("Conversion of '\\use_bibtopic = 1' not yet implemented.")
        # Don't remove '\\use_jurabib' so that people will get warnings by lyx
    del header[i]

##
# Sideway Floats
#

def convert_float(lines, opt):
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset Float', i)
        if i == -1:
            return
        # Seach for a line starting 'wide'
        # If, however, we find a line starting '\begin_layout'
        # (_always_ present) then break with a warning message
        i = i + 1
        while 1:
            if (lines[i][:4] == "wide"):
                lines.insert(i + 1, 'sideways false')
                break
            elif (lines[i][:13] == "\\begin_layout"):
                opt.warning("Malformed lyx file.")
                break
            i = i + 1
        i = i + 1


def revert_float(lines, opt):
    i = 0
    while 1:
        i = find_token(lines, '\\begin_inset Float', i)
        if i == -1:
            return
        j = find_end_of_inset(lines, i)
        if j == -1:
            opt.warning("Malformed lyx file: Missing '\\end_inset'.")
            i = i + 1
            continue
        if get_value(lines, 'sideways', i, j) != "false":
            opt.warning("Conversion of 'sideways true' not yet implemented.")
            # Don't remove 'sideways' so that people will get warnings by lyx
            i = i + 1
            continue
        del_token(lines, 'sideways', i, j)
        i = i + 1


def convert_graphics(lines, opt):
    """ Add extension to filenames of insetgraphics if necessary.
    """
    i = 0
    while 1:
        i = find_token(lines, "\\begin_inset Graphics", i)
        if i == -1:
            return

	j = find_token2(lines, "filename", i)
        if j == -1:
            return
        i = i + 1
	filename = split(lines[j])[1]
	absname = os.path.normpath(os.path.join(opt.dir, filename))
	if opt.input == stdin and not os.path.isabs(filename):
	    # We don't know the directory and cannot check the file.
	    # We could use a heuristic and take the current directory,
	    # and we could try to find out if filename has an extension,
	    # but that would be just guesses and could be wrong.
	    opt.warning("""Warning: Can not determine whether file
         %s
         needs an extension when reading from standard input.
         You may need to correct the file manually or run
         lyx2lyx again with the .lyx file as commandline argument.""" % filename)
	    continue
	# This needs to be the same algorithm as in pre 233 insetgraphics
	if access(absname, F_OK):
	    continue
	if access(absname + ".ps", F_OK):
	    lines[j] = replace(lines[j], filename, filename + ".ps")
	    continue
	if access(absname + ".eps", F_OK):
	    lines[j] = replace(lines[j], filename, filename + ".eps")


##
# Convert firstname and surname from styles -> char styles
#
def convert_names(lines, opt):
    """ Convert in the docbook backend from firstname and surname style
    to charstyles.
    """
    if opt.backend != "docbook":
        return

    i = 0

    while 1:
        i = find_token(lines, "\\begin_layout Author", i)
        if i == -1:
            return

        i = i + 1
        while lines[i] == "":
            i = i + 1

        if lines[i][:11] != "\\end_layout" or lines[i+2][:13] != "\\begin_deeper":
            i = i + 1
            continue

        k = i
        i = find_end_of( lines, i+3, "\\begin_deeper","\\end_deeper")
        if i == -1:
            # something is really wrong, abort
            opt.warning("Missing \\end_deeper, after style Author.")
            opt.warning("Aborted attempt to parse FirstName and Surname.")
            return
        firstname, surname = "", ""

        name = lines[k:i]

        j = find_token(name, "\\begin_layout FirstName", 0)
        if j != -1:
            j = j + 1
            while(name[j] != "\\end_layout"):
                firstname = firstname + name[j]
                j = j + 1

        j = find_token(name, "\\begin_layout Surname", 0)
        if j != -1:
            j = j + 1
            while(name[j] != "\\end_layout"):
                surname = surname + name[j]
                j = j + 1

        # delete name
        del lines[k+2:i+1]

        lines[k-1:k-1] = ["", "",
                          "\\begin_inset CharStyle Firstname",
                          "status inlined",
                          "",
                          "\\begin_layout Standard",
                          "",
                          "%s" % firstname,
                          "\end_layout",
                          "",
                          "\end_inset",
                          "",
                          "",
                          "\\begin_inset CharStyle Surname",
                          "status inlined",
                          "",
                          "\\begin_layout Standard",
                          "",
                          "%s" % surname,
                          "\\end_layout",
                          "",
                          "\\end_inset",
                          ""]


def revert_names(lines, opt):
    """ Revert in the docbook backend from firstname and surname char style
    to styles.
    """
    if opt.backend != "docbook":
        return


##
#    \use_natbib 1                       \cite_engine <style>
#    \use_numerical_citations 0     ->   where <style> is one of
#    \use_jurabib 0                      "basic", "natbib_authoryear",
#                                        "natbib_numerical" or "jurabib"
def convert_cite_engine(header, opt):
    a = find_token(header, "\\use_natbib", 0)
    if a == -1:
        opt.warning("Malformed lyx file: Missing '\\use_natbib'.")
        return

    b = find_token(header, "\\use_numerical_citations", 0)
    if b == -1 or b != a+1:
        opt.warning("Malformed lyx file: Missing '\\use_numerical_citations'.")
        return

    c = find_token(header, "\\use_jurabib", 0)
    if c == -1 or c != b+1:
        opt.warning("Malformed lyx file: Missing '\\use_jurabib'.")
        return

    use_natbib = int(split(header[a])[1])
    use_numerical_citations = int(split(header[b])[1])
    use_jurabib = int(split(header[c])[1])

    cite_engine = "basic"
    if use_natbib:
        if use_numerical_citations:
            cite_engine = "natbib_numerical"
        else:
             cite_engine = "natbib_authoryear"
    elif use_jurabib:
        cite_engine = "jurabib"

    del header[a:c+1]
    header.insert(a, "\\cite_engine " + cite_engine)


def revert_cite_engine(header, opt):
    i = find_token(header, "\\cite_engine", 0)
    if i == -1:
        opt.warning("Malformed lyx file: Missing '\\cite_engine'.")
        return

    cite_engine = split(header[i])[1]

    use_natbib = '0'
    use_numerical = '0'
    use_jurabib = '0'
    if cite_engine == "natbib_numerical":
        use_natbib = '1'
        use_numerical = '1'
    elif cite_engine == "natbib_authoryear":
        use_natbib = '1'
    elif cite_engine == "jurabib":
        use_jurabib = '1'

    del header[i]
    header.insert(i, "\\use_jurabib " + use_jurabib)
    header.insert(i, "\\use_numerical_citations " + use_numerical)
    header.insert(i, "\\use_natbib " + use_natbib)


##
# Paper package
#
def convert_paperpackage(header, opt):
    i = find_token(header, "\\paperpackage", 0)
    if i == -1:
        opt.warning("Malformed lyx file: Missing '\\paperpackage'.")
        return

    packages = {'a4':'none', 'a4wide':'a4', 'widemarginsa4':'a4wide'}
    paperpackage = split(header[i])[1]
    header[i] = replace(header[i], paperpackage, packages[paperpackage])


def revert_paperpackage(header, opt):
    i = find_token(header, "\\paperpackage", 0)
    if i == -1:
        opt.warning("Malformed lyx file: Missing '\\paperpackage'.")
        return

    packages = {'none':'a4', 'a4':'a4wide', 'a4wide':'widemarginsa4',
                'widemarginsa4':''}
    paperpackage = split(header[i])[1]
    header[i] = replace(header[i], paperpackage, packages[paperpackage])


##
# Bullets
#
def convert_bullets(header, opt):
    i = 0
    while 1:
        i = find_token(header, "\\bullet", i)
        if i == -1:
            return
        if header[i][:12] == '\\bulletLaTeX':
            header[i] = header[i] + ' ' + strip(header[i+1])
            n = 3
        else:
            header[i] = header[i] + ' ' + strip(header[i+1]) +\
                        ' ' + strip(header[i+2]) + ' ' + strip(header[i+3])
            n = 5
        del header[i+1:i + n]
        i = i + 1


def revert_bullets(header, opt):
    i = 0
    while 1:
        i = find_token(header, "\\bullet", i)
        if i == -1:
            return
        if header[i][:12] == '\\bulletLaTeX':
            n = find(header[i], '"')
            if n == -1:
                opt.warning("Malformed header.")
                return
            else:
                header[i:i+1] = [header[i][:n-1],'\t' + header[i][n:], '\\end_bullet']
            i = i + 3
        else:
            frag = split(header[i])
            if len(frag) != 5:
                opt.warning("Malformed header.")
                return
            else:
                header[i:i+1] = [frag[0] + ' ' + frag[1],
                                 '\t' + frag[2],
                                 '\t' + frag[3],
                                 '\t' + frag[4],
                                 '\\end_bullet']
                i = i + 5


##
# \begin_header and \begin_document
#
def add_begin_header(header, opt):
    i = find_token(header, '\\lyxformat', 0)
    header.insert(i+1, '\\begin_header')
    header.insert(i+1, '\\begin_document')


def remove_begin_header(header, opt):
    i = find_token(header, "\\begin_document", 0)
    if i != -1:
        del header[i]
    i = find_token(header, "\\begin_header", 0)
    if i != -1:
        del header[i]


##
# \begin_body and \end_body
#
def add_begin_body(body, opt):
    body.insert(0, '\\begin_body')
    body.insert(1, '')
    i = find_token(body, "\\end_document", 0)
    body.insert(i, '\\end_body')

def remove_begin_body(body, opt):
    i = find_token(body, "\\begin_body", 0)
    if i != -1:
        del body[i]
        if not body[i]:
            del body[i]
    i = find_token(body, "\\end_body", 0)
    if i != -1:
        del body[i]


##
# \papersize
#
def normalize_papersize(header):
    i = find_token(header, '\\papersize', 0)
    if i == -1:
        return

    tmp = split(header[i])
    if tmp[1] == "Default":
        header[i] = '\\papersize default'
        return
    if tmp[1] == "Custom":
        header[i] = '\\papersize custom'


def denormalize_papersize(header):
    i = find_token(header, '\\papersize', 0)
    if i == -1:
        return

    tmp = split(header[i])
    if tmp[1] == "custom":
        header[i] = '\\papersize Custom'


##
# Strip spaces at end of command line
#
def strip_end_space(body):
    for i in range(len(body)):
        if body[i][:1] == '\\':
            body[i] = strip(body[i])


##
# Convertion hub
#
def convert(file):
    if file.format < 223:
        insert_tracking_changes(file.header)
        add_end_header(file.header)
        remove_color_default(file.body)
	convert_spaces(file.body)
	convert_bibtex(file.body)
	remove_insetparent(file.body)
	file.format = 223
    if file.end_format == file.format: return

    if file.format < 224:
	convert_external(file.body)
	convert_comment(file.body)
	file.format = 224
    if file.end_format == file.format: return

    if file.format < 225:
	add_end_layout(file.body)
	layout2begin_layout(file.body)
	convert_end_document(file.body)
	convert_table_valignment_middle(file.body)
	convert_breaks(file.body)
	file.format = 225
    if file.end_format == file.format: return

    if file.format < 226:
	convert_note(file.body)
	file.format = 226
    if file.end_format == file.format: return

    if file.format < 227:
	convert_box(file.body)
	file.format = 227
    if file.end_format == file.format: return

    if file.format < 228:
	convert_collapsable(file.body, file)
	convert_ert(file.body, file)
	file.format = 228
    if file.end_format == file.format: return

    if file.format < 229:
	convert_minipage(file.body)
	file.format = 229
    if file.end_format == file.format: return

    if file.format < 230:
        convert_jurabib(file.header, file)
	file.format = 230
    if file.end_format == file.format: return

    if file.format < 231:
        convert_float(file.body, file)
	file.format = 231
    if file.end_format == file.format: return

    if file.format < 232:
        convert_bibtopic(file.header, file)
	file.format = 232
    if file.end_format == file.format: return

    if file.format < 233:
        convert_graphics(file.body, file)
        convert_names(file.body, file)
	file.format = 233
    if file.end_format == file.format: return

    if file.format < 234:
        convert_cite_engine(file.header, file)
	file.format = 234
    if file.end_format == file.format: return

    if file.format < 235:
        convert_paperpackage(file.header, file)
	file.format = 235
    if file.end_format == file.format: return

    if file.format < 236:
        convert_bullets(file.header, file)
        add_begin_header(file.header, file)
        add_begin_body(file.body, file)
        normalize_papersize(file.header)
        strip_end_space(file.body)
        file.format = 236

def revert(file):
    if file.format > 235:
        denormalize_papersize(file.header)
        remove_begin_body(file.body, file)
        remove_begin_header(file.header, file)
        revert_bullets(file.header, file)
        file.format = 235
    if file.end_format == file.format: return

    if file.format > 234:
        revert_paperpackage(file.header, file)
	file.format = 234
    if file.end_format == file.format: return

    if file.format > 233:
        revert_cite_engine(file.header, file)
	file.format = 233
    if file.end_format == file.format: return

    if file.format > 232:
        revert_names(file.body, file)
	file.format = 232
    if file.end_format == file.format: return

    if file.format > 231:
        revert_bibtopic(file.header, file)
	file.format = 231
    if file.end_format == file.format: return

    if file.format > 230:
        revert_float(file.body, file)
	file.format = 230
    if file.end_format == file.format: return

    if file.format > 229:
        revert_jurabib(file.header, file)
	file.format = 229
    if file.end_format == file.format: return

    if file.format > 228:
	file.format = 228
    if file.end_format == file.format: return

    if file.format > 227:
	revert_collapsable(file.body, file)
	revert_ert(file.body, file)
	file.format = 227
    if file.end_format == file.format: return

    if file.format > 226:
	revert_box(file.body)
	revert_external_2(file.body)
	file.format = 226
    if file.end_format == file.format: return

    if file.format > 225:
	revert_note(file.body)
	file.format = 225
    if file.end_format == file.format: return

    if file.format > 224:
	rm_end_layout(file.body)
	begin_layout2layout(file.body)
	revert_end_document(file.body)
	revert_valignment_middle(file.body)
	convert_vspace(file.header, file.body, file)
	convert_frameless_box(file.body, file)
    if file.end_format == file.format: return

    if file.format > 223:
	revert_external_2(file.body)
	revert_comment(file.body)
	file.format = 223
    if file.end_format == file.format: return

    if file.format > 221:
	rm_end_header(file.header)
	revert_spaces(file.body)
	revert_bibtex(file.body)
	rm_tracking_changes(file.header)
	rm_body_changes(file.body)
	file.format = 221


if __name__ == "__main__":
    pass
