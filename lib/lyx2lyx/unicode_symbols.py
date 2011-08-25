# This file is part of lyx2lyx
# -*- coding: utf-8 -*-
# Copyright (C) 2011 The LyX team
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

" Import unicode_reps from this module for access to the unicode<->LaTeX mapping. "

import sys, os, re

def read_unicodesymbols():
    " Read the unicodesymbols list of unicode characters and corresponding commands."
    pathname = os.path.abspath(os.path.dirname(sys.argv[0]))
    fp = open(os.path.join(pathname.strip('lyx2lyx'), 'unicodesymbols'))
    spec_chars = []
    # Two backslashes, followed by some non-word character, and then a character
    # in brackets. The idea is to check for constructs like: \"{u}, which is how
    # they are written in the unicodesymbols file; but they can also be written
    # as: \"u or even \" u.
    r = re.compile(r'\\\\(\W)\{(\w)\}')
    for line in fp.readlines():
        if line[0] != '#' and line.strip() != "":
            line=line.replace(' "',' ') # remove all quotation marks with spaces before
            line=line.replace('" ',' ') # remove all quotation marks with spaces after
            line=line.replace(r'\"','"') # replace \" by " (for characters with diaeresis)
            try:
                [ucs4,command,dead] = line.split(None,2)
                if command[0:1] != "\\":
                    continue
                spec_chars.append([command, unichr(eval(ucs4))])
            except:
                continue
            m = r.match(command)
            if m != None:
                command = "\\\\"
                # If the character is a double-quote, then we need to escape it, too,
                # since it is done that way in the LyX file.
                if m.group(1) == "\"":
                    command += "\\"
                commandbl = command
                command += m.group(1) + m.group(2)
                commandbl += m.group(1) + ' ' + m.group(2)
                spec_chars.append([command, unichr(eval(ucs4))])
                spec_chars.append([commandbl, unichr(eval(ucs4))])
    fp.close()
    return spec_chars


unicode_reps = read_unicodesymbols()
