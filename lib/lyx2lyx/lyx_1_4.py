# This file is part of lyx2lyx
# -*- coding: iso-8859-1 -*-
# Copyright (C) 2002 Dekel Tsur <dekel@lyx.org>
# Copyright (C) 2002-2004 José Matos <jamatos@lyx.org>
# Copyright (C) 2004-2005 Georg Baum <Georg.Baum@post.rwth-aachen.de>
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
			 find_tokens, find_end_of, find_token2, find_re
from sys import stdin
from string import replace, split, find, strip, join

from lyx_0_12 import update_latexaccents

##
# Remove \color default
#
def remove_color_default(file):
    i = 0
    while 1:
        i = find_token(file.body, "\\color default", i)
        if i == -1:
            return
        file.body[i] = replace(file.body[i], "\\color default",
                           "\\color inherit")


##
# Add \end_header
#
def add_end_header(file):
    file.header.append("\\end_header");


def rm_end_header(file):
    i = find_token(file.header, "\\end_header", 0)
    if i == -1:
        return
    del file.header[i]


##
# \SpecialChar ~ -> \InsetSpace ~
#
def convert_spaces(file):
    for i in range(len(file.body)):
        file.body[i] = replace(file.body[i],"\\SpecialChar ~","\\InsetSpace ~")


def revert_spaces(file):
    for i in range(len(file.body)):
        file.body[i] = replace(file.body[i],"\\InsetSpace ~", "\\SpecialChar ~")


##
# equivalent to lyx::support::escape()
#
def lyx_support_escape(lab):
    hexdigit = ['0', '1', '2', '3', '4', '5', '6', '7',
                '8', '9', 'A', 'B', 'C', 'D', 'E', 'F']
    enc = ""
    for c in lab:
        o = ord(c)
        if o >= 128 or c == '=' or c == '%':
            enc = enc + '='
            enc = enc + hexdigit[o >> 4]
            enc = enc + hexdigit[o & 15]
        else:
            enc = enc + c
    return enc;


##
# \begin_inset LatexCommand \eqref -> ERT
#
def revert_eqref(file):
    regexp = re.compile(r'^\\begin_inset\s+LatexCommand\s+\\eqref')
    i = 0
    while 1:
        i = find_re(file.body, regexp, i)
        if i == -1:
            break
        eqref = lyx_support_escape(regexp.sub("", file.body[i]))
        file.body[i:i+1] = ["\\begin_inset ERT", "status Collapsed", "",
                            "\\layout Standard", "", "\\backslash ",
                            "eqref" + eqref]
        i = i + 7


##
# BibTeX changes
#
def convert_bibtex(file):
    for i in range(len(file.body)):
        file.body[i] = replace(file.body[i],"\\begin_inset LatexCommand \\BibTeX",
                                  "\\begin_inset LatexCommand \\bibtex")


def revert_bibtex(file):
    for i in range(len(file.body)):
        file.body[i] = replace(file.body[i], "\\begin_inset LatexCommand \\bibtex",
                                  "\\begin_inset LatexCommand \\BibTeX")


##
# Remove \lyxparent
#
def remove_insetparent(file):
    i = 0
    while 1:
        i = find_token(file.body, "\\begin_inset LatexCommand \\lyxparent", i)
        if i == -1:
            break
        del file.body[i:i+3]


##
#  Inset External
#
def convert_external(file):
    external_rexp = re.compile(r'\\begin_inset External ([^,]*),"([^"]*)",')
    external_header = "\\begin_inset External"
    i = 0
    while 1:
        i = find_token(file.body, external_header, i)
        if i == -1:
            break
        look = external_rexp.search(file.body[i])
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
            file.body[i:i+1] = [top, filename]
            i = i + 1
        else:
            # Convert the old External Inset format to the new.
            top = external_header
            template = "\ttemplate " + args[0]
            if args[1]:
                filename = "\tfilename " + args[1]
                file.body[i:i+1] = [top, template, filename]
                i = i + 2
            else:
                file.body[i:i+1] = [top, template]
                i = i + 1


def revert_external_1(file):
    external_header = "\\begin_inset External"
    i = 0
    while 1:
        i = find_token(file.body, external_header, i)
        if i == -1:
            break

        template = split(file.body[i+1])
        template.reverse()
        del file.body[i+1]

        filename = split(file.body[i+1])
        filename.reverse()
        del file.body[i+1]

        params = split(file.body[i+1])
        params.reverse()
        if file.body[i+1]: del file.body[i+1]

        file.body[i] = file.body[i] + " " + template[0]+ ', "' + filename[0] + '", " '+ join(params[1:]) + '"'
        i = i + 1


def revert_external_2(file):
    draft_token = '\tdraft'
    i = 0
    while 1:
        i = find_token(file.body, '\\begin_inset External', i)
        if i == -1:
            break
        j = find_end_of_inset(file.body, i + 1)
        if j == -1:
            #this should not happen
            break
        k = find_token(file.body, draft_token, i+1, j-1)
        if (k != -1 and len(draft_token) == len(file.body[k])):
            del file.body[k]
        i = j + 1


##
# Comment
#
def convert_comment(file):
    i = 0
    comment = "\\layout Comment"
    while 1:
        i = find_token(file.body, comment, i)
        if i == -1:
            return

        file.body[i:i+1] = ["\\layout Standard","","",
                        "\\begin_inset Comment",
                        "collapsed true","",
                        "\\layout Standard"]
        i = i + 7

        while 1:
                old_i = i
            	i = find_token(file.body, "\\layout", i)
                if i == -1:
                    i = len(file.body) - 1
                    file.body[i:i] = ["\\end_inset","",""]
                    return

                j = find_token(file.body, '\\begin_deeper', old_i, i)
                if j == -1: j = i + 1
                k = find_token(file.body, '\\begin_inset', old_i, i)
                if k == -1: k = i + 1

                if j < i and j < k:
                    i = j
                    del file.body[i]
                    i = find_end_of( file.body, i, "\\begin_deeper","\\end_deeper")
                    if i == -1:
                        #This case should not happen
                        #but if this happens deal with it greacefully adding
                        #the missing \end_deeper.
                        i = len(file.body) - 1
                        file.body[i:i] = ["\end_deeper",""]
                        return
                    else:
                        del file.body[i]
                        continue

                if k < i:
                    i = k
                    i = find_end_of( file.body, i, "\\begin_inset","\\end_inset")
                    if i == -1:
                        #This case should not happen
                        #but if this happens deal with it greacefully adding
                        #the missing \end_inset.
                        i = len(file.body) - 1
                        file.body[i:i] = ["\\end_inset","","","\\end_inset","",""]
                        return
                    else:
                        i = i + 1
                        continue

                if find(file.body[i], comment) == -1:
                    file.body[i:i] = ["\\end_inset"]
                    i = i + 1
                    break
                file.body[i:i+1] = ["\\layout Standard"]
                i = i + 1


