# This file is part of lyx2lyx
# -*- coding: utf-8 -*-
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

" This modules tests the functions used to help parse lines."

from parser_tools import *

import unittest

ug = r"""
\begin_layout Standard
The
\begin_inset Quotes eld
\end_inset


\emph on
Introduction
\emph default

\begin_inset Quotes erd
\end_inset

 describes several things in addition to LyX's philosophy: most importantly,
 the format of all of the manuals.
 If you don't read it, you'll have a bear of a time navigating this manual.
 You might also be better served looking in one of the other manuals instead
 of this one.

\begin_inset Quotes eld
\end_inset


\emph on
Introduction
\emph default

\begin_inset Quotes erd
\end_inset

 describes that, too.
\end_layout

"""

lines = ug.splitlines()

class TestParserTools(unittest.TestCase):

    def test_check_token(self):
        line = "\\begin_layout Standard"

        self.assertEquals(check_token(line, '\\begin_layout'), True)
        self.assertEquals(check_token(line, 'Standard'), False)


    def test_is_nonempty_line(self):
        self.assertEquals(is_nonempty_line(lines[0]), False)
        self.assertEquals(is_nonempty_line(lines[1]), True)
        self.assertEquals(is_nonempty_line(" "*5), False)


    def test_find_token(self):
        self.assertEquals(find_token(lines, '\\emph', 0), 7)
        self.assertEquals(find_token(lines, '\\emph', 0, 5), -1)
        self.assertEquals(find_token(lines, '\\emp', 0, exact = True), -1)
        self.assertEquals(find_token(lines, '\\emp', 0, exact = False), 7)
        self.assertEquals(find_token(lines, 'emph', 0), -1)


    def test_find_tokens(self):
        tokens = ['\\emph', '\\end_inset']
        self.assertEquals(find_tokens(lines, tokens, 0), 4)
        self.assertEquals(find_tokens(lines, tokens, 0, 4), -1)


if __name__ == '__main__':  
    unittest.main() 
