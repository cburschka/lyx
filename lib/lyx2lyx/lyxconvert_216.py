# This file is part of lyx2lyx
# Copyright (C) 2002 José Matos <jamatos@lyx.org>
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

import re, string, sys
from parser_tools import *

lyxtable_re = re.compile(r".*\\LyXTable$")
def update_tabular(lines):
    i=0
    while 1:
        i = find_re(lines, lyxtable_re, i)
        if i == -1:
            break
        prop_dict = {"family" : "default", "series" : "default",
                      "shape" : "default", "size" : "default",
                      "emph" : "default", "bar" : "default",
                      "noun" : "default", "latex" : "default", "color" : "default"}

        # remove \LyXTable
        lines[i] = lines[i][:-9]
        i = i + 1
        lines.insert(i,'')
        i = i + 1
        lines[i] = "\\begin_inset  Tabular"
        i = i + 1
        head = string.split(lines[i])
        rows = int(head[0])
        columns = int(head[1])

        tabular_line = i
        i = i +1
        lines.insert(i, '<Features rotate="%s" islongtable="%s" endhead="%s" endfirsthead="%s" endfoot="%s" endlastfoot="%s">' % (head[2],head[3],head[4],head[5],head[6],head[7]))

        i = i +1
        
        row_info = []
        cont_row = []
        for j in range(rows):
            row_info.append(string.split(lines[i]))
            if string.split(lines[i])[2] == '1':
                cont_row.append(j)
            del lines[i]

        column_info = []
        col_info_re = re.compile(r'(\d) (\d) (\d) (".*") (".*")')
        for j in range(columns):
            column_info.append(col_info_re.match(lines[i]).groups())
            del lines[i]

        cell_info = []
        cell_col = []
        ncells = 0
        cell_re = re.compile(r'(\d) (\d) (\d) (\d) (\d) (\d) (\d) (".*") (".*")')
        for j in range(rows):
            for k in range(columns):
                #add column location to read properties
                cell_info.append(cell_re.match(lines[i]).groups())
                cell_col.append(k)
                if lines[i][0] != "2":
                    ncells = ncells + 1
                del lines[i]

        lines[tabular_line] = '<LyXTabular version="1" rows="%s" columns="%s">' % (rows-len(cont_row),columns)
        del lines[i]
        if not lines[i]:
            del lines[i]
        
        # Read cells
        l = 0
        cell_content = []
        for j in range(rows):
            cell_content.append([])

        for j in range(rows):
            for k in range(columns):
                cell_content[j].append([])

        for j in range(rows):
            for k in range(columns):
                m = j*columns + k
                if cell_info[m][0] == '2':
                    continue

                if l == ncells -1:
                    # the end variable refers to cell end, not to file end.
                    end = find_tokens(lines, ['\\layout','\\the_end','\\end_deeper','\\end_float'], i)
                else:
                    end = find_token(lines, '\\newline ', i)

                if end == -1:
                    sys.stderr.write("Malformed lyx file\n")
                    sys.exit(1)

                end = end - i
                while end > 0:
                    cell_content[j][k].append(lines[i])
                    del lines[i]
                    end = end -1

                if lines[i] == '\\newline ':
                    del lines[i]
                l = l + 1

        tmp = []
        tmp.append("")

        for j in range(rows):
            if j in cont_row:
                continue
            tmp.append('<Row topline="%s" bottomline="%s" newpage="%s">' % (row_info[j][0],row_info[j][1],row_info[j][3]))

            for k in range(columns):
                if j:
                    tmp.append('<Column>')
                else:
                    tmp.append('<Column alignment="%s" valignment="0" leftline="%s" rightline="%s" width=%s special=%s>' % (column_info[k][0],column_info[k][1], column_info[k][2], column_info[k][3], column_info[k][4]))
                m = j*columns + k

                leftline = int(column_info[k][1])
                if cell_info[m][0] == '1':
                    n = m + 1
                    while n < rows * columns - 1 and cell_info[n][0] == '2':
                        n = n + 1
                    rightline = int(column_info[cell_col[n-1]][2])
                else:
                    # not a multicolumn main cell
                    rightline = int(column_info[k][2])

                tmp.append('<Cell multicolumn="%s" alignment="%s" valignment="0" topline="%s" bottomline="%s" leftline="%d" rightline="%d" rotate="%s" usebox="%s" width=%s special=%s>' % (cell_info[m][0],cell_info[m][1],cell_info[m][2],cell_info[m][3],leftline,rightline,cell_info[m][5],cell_info[m][6],cell_info[m][7],cell_info[m][8]))
                tmp.append('\\begin_inset Text')
                tmp.append('')
                tmp.append('\\layout Standard')
                tmp.append('')

                if cell_info[m][0] != '2':
                    paragraph = []
                    if cell_info[m][4] == '1':
                        l = j
                        paragraph.extend(cell_content[j][k])
                        while cell_info[m][4] == '1':
                            m = m + columns
                            l = l + 1
                            paragraph.extend(cell_content[l][k])
                    else:
                        paragraph = cell_content[j][k]
                    tmp.extend(set_paragraph_properties(paragraph, prop_dict))
                
                tmp.append('\\end_inset ')
                tmp.append('</Cell>')
                tmp.append('</Column>')
            tmp.append('</Row>')

        tmp.append('</LyXTabular>')
        tmp.append('')
        tmp.append('\\end_inset ')
        tmp.append('')
        tmp.append('')
        tail = lines[i:]
        lines[i:] = []
        lines.extend(tmp)
        lines.extend(tail)

        i = i + len(tmp)