def revert_comment(file):
    i = 0
    while 1:
        i = find_tokens(file.body, ["\\begin_inset Comment", "\\begin_inset Greyedout"], i)

        if i == -1:
            return
        file.body[i] = "\\begin_inset Note"
        i = i + 1


##
# Add \end_layout
#
def add_end_layout(file):
    i = find_token(file.body, '\\layout', 0)

    if i == -1:
        return

    i = i + 1
    struct_stack = ["\\layout"]

    while 1:
        i = find_tokens(file.body, ["\\begin_inset", "\\end_inset", "\\layout",
                                "\\begin_deeper", "\\end_deeper", "\\the_end"], i)

        if i != -1:
            token = split(file.body[i])[0]
        else:
            file.warning("Truncated file.")
            i = len(file.body)
            file.body.insert(i, '\\the_end')
            token = ""

        if token == "\\begin_inset":
            struct_stack.append(token)
            i = i + 1
            continue

        if token == "\\end_inset":
            tail = struct_stack.pop()
            if tail == "\\layout":
                file.body.insert(i,"")
                file.body.insert(i,"\\end_layout")
                i = i + 2
                #Check if it is the correct tag
                struct_stack.pop()
            i = i + 1
            continue

        if token == "\\layout":
            tail = struct_stack.pop()
            if tail == token:
                file.body.insert(i,"")
                file.body.insert(i,"\\end_layout")
                i = i + 3
            else:
                struct_stack.append(tail)
                i = i + 1
            struct_stack.append(token)
            continue

        if token == "\\begin_deeper":
            file.body.insert(i,"")
            file.body.insert(i,"\\end_layout")
            i = i + 3
            struct_stack.append(token)
            continue

        if token == "\\end_deeper":
            if struct_stack[-1] == '\\layout':
                file.body.insert(i, '\\end_layout')
                i = i + 1
                struct_stack.pop()
            i = i + 1
            continue

        #case \end_document
        file.body.insert(i, "")
        file.body.insert(i, "\\end_layout")
        return


def rm_end_layout(file):
    i = 0
    while 1:
        i = find_token(file.body, '\\end_layout', i)

        if i == -1:
            return

        del file.body[i]


##
# Handle change tracking keywords
#
def insert_tracking_changes(file):
    i = find_token(file.header, "\\tracking_changes", 0)
    if i == -1:
        file.header.append("\\tracking_changes 0")


def rm_tracking_changes(file):
    i = find_token(file.header, "\\author", 0)
    if i != -1:
        del file.header[i]

    i = find_token(file.header, "\\tracking_changes", 0)
    if i == -1:
        return
    del file.header[i]


def rm_body_changes(file):
    i = 0
    while 1:
        i = find_token(file.body, "\\change_", i)
        if i == -1:
            return

        del file.body[i]


##
# \layout -> \begin_layout
#
def layout2begin_layout(file):
    i = 0
    while 1:
        i = find_token(file.body, '\\layout', i)
        if i == -1:
            return

        file.body[i] = replace(file.body[i], '\\layout', '\\begin_layout')
        i = i + 1


def begin_layout2layout(file):
    i = 0
    while 1:
        i = find_token(file.body, '\\begin_layout', i)
        if i == -1:
            return

        file.body[i] = replace(file.body[i], '\\begin_layout', '\\layout')
        i = i + 1


##
# valignment="center" -> valignment="middle"
#
def convert_valignment_middle(body, start, end):
    for i in range(start, end):
        if re.search('^<(column|cell) .*valignment="center".*>$', body[i]):
            body[i] = replace(body[i], 'valignment="center"', 'valignment="middle"')


def convert_table_valignment_middle(file):
    regexp = re.compile(r'^\\begin_inset\s+Tabular')
    i = 0
    while 1:
        i = find_re(file.body, regexp, i)
        if i == -1:
            return
        j = find_end_of_inset(file.body, i + 1)
        if j == -1:
            #this should not happen
            convert_valignment_middle(file.body, i + 1, len(file.body))
            return
        convert_valignment_middle(file.body, i + 1, j)
        i = j + 1


def revert_table_valignment_middle(body, start, end):
    for i in range(start, end):
        if re.search('^<(column|cell) .*valignment="middle".*>$', body[i]):
            body[i] = replace(body[i], 'valignment="middle"', 'valignment="center"')


def revert_valignment_middle(file):
    regexp = re.compile(r'^\\begin_inset\s+Tabular')
    i = 0
    while 1:
        i = find_re(file.body, regexp, i)
        if i == -1:
            return
        j = find_end_of_inset(file.body, i + 1)
        if j == -1:
            #this should not happen
            revert_table_valignment_middle(file.body, i + 1, len(file.body))
            return
        revert_table_valignment_middle(file.body, i + 1, j)
        i = j + 1


##
#  \the_end -> \end_document
#
def convert_end_document(file):
    i = find_token(file.body, "\\the_end", 0)
    if i == -1:
        file.body.append("\\end_document")
        return
    file.body[i] = "\\end_document"


