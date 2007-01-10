#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file fen2ascii.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Kayvan A. Sylvan

# Full author contact details are available in file CREDITS.

# This script will convert a chess position in the FEN
# format to an ascii representation of the position.

import sys, string, os, locale

# We expect two args, the names of the input and output files.
if len(sys.argv) != 3:
    sys.exit(1)

language, output_encoding = locale.getdefaultlocale()
if output_encoding == None:
    output_encoding = 'latin1'

input = unicode(sys.argv[1], 'utf8').encode(output_encoding)
output = unicode(sys.argv[2], 'utf8').encode(output_encoding)

os.close(0)
os.close(1)
sys.stdin = open(input, "r")
sys.stdout = open(output, "w")

line = sys.stdin.readline()
if line[-1] == '\n':
    line = line[:-1]

line=string.split(line,' ')[0]
comp=string.split(line,'/')

cont=1
margin= " "*6

print margin+'   +'+"-"*15+'+'
for i in range(8):

    cont = cont + 1
    tmp=""
    for j in comp[i]:
	if j>='0' and j <= '9':
	    for k in range(int(j)):
		cont = cont + 1
		x, mod = divmod(cont,2)
		if mod : tmp = tmp + '| '
		else : tmp = tmp + '|*'
	else :
	    tmp = tmp + '|' + j
	    cont = cont + 1

    row = 8 - i
    print margin, row, tmp+"|"

print margin+'   +'+"-"*15+'+'
print margin+'    a b c d e f g h '