prop_exp = re.compile(r"\\(\S*)\s*(\S*)")
def set_paragraph_properties(lines, prop_dict):
    # we need to preserve the order of options
    properties = ["family","series","shape","size",
                  "emph","bar","noun","latex","color"]
    prop_value = {"family" : "default", "series" : "medium",
                   "shape" : "up", "size" : "normal",
                   "emph" : "off", "bar" : "no",
                   "noun" : "off", "latex" : "no_latex", "color" : "none"}

    start = 0
    end = 0
    i = 0
    n = len(lines)

    #skip empty lines
    while i<n and lines[i] == "":
        i = i + 1
    start = i

    #catch open char properties
    while i<n and lines[i][:1] == "\\":
        result = prop_exp.match(lines[i])
        # sys.stderr.write(lines[i]+"\n")
        prop = result.group(1)
        if prop not in properties:
            break
        else:
            prop_dict[prop] = result.group(2)
        i = i + 1
    end = i

    aux = []
    insert = 0
    for prop in properties:
        if prop_dict[prop] != 'default':
            insert = 1
            if prop == "color":
                aux.append("\\%s %s" % (prop, prop_dict[prop]))
            elif prop != "family" or prop_dict[prop] != "roman":
                    aux.append("\\%s %s " % (prop, prop_dict[prop]))

    # remove final char properties
    n = len(lines)
    changed_prop = []

    while n:
        n = n - 1
        if not lines[n]:
            del lines[n]
            continue

        if lines[n][:1] == '\\':
            result = prop_exp.match(lines[n])
            prop = result.group(1)
            if prop in properties:
                changed_prop.append(prop)
                prop_dict[prop] = result.group(2)
                del lines[n]
                continue

            if check_token(lines[n],'\\end_inset'):
                # ensure proper newlines after inset end
                lines.append('')
                lines.append('')
        break

    for line in lines[end:]:
        if line[:1] == '\\':
            result = prop_exp.match(line)
            prop = result.group(1)
            if prop in properties and prop not in changed_prop:
                prop_dict[prop] = result.group(2)

    if not lines[start:] and not lines[end:]:
        return []
    
    result = lines[:start] + aux[:] + lines[end:]
    if insert and result[0] != '':
        return [''] + result[:]

    return result[:]

def update_language(header):
    i = find_token(header, "\\language", 0)
    if i == -1:
        # no language, should emit a warning
        header.append('\\language english')
        return
    # This is the lyx behaviour: defaults to english
    header[i] = '\\language english'
    return

def convert(header,body):
    update_tabular(body)
    update_language(header)

if __name__ == "__main__":
    pass