def revert_end_document(file):
    i = find_token(file.body, "\\end_document", 0)
    if i == -1:
        file.body.append("\\the_end")
        return
    file.body[i] = "\\the_end"


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
def convert_breaks(file):
    par_params = ('added_space_bottom', 'added_space_top', 'align',
                 'labelwidthstring', 'line_bottom', 'line_top', 'noindent',
                 'pagebreak_bottom', 'pagebreak_top', 'paragraph_spacing',
                 'start_of_appendix')
    i = 0
    while 1:
        i = find_token(file.body, "\\begin_layout", i)
        if i == -1:
            return
        i = i + 1

        # Merge all paragraph parameters into a single line
        # We cannot check for '\\' only because paragraphs may start e.g.
        # with '\\backslash'
        while file.body[i + 1][:1] == '\\' and split(file.body[i + 1][1:])[0] in par_params:
            file.body[i] = file.body[i + 1] + ' ' + file.body[i]
            del file.body[i+1]

        line_top   = find(file.body[i],"\\line_top")
        line_bot   = find(file.body[i],"\\line_bottom")
        pb_top     = find(file.body[i],"\\pagebreak_top")
        pb_bot     = find(file.body[i],"\\pagebreak_bottom")
        vspace_top = find(file.body[i],"\\added_space_top")
        vspace_bot = find(file.body[i],"\\added_space_bottom")

        if line_top == -1 and line_bot == -1 and pb_bot == -1 and pb_top == -1 and vspace_top == -1 and vspace_bot == -1:
            continue

        for tag in "\\line_top", "\\line_bottom", "\\pagebreak_top", "\\pagebreak_bottom":
            file.body[i] = replace(file.body[i], tag, "")

        if vspace_top != -1:
            # the position could be change because of the removal of other
            # paragraph properties above
            vspace_top = find(file.body[i],"\\added_space_top")
            tmp_list = split(file.body[i][vspace_top:])
            vspace_top_value = tmp_list[1]
            file.body[i] = file.body[i][:vspace_top] + join(tmp_list[2:])

        if vspace_bot != -1:
            # the position could be change because of the removal of other
            # paragraph properties above
            vspace_bot = find(file.body[i],"\\added_space_bottom")
            tmp_list = split(file.body[i][vspace_bot:])
            vspace_bot_value = tmp_list[1]
            file.body[i] = file.body[i][:vspace_bot] + join(tmp_list[2:])

        file.body[i] = strip(file.body[i])
        i = i + 1

        #  Create an empty paragraph for line and page break that belong
        # above the paragraph
        if pb_top !=-1 or line_top != -1 or vspace_top != -1:

            paragraph_above = ['','\\begin_layout Standard','','']

            if pb_top != -1:
                paragraph_above.extend(['\\newpage ',''])

            if vspace_top != -1:
                paragraph_above.extend(['\\begin_inset VSpace ' + vspace_top_value,'\\end_inset','',''])

            if line_top != -1:
                paragraph_above.extend(['\\lyxline ',''])

            paragraph_above.extend(['\\end_layout',''])

            #inset new paragraph above the current paragraph
            file.body[i-2:i-2] = paragraph_above
            i = i + len(paragraph_above)

        # Ensure that nested style are converted later.
        k = find_end_of(file.body, i, "\\begin_layout", "\\end_layout")

        if k == -1:
            return

        if pb_bot !=-1 or line_bot != -1 or vspace_bot != -1:

            paragraph_below = ['','\\begin_layout Standard','','']

            if line_bot != -1:
                paragraph_below.extend(['\\lyxline ',''])

            if vspace_bot != -1:
                paragraph_below.extend(['\\begin_inset VSpace ' + vspace_bot_value,'\\end_inset','',''])

            if pb_bot != -1:
                paragraph_below.extend(['\\newpage ',''])

            paragraph_below.extend(['\\end_layout',''])

            #inset new paragraph above the current paragraph
            file.body[k + 1: k + 1] = paragraph_below


##
#  Notes
#
def convert_note(file):
    i = 0
    while 1:
        i = find_tokens(file.body, ["\\begin_inset Note",
                                "\\begin_inset Comment",
                                "\\begin_inset Greyedout"], i)
        if i == -1:
            break

        file.body[i] = file.body[i][0:13] + 'Note ' + file.body[i][13:]
        i = i + 1


def revert_note(file):
    note_header = "\\begin_inset Note "
    i = 0
    while 1:
        i = find_token(file.body, note_header, i)
        if i == -1:
            break

        file.body[i] = "\\begin_inset " + file.body[i][len(note_header):]
        i = i + 1


##
# Box
#
def convert_box(file):
    i = 0
    while 1:
        i = find_tokens(file.body, ["\\begin_inset Boxed",
                                "\\begin_inset Doublebox",
                                "\\begin_inset Frameless",
                                "\\begin_inset ovalbox",
                                "\\begin_inset Ovalbox",
                                "\\begin_inset Shadowbox"], i)
        if i == -1:
            break

        file.body[i] = file.body[i][0:13] + 'Box ' + file.body[i][13:]
        i = i + 1


def revert_box(file):
    box_header = "\\begin_inset Box "
    i = 0
    while 1:
        i = find_token(file.body, box_header, i)
        if i == -1:
            break

        file.body[i] = "\\begin_inset " + file.body[i][len(box_header):]
        i = i + 1


