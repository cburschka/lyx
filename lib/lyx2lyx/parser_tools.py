# This file is part of lyx2lyx
# Copyright (C) 2002 Dekel Tsur <dekel@lyx.org>, José Matos <jamatos@lyx.org>
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

def check_token(lines, token, i):
    if lines[i][:len(token)] == token:
	return 1
    return 0

def find_token(lines, token, start):
    n = len(lines)
    m = len(token)
    i = start
    while i < n:
	line = lines[i]
	if line[:m] == token:
	    return i
	i = i+1
    return -1

def find_token2(lines, token1, token2, start):
    n = len(lines)
    m1 = len(token1)
    m2 = len(token2)
    i = start
    while i < n:
	line = lines[i]
	if line[:m1] == token1 or line[:m2] == token2:
	    return i
	i = i+1
    return -1

def find_token_backwards(lines, token, start):
    n = len(lines)
    m = len(token)
    i = start
    while i >= 0:
	line = lines[i]
	if line[:m] == token:
	    return i
	i = i-1
    return -1

def set_format(lines, number):
    i = find_token(lines, "\\lyxformat", 0)
    lines[i] = "\\lyxformat %s" % number
