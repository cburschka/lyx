#!/usr/bin/python
#
# Copyright (C) 2000 The LyX Team.
#
# This file is distributed under the GPL license.
#
# This script will convert a chess position in the FEN
# format to a chunk of LaTeX to be used with the chess.sty
# style.

import sys,string,os

os.close(0)
os.close(1)
sys.stdin = open(sys.argv[1],"r")
sys.stdout = open(sys.argv[2],"w")

line = sys.stdin.readline()
if line[-1] == '\n':
    line = line[:-1]

line=string.split(line,' ')[0]
comp=string.split(line,'/')

first = 1
cont=1
margin= " "*6

for i in range(8):

    cont = cont + 1
    tmp=""
    for j in comp[i]:
	if j>='0' and j <= '9':
	    for k in range(int(j)):
		cont = cont + 1
		x, mod = divmod(cont,2)
		if mod : tmp = tmp + ' '
		else : tmp = tmp + '*'
	else :
	    tmp = tmp + j
	    cont = cont + 1

    if first: 
	first = 0
	print "\\board{"+tmp+"}"
    else : 
	print margin+"{"+tmp+"}"

print "\\showboard%"