##
# Collapse
#
def convert_collapsable(file):
    i = 0
    while 1:
        i = find_tokens(file.body, ["\\begin_inset Box",
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
            if (file.body[i] == "collapsed false"):
                file.body[i] = "status open"
                break
            elif (file.body[i] == "collapsed true"):
                file.body[i] = "status collapsed"
                break
            elif (file.body[i][:13] == "\\begin_layout"):
                file.warning("Malformed LyX file: Missing 'collapsed'.")
                break
            i = i + 1

        i = i + 1


def revert_collapsable(file):
    i = 0
    while 1:
        i = find_tokens(file.body, ["\\begin_inset Box",
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
            if (file.body[i] == "status open"):
                file.body[i] = "collapsed false"
                break
            elif (file.body[i] == "status collapsed" or
                  file.body[i] == "status inlined"):
                file.body[i] = "collapsed true"
                break
            elif (file.body[i][:13] == "\\begin_layout"):
                file.warning("Malformed LyX file: Missing 'status'.")
                break
            i = i + 1

        i = i + 1


##
#  ERT
#
def convert_ert(file):
    i = 0
    while 1:
        i = find_token(file.body, "\\begin_inset ERT", i)
        if i == -1:
            break

        # Seach for a line starting 'status'
        # If, however, we find a line starting '\begin_layout'
        # (_always_ present) then break with a warning message
        i = i + 1
        while 1:
            if (file.body[i] == "status Open"):
                file.body[i] = "status open"
                break
            elif (file.body[i] == "status Collapsed"):
                file.body[i] = "status collapsed"
                break
            elif (file.body[i] == "status Inlined"):
                file.body[i] = "status inlined"
                break
            elif (file.body[i][:13] == "\\begin_layout"):
                file.warning("Malformed LyX file: Missing 'status'.")
                break
            i = i + 1

        i = i + 1


def revert_ert(file):
    i = 0
    while 1:
        i = find_token(file.body, "\\begin_inset ERT", i)
        if i == -1:
            break

        # Seach for a line starting 'status'
        # If, however, we find a line starting '\begin_layout'
        # (_always_ present) then break with a warning message
        i = i + 1
        while 1:
            if (file.body[i] == "status open"):
                file.body[i] = "status Open"
                break
            elif (file.body[i] == "status collapsed"):
                file.body[i] = "status Collapsed"
                break
            elif (file.body[i] == "status inlined"):
                file.body[i] = "status Inlined"
                break
            elif (file.body[i][:13] == "\\begin_layout"):
                file.warning("Malformed LyX file : Missing 'status'.")
                break
            i = i + 1

        i = i + 1


##
# Minipages
#
def convert_minipage(file):
    """ Convert minipages to the box inset.
    We try to use the same order of arguments as lyx does.
    """
    pos = ["t","c","b"]
    inner_pos = ["c","t","b","s"]

    i = 0
    while 1:
        i = find_token(file.body, "\\begin_inset Minipage", i)
        if i == -1:
            return

        file.body[i] = "\\begin_inset Box Frameless"
        i = i + 1

        # convert old to new position using the pos list
        if file.body[i][:8] == "position":
            file.body[i] = 'position "%s"' % pos[int(file.body[i][9])]
        else:
            file.body.insert(i, 'position "%s"' % pos[0])
        i = i + 1

        file.body.insert(i, 'hor_pos "c"')
        i = i + 1
        file.body.insert(i, 'has_inner_box 1')
        i = i + 1

        # convert the inner_position
        if file.body[i][:14] == "inner_position":
            file.body[i] = 'inner_pos "%s"' %  inner_pos[int(file.body[i][15])]
        else:
            file.body.insert('inner_pos "%s"' % inner_pos[0])
        i = i + 1

        # We need this since the new file format has a height and width
        # in a different order.
        if file.body[i][:6] == "height":
            height = file.body[i][6:]
            # test for default value of 221 and convert it accordingly
            if height == ' "0pt"':
                height = ' "1pt"'
            del file.body[i]
        else:
            height = ' "1pt"'

        if file.body[i][:5] == "width":
            width = file.body[i][5:]
            del file.body[i]
        else:
            width = ' "0"'

        if file.body[i][:9] == "collapsed":
            if file.body[i][9:] == "true":
		status = "collapsed"
            else:
		status = "open"
            del file.body[i]
        else:
	    status = "collapsed"

        file.body.insert(i, 'use_parbox 0')
        i = i + 1
        file.body.insert(i, 'width' + width)
        i = i + 1
        file.body.insert(i, 'special "none"')
        i = i + 1
        file.body.insert(i, 'height' + height)
        i = i + 1
        file.body.insert(i, 'height_special "totalheight"')
        i = i + 1
        file.body.insert(i, 'status ' + status)
        i = i + 1


# -------------------------------------------------------------------------------------------
# Convert backslashes and '\n' into valid ERT code, append the converted
# text to body[i] and return the (maybe incremented) line index i
def convert_ertbackslash(body, i, ert):
    for c in ert:
	if c == '\\':
	    body[i] = body[i] + '\\backslash '
	    i = i + 1
	    body.insert(i, '')
	elif c == '\n':
	    body[i+1:i+1] = ['\\newline ', '']
	    i = i + 2
	else:
	    body[i] = body[i] + c
    return i


def convert_vspace(file):

    # Get default spaceamount
    i = find_token(file.header, '\\defskip', 0)
    if i == -1:
	defskipamount = 'medskip'
    else:
	defskipamount = split(file.header[i])[1]

    # Convert the insets
    i = 0
    while 1:
        i = find_token(file.body, '\\begin_inset VSpace', i)
        if i == -1:
            return
	spaceamount = split(file.body[i])[2]

	# Are we at the beginning or end of a paragraph?
	paragraph_start = 1
	start = get_paragraph(file.body, i) + 1
	for k in range(start, i):
	    if is_nonempty_line(file.body[k]):
		paragraph_start = 0
		break
	paragraph_end = 1
	j = find_end_of_inset(file.body, i)
	if j == -1:
	    file.warning("Malformed LyX file: Missing '\\end_inset'.")
	    i = i + 1
	    continue
	end = get_next_paragraph(file.body, i)
	for k in range(j + 1, end):
	    if is_nonempty_line(file.body[k]):
		paragraph_end = 0
		break

	# Convert to paragraph formatting if we are at the beginning or end
	# of a paragraph and the resulting paragraph would not be empty
	if ((paragraph_start and not paragraph_end) or
	    (paragraph_end   and not paragraph_start)):
	    # The order is important: del and insert invalidate some indices
	    del file.body[j]
	    del file.body[i]
	    if paragraph_start:
		file.body.insert(start, '\\added_space_top ' + spaceamount + ' ')
	    else:
		file.body.insert(start, '\\added_space_bottom ' + spaceamount + ' ')
	    continue

	# Convert to ERT
	file.body[i:i+1] = ['\\begin_inset ERT', 'status Collapsed', '',
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
	    file.body.insert(i, spaceamount)
	else :
	    if keep:
		file.body.insert(i, 'vspace*{')
	    else:
		file.body.insert(i, 'vspace{')
	    i = convert_ertbackslash(file.body, i, spaceamount)
            file.body[i] =  file.body[i] + '}'
        i = i + 1


# Convert a LyX length into a LaTeX length
def convert_len(len, special):
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

    return len


# Convert a LyX length into valid ERT code and append it to body[i]
# Return the (maybe incremented) line index i
def convert_ertlen(body, i, len, special):
    # Convert backslashes and insert the converted length into body
    return convert_ertbackslash(body, i, convert_len(len, special))


# Return the value of len without the unit in numerical form
def len2value(len):
    result = re.search('([+-]?[0-9.]+)', len)
    if result:
	return float(result.group(1))
    # No number means 1.0
    return 1.0


# Convert text to ERT and insert it at body[i]
# Return the index of the line after the inserted ERT
def insert_ert(body, i, status, text):
    body[i:i] = ['\\begin_inset ERT', 'status ' + status, '',
                 '\\layout Standard', '']
    i = i + 5
    i = convert_ertbackslash(body, i, text) + 1
    body[i:i] = ['', '\\end_inset', '']
    i = i + 3
    return i


# Add text to the preamble if it is not already there.
# Only the first line is checked!
def add_to_preamble(file, text):
    i = find_token(file.header, '\\begin_preamble', 0)
    if i == -1:
        file.header.extend(['\\begin_preamble'] + text + ['\\end_preamble'])
        return

    j = find_token(file.header, '\\end_preamble', i)
    if j == -1:
        file.warning("Malformed LyX file: Missing '\\end_preamble'.")
        file.warning("Adding it now and hoping for the best.")
        file.header.append('\\end_preamble')
        j = len(file.header)

    if find_token(file.header, text[0], i, j) != -1:
        return
    file.header[j:j] = text


def convert_frameless_box(file):
    pos = ['t', 'c', 'b']
    inner_pos = ['c', 't', 'b', 's']
    i = 0
    while 1:
        i = find_token(file.body, '\\begin_inset Frameless', i)
        if i == -1:
            return
	j = find_end_of_inset(file.body, i)
	if j == -1:
	    file.warning("Malformed LyX file: Missing '\\end_inset'.")
	    i = i + 1
	    continue
	del file.body[i]
	j = j - 1

	# Gather parameters
	params = {'position':0, 'hor_pos':'c', 'has_inner_box':'1',
                  'inner_pos':1, 'use_parbox':'0', 'width':'100col%',
	          'special':'none', 'height':'1in',
	          'height_special':'totalheight', 'collapsed':'false'}
	for key in params.keys():
	    value = replace(get_value(file.body, key, i, j), '"', '')
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
		j = del_token(file.body, key, i, j)
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

            # Here we know that this box is not supported in file format 224.
            # Therefore we need to convert it to ERT. We can't simply convert
            # the beginning and end of the box to ERT, because the
            # box inset may contain layouts that are different from the
            # surrounding layout. After the conversion the contents of the
            # box inset is on the same level as the surrounding text, and
            # paragraph layouts and align parameters can get mixed up.

            # A possible solution for this problem:
            # Convert the box to a minipage and redefine the minipage
            # environment in ERT so that the original box is simulated.
            # For minipages we could do this in a way that the width and
            # position can still be set from LyX, but this did not work well.
            # This is not possible for parboxes either, so we convert the
            # original box to ERT, put the minipage inset inside the box
            # and redefine the minipage environment to be empty.

            # Commands that are independant of a particular box can go to
            # the preamble.
            # We need to define lyxtolyxrealminipage with 3 optional
            # arguments although LyX 1.3 uses only the first one.
            # Otherwise we will get LaTeX errors if this document is
            # converted to format 225 or above again (LyX 1.4 uses all
            # optional arguments).
            add_to_preamble(file,
                ['% Commands inserted by lyx2lyx for frameless boxes',
                 '% Save the original minipage environment',
                 '\\let\\lyxtolyxrealminipage\\minipage',
                 '\\let\\endlyxtolyxrealminipage\\endminipage',
                 '% Define an empty lyxtolyximinipage environment',
                 '% with 3 optional arguments',
                 '\\newenvironment{lyxtolyxiiiminipage}[4]{}{}',
                 '\\newenvironment{lyxtolyxiiminipage}[2][\\lyxtolyxargi]%',
                 '  {\\begin{lyxtolyxiiiminipage}{\\lyxtolyxargi}{\\lyxtolyxargii}{#1}{#2}}%',
                 '  {\\end{lyxtolyxiiiminipage}}',
                 '\\newenvironment{lyxtolyximinipage}[1][\\totalheight]%',
                 '  {\\def\\lyxtolyxargii{{#1}}\\begin{lyxtolyxiiminipage}}%',
                 '  {\\end{lyxtolyxiiminipage}}',
                 '\\newenvironment{lyxtolyxminipage}[1][c]%',
                 '  {\\def\\lyxtolyxargi{{#1}}\\begin{lyxtolyximinipage}}',
                 '  {\\end{lyxtolyximinipage}}'])

            if params['use_parbox'] != '0':
                ert = '\\parbox'
            else:
                ert = '\\begin{lyxtolyxrealminipage}'

            # convert optional arguments only if not latex default
            if (pos[params['position']] != 'c' or
                inner_pos[params['inner_pos']] != pos[params['position']] or
                params['height_special'] != 'totalheight' or
                len2value(params['height']) != 1.0):
                ert = ert + '[' + pos[params['position']] + ']'
            if (inner_pos[params['inner_pos']] != pos[params['position']] or
                params['height_special'] != 'totalheight' or
                len2value(params['height']) != 1.0):
                ert = ert + '[' + convert_len(params['height'],
                                              params['height_special']) + ']'
            if inner_pos[params['inner_pos']] != pos[params['position']]:
                ert = ert + '[' + inner_pos[params['inner_pos']] + ']'

            ert = ert + '{' + convert_len(params['width'],
                                          params['special']) + '}'

            if params['use_parbox'] != '0':
                ert = ert + '{'
            ert = ert + '\\let\\minipage\\lyxtolyxminipage%\n'
            ert = ert + '\\let\\endminipage\\endlyxtolyxminipage%\n'

            old_i = i
            i = insert_ert(file.body, i, 'Collapsed', ert)
            j = j + i - old_i - 1

            file.body[i:i] = ['\\begin_inset Minipage',
                              'position %d' % params['position'],
                              'inner_position 1',
                              'height "1in"',
                              'width "' + params['width'] + '"',
                              'collapsed ' + params['collapsed']]
            i = i + 6
            j = j + 6

            # Restore the original minipage environment since we may have
            # minipages inside this box.
            # Start a new paragraph because the following may be nonstandard
            file.body[i:i] = ['\\layout Standard', '', '']
            i = i + 2
            j = j + 3
            ert = '\\let\\minipage\\lyxtolyxrealminipage%\n'
            ert = ert + '\\let\\endminipage\\lyxtolyxrealendminipage%'
            old_i = i
            i = insert_ert(file.body, i, 'Collapsed', ert)
            j = j + i - old_i - 1

            # Redefine the minipage end before the inset end.
            # Start a new paragraph because the previous may be nonstandard
            file.body[j:j] = ['\\layout Standard', '', '']
            j = j + 2
            ert = '\\let\\endminipage\\endlyxtolyxminipage'
            j = insert_ert(file.body, j, 'Collapsed', ert)
	    j = j + 1
            file.body.insert(j, '')
	    j = j + 1

            # LyX writes '%\n' after each box. Therefore we need to end our
            # ERT with '%\n', too, since this may swallow a following space.
            if params['use_parbox'] != '0':
                ert = '}%\n'
            else:
                ert = '\\end{lyxtolyxrealminipage}%\n'
            j = insert_ert(file.body, j, 'Collapsed', ert)

            # We don't need to restore the original minipage after the inset
            # end because the scope of the redefinition is the original box.

	else:

	    # Convert to minipage
	    file.body[i:i] = ['\\begin_inset Minipage',
	                      'position %d' % params['position'],
			      'inner_position %d' % params['inner_pos'],
			      'height "' + params['height'] + '"',
	                      'width "' + params['width'] + '"',
	                      'collapsed ' + params['collapsed']]
	    i = i + 6

##
# Convert jurabib
#

def convert_jurabib(file):
    i = find_token(file.header, '\\use_numerical_citations', 0)
    if i == -1:
        file.warning("Malformed lyx file: Missing '\\use_numerical_citations'.")
        return
    file.header.insert(i + 1, '\\use_jurabib 0')


def revert_jurabib(file):
    i = find_token(file.header, '\\use_jurabib', 0)
    if i == -1:
        file.warning("Malformed lyx file: Missing '\\use_jurabib'.")
        return
    if get_value(file.header, '\\use_jurabib', 0) != "0":
        file.warning("Conversion of '\\use_jurabib = 1' not yet implemented.")
        # Don't remove '\\use_jurabib' so that people will get warnings by lyx
        return
    del file.header[i]

##
# Convert bibtopic
#

def convert_bibtopic(file):
    i = find_token(file.header, '\\use_jurabib', 0)
    if i == -1:
        file.warning("Malformed lyx file: Missing '\\use_jurabib'.")
        return
    file.header.insert(i + 1, '\\use_bibtopic 0')


def revert_bibtopic(file):
    i = find_token(file.header, '\\use_bibtopic', 0)
    if i == -1:
        file.warning("Malformed lyx file: Missing '\\use_bibtopic'.")
        return
    if get_value(file.header, '\\use_bibtopic', 0) != "0":
        file.warning("Conversion of '\\use_bibtopic = 1' not yet implemented.")
        # Don't remove '\\use_jurabib' so that people will get warnings by lyx
    del file.header[i]

##
# Sideway Floats
#

def convert_float(file):
    i = 0
    while 1:
        i = find_token(file.body, '\\begin_inset Float', i)
        if i == -1:
            return
        # Seach for a line starting 'wide'
        # If, however, we find a line starting '\begin_layout'
        # (_always_ present) then break with a warning message
        i = i + 1
        while 1:
            if (file.body[i][:4] == "wide"):
                file.body.insert(i + 1, 'sideways false')
                break
            elif (file.body[i][:13] == "\\begin_layout"):
                file.warning("Malformed lyx file: Missing 'wide'.")
                break
            i = i + 1
        i = i + 1


def revert_float(file):
    i = 0
    while 1:
        i = find_token(file.body, '\\begin_inset Float', i)
        if i == -1:
            return
        j = find_end_of_inset(file.body, i)
        if j == -1:
            file.warning("Malformed lyx file: Missing '\\end_inset'.")
            i = i + 1
            continue
        if get_value(file.body, 'sideways', i, j) != "false":
            file.warning("Conversion of 'sideways true' not yet implemented.")
            # Don't remove 'sideways' so that people will get warnings by lyx
            i = i + 1
            continue
        del_token(file.body, 'sideways', i, j)
        i = i + 1


def convert_graphics(file):
    """ Add extension to filenames of insetgraphics if necessary.
    """
    i = 0
    while 1:
        i = find_token(file.body, "\\begin_inset Graphics", i)
        if i == -1:
            return

	j = find_token2(file.body, "filename", i)
        if j == -1:
            return
        i = i + 1
	filename = split(file.body[j])[1]
	absname = os.path.normpath(os.path.join(file.dir, filename))
	if file.input == stdin and not os.path.isabs(filename):
	    # We don't know the directory and cannot check the file.
	    # We could use a heuristic and take the current directory,
	    # and we could try to find out if filename has an extension,
	    # but that would be just guesses and could be wrong.
	    file.warning("""Warning: Can not determine whether file
         %s
         needs an extension when reading from standard input.
         You may need to correct the file manually or run
         lyx2lyx again with the .lyx file as commandline argument.""" % filename)
	    continue
	# This needs to be the same algorithm as in pre 233 insetgraphics
	if access(absname, F_OK):
	    continue
	if access(absname + ".ps", F_OK):
	    file.body[j] = replace(file.body[j], filename, filename + ".ps")
	    continue
	if access(absname + ".eps", F_OK):
	    file.body[j] = replace(file.body[j], filename, filename + ".eps")


##
# Convert firstname and surname from styles -> char styles
#
def convert_names(file):
    """ Convert in the docbook backend from firstname and surname style
    to charstyles.
    """
    if file.backend != "docbook":
        return

    i = 0

    while 1:
        i = find_token(file.body, "\\begin_layout Author", i)
        if i == -1:
            return

        i = i + 1
        while file.body[i] == "":
            i = i + 1

        if file.body[i][:11] != "\\end_layout" or file.body[i+2][:13] != "\\begin_deeper":
            i = i + 1
            continue

        k = i
        i = find_end_of( file.body, i+3, "\\begin_deeper","\\end_deeper")
        if i == -1:
            # something is really wrong, abort
            file.warning("Missing \\end_deeper, after style Author.")
            file.warning("Aborted attempt to parse FirstName and Surname.")
            return
        firstname, surname = "", ""

        name = file.body[k:i]

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
        del file.body[k+2:i+1]

        file.body[k-1:k-1] = ["", "",
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


def revert_names(file):
    """ Revert in the docbook backend from firstname and surname char style
    to styles.
    """
    if file.backend != "docbook":
        return


##
#    \use_natbib 1                       \cite_engine <style>
#    \use_numerical_citations 0     ->   where <style> is one of
#    \use_jurabib 0                      "basic", "natbib_authoryear",
#                                        "natbib_numerical" or "jurabib"
def convert_cite_engine(file):
    a = find_token(file.header, "\\use_natbib", 0)
    if a == -1:
        file.warning("Malformed lyx file: Missing '\\use_natbib'.")
        return

    b = find_token(file.header, "\\use_numerical_citations", 0)
    if b == -1 or b != a+1:
        file.warning("Malformed lyx file: Missing '\\use_numerical_citations'.")
        return

    c = find_token(file.header, "\\use_jurabib", 0)
    if c == -1 or c != b+1:
        file.warning("Malformed lyx file: Missing '\\use_jurabib'.")
        return

    use_natbib = int(split(file.header[a])[1])
    use_numerical_citations = int(split(file.header[b])[1])
    use_jurabib = int(split(file.header[c])[1])

    cite_engine = "basic"
    if use_natbib:
        if use_numerical_citations:
            cite_engine = "natbib_numerical"
        else:
             cite_engine = "natbib_authoryear"
    elif use_jurabib:
        cite_engine = "jurabib"

    del file.header[a:c+1]
    file.header.insert(a, "\\cite_engine " + cite_engine)


def revert_cite_engine(file):
    i = find_token(file.header, "\\cite_engine", 0)
    if i == -1:
        file.warning("Malformed lyx file: Missing '\\cite_engine'.")
        return

    cite_engine = split(file.header[i])[1]

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

    del file.header[i]
    file.header.insert(i, "\\use_jurabib " + use_jurabib)
    file.header.insert(i, "\\use_numerical_citations " + use_numerical)
    file.header.insert(i, "\\use_natbib " + use_natbib)


##
# Paper package
#
def convert_paperpackage(file):
    i = find_token(file.header, "\\paperpackage", 0)
    if i == -1:
        file.warning("Malformed lyx file: Missing '\\paperpackage'.")
        return

    packages = {'default':'none','a4':'none', 'a4wide':'a4', 'widemarginsa4':'a4wide'}
    if len(split(file.header[i])) > 1:
        paperpackage = split(file.header[i])[1]
    else:
        paperpackage = "default"
    file.header[i] = replace(file.header[i], paperpackage, packages[paperpackage])


def revert_paperpackage(file):
    i = find_token(file.header, "\\paperpackage", 0)
    if i == -1:
        file.warning("Malformed lyx file: Missing '\\paperpackage'.")
        return

    packages = {'none':'a4', 'a4':'a4wide', 'a4wide':'widemarginsa4',
                'widemarginsa4':'', 'default': 'default'}
    if len(split(file.header[i])) > 1:
        paperpackage = split(file.header[i])[1]
    else:
        paperpackage = 'default'
    file.header[i] = replace(file.header[i], paperpackage, packages[paperpackage])


##
# Bullets
#
def convert_bullets(file):
    i = 0
    while 1:
        i = find_token(file.header, "\\bullet", i)
        if i == -1:
            return
        if file.header[i][:12] == '\\bulletLaTeX':
            file.header[i] = file.header[i] + ' ' + strip(file.header[i+1])
            n = 3
        else:
            file.header[i] = file.header[i] + ' ' + strip(file.header[i+1]) +\
                        ' ' + strip(file.header[i+2]) + ' ' + strip(file.header[i+3])
            n = 5
        del file.header[i+1:i + n]
        i = i + 1


def revert_bullets(file):
    i = 0
    while 1:
        i = find_token(file.header, "\\bullet", i)
        if i == -1:
            return
        if file.header[i][:12] == '\\bulletLaTeX':
            n = find(file.header[i], '"')
            if n == -1:
                file.warning("Malformed header.")
                return
            else:
                file.header[i:i+1] = [file.header[i][:n-1],'\t' + file.header[i][n:], '\\end_bullet']
            i = i + 3
        else:
            frag = split(file.header[i])
            if len(frag) != 5:
                file.warning("Malformed header.")
                return
            else:
                file.header[i:i+1] = [frag[0] + ' ' + frag[1],
                                 '\t' + frag[2],
                                 '\t' + frag[3],
                                 '\t' + frag[4],
                                 '\\end_bullet']
                i = i + 5


##
# \begin_header and \begin_document
#
def add_begin_header(file):
    i = find_token(file.header, '\\lyxformat', 0)
    file.header.insert(i+1, '\\begin_header')
    file.header.insert(i+1, '\\begin_document')


def remove_begin_header(file):
    i = find_token(file.header, "\\begin_document", 0)
    if i != -1:
        del file.header[i]
    i = find_token(file.header, "\\begin_header", 0)
    if i != -1:
        del file.header[i]


##
# \begin_file.body and \end_file.body
#
def add_begin_body(file):
    file.body.insert(0, '\\begin_body')
    file.body.insert(1, '')
    i = find_token(file.body, "\\end_document", 0)
    file.body.insert(i, '\\end_body')

def remove_begin_body(file):
    i = find_token(file.body, "\\begin_body", 0)
    if i != -1:
        del file.body[i]
        if not file.body[i]:
            del file.body[i]
    i = find_token(file.body, "\\end_body", 0)
    if i != -1:
        del file.body[i]


##
# \papersize
#
def normalize_papersize(file):
    i = find_token(file.header, '\\papersize', 0)
    if i == -1:
        return

    tmp = split(file.header[i])
    if tmp[1] == "Default":
        file.header[i] = '\\papersize default'
        return
    if tmp[1] == "Custom":
        file.header[i] = '\\papersize custom'


def denormalize_papersize(file):
    i = find_token(file.header, '\\papersize', 0)
    if i == -1:
        return

    tmp = split(file.header[i])
    if tmp[1] == "custom":
        file.header[i] = '\\papersize Custom'


##
# Strip spaces at end of command line
#
def strip_end_space(file):
    for i in range(len(file.body)):
        if file.body[i][:1] == '\\':
            file.body[i] = strip(file.body[i])


##
# Use boolean values for \use_geometry, \use_bibtopic and \tracking_changes
#
def use_x_boolean(file):
    bin2bool = {'0': 'false', '1': 'true'}
    for use in '\\use_geometry', '\\use_bibtopic', '\\tracking_changes':
        i = find_token(file.header, use, 0)
        if i == -1:
            continue
        decompose = split(file.header[i])
        file.header[i] = decompose[0] + ' ' + bin2bool[decompose[1]]


def use_x_binary(file):
    bool2bin = {'false': '0', 'true': '1'}
    for use in '\\use_geometry', '\\use_bibtopic', '\\tracking_changes':
        i = find_token(file.header, use, 0)
        if i == -1:
            continue
        decompose = split(file.header[i])
        file.header[i] = decompose[0] + ' ' + bool2bin[decompose[1]]

##
# Place all the paragraph parameters in their own line
#
def normalize_paragraph_params(file):
    body = file.body
    allowed_parameters = '\\paragraph_spacing', '\\noindent', '\\align', '\\labelwidthstring', "\\start_of_appendix"

    i = 0
    while 1:
        i = find_token(file.body, '\\begin_layout', i)
        if i == -1:
            return

        i = i + 1
        while 1:
            if strip(body[i]) and split(body[i])[0] not in allowed_parameters:
                break

            j = find(body[i],'\\', 1)

            if j != -1:
                body[i:i+1] = [strip(body[i][:j]), body[i][j:]]

            i = i + 1


##
# Add/remove output_changes parameter
#
def convert_output_changes (file):
    i = find_token(file.header, '\\tracking_changes', 0)
    if i == -1:
        file.warning("Malformed lyx file: Missing '\\tracking_changes'.")
        return
    file.header.insert(i+1, '\\output_changes true')


def revert_output_changes (file):
    i = find_token(file.header, '\\output_changes', 0)
    if i == -1:
        return
    del file.header[i]


##
# Convert paragraph breaks and sanitize paragraphs
#
def convert_ert_paragraphs(file):
    forbidden_settings = [
                          # paragraph parameters
                          '\\paragraph_spacing', '\\labelwidthstring',
                          '\\start_of_appendix', '\\noindent',
                          '\\leftindent', '\\align',
                          # font settings
                          '\\family', '\\series', '\\shape', '\\size',
                          '\\emph', '\\numeric', '\\bar', '\\noun',
                          '\\color', '\\lang']
    i = 0
    while 1:
        i = find_token(file.body, '\\begin_inset ERT', i)
        if i == -1:
            return
        j = find_end_of_inset(file.body, i)
        if j == -1:
            file.warning("Malformed lyx file: Missing '\\end_inset'.")
            i = i + 1
            continue

        # convert non-standard paragraphs to standard
        k = i
        while 1:
            k = find_token(file.body, "\\begin_layout", k, j)
            if k == -1:
                break
            file.body[k] = "\\begin_layout Standard"
            k = k + 1

        # remove all paragraph parameters and font settings
        k = i
        while k < j:
            if (strip(file.body[k]) and
                split(file.body[k])[0] in forbidden_settings):
                del file.body[k]
                j = j - 1
            else:
                k = k + 1

        # insert an empty paragraph before each paragraph but the first
        k = i
        first_pagraph = 1
        while 1:
            k = find_token(file.body, "\\begin_layout Standard", k, j)
            if k == -1:
                break
            if first_pagraph:
                first_pagraph = 0
                k = k + 1
                continue
            file.body[k:k] = ["\\begin_layout Standard", "",
                              "\\end_layout", ""]
            k = k + 5
            j = j + 4

        # convert \\newline to new paragraph
        k = i
        while 1:
            k = find_token(file.body, "\\newline", k, j)
            if k == -1:
                break
            file.body[k:k+1] = ["\\end_layout", "", "\\begin_layout Standard"]
            k = k + 4
            j = j + 3
        i = i + 1


##
# Remove double paragraph breaks
#
def revert_ert_paragraphs(file):
    i = 0
    while 1:
        i = find_token(file.body, '\\begin_inset ERT', i)
        if i == -1:
            return
        j = find_end_of_inset(file.body, i)
        if j == -1:
            file.warning("Malformed lyx file: Missing '\\end_inset'.")
            i = i + 1
            continue

        # replace paragraph breaks with \newline
        k = i
        while 1:
            k = find_token(file.body, "\\end_layout", k, j)
            l = find_token(file.body, "\\begin_layout", k, j)
            if k == -1 or l == -1:
                break
            file.body[k:l+1] = ["\\newline"]
            j = j - l + k
            k = k + 1

        # replace double \newlines with paragraph breaks
        k = i
        while 1:
            k = find_token(file.body, "\\newline", k, j)
            if k == -1:
                break
            l = k + 1
            while file.body[l] == "":
                l = l + 1
            if strip(file.body[l]) and split(file.body[l])[0] == "\\newline":
                file.body[k:l+1] = ["\\end_layout", "",
                                    "\\begin_layout Standard"]
                j = j - l + k + 2
                k = k + 3
            else:
                k = k + 1
        i = i + 1


def convert_french(file):
    regexp = re.compile(r'^\\language\s+frenchb')
    i = find_re(file.header, regexp, 0)
    if i != -1:
        file.header[i] = "\\language french"


##
# Convertion hub
#

convert = [[223, [insert_tracking_changes, add_end_header, remove_color_default,
                  convert_spaces, convert_bibtex, remove_insetparent]],
           [224, [convert_external, convert_comment]],
           [225, [add_end_layout, layout2begin_layout, convert_end_document,
                  convert_table_valignment_middle, convert_breaks]],
           [226, [convert_note]],
           [227, [convert_box]],
           [228, [convert_collapsable, convert_ert]],
           [229, [convert_minipage]],
           [230, [convert_jurabib]],
           [231, [convert_float]],
           [232, [convert_bibtopic]],
           [233, [convert_graphics, convert_names]],
           [234, [convert_cite_engine]],
           [235, [convert_paperpackage]],
           [236, [convert_bullets, add_begin_header, add_begin_body,
                  normalize_papersize, strip_end_space]],
           [237, [use_x_boolean]],
           [238, [update_latexaccents]],
           [239, [normalize_paragraph_params]],
           [240, [convert_output_changes]],
           [241, [convert_ert_paragraphs]],
           [242, [convert_french]]]

revert =  [[241, []],
           [240, [revert_ert_paragraphs]],
           [239, [revert_output_changes]],
           [238, []],
           [237, []],
           [236, [use_x_binary]],
           [235, [denormalize_papersize, remove_begin_body,remove_begin_header,
                  revert_bullets]],
           [234, [revert_paperpackage]],
           [233, [revert_cite_engine]],
           [232, [revert_names]],
           [231, [revert_bibtopic]],
           [230, [revert_float]],
           [229, [revert_jurabib]],
           [228, []],
           [227, [revert_collapsable, revert_ert]],
           [226, [revert_box, revert_external_2]],
           [225, [revert_note]],
           [224, [rm_end_layout, begin_layout2layout, revert_end_document,
                  revert_valignment_middle, convert_vspace, convert_frameless_box]],
           [223, [revert_external_2, revert_comment, revert_eqref]],
           [221, [rm_end_header, revert_spaces, revert_bibtex,
                  rm_tracking_changes, rm_body_changes]]]


if __name__ == "__main__":
    pass
