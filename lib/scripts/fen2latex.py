#!/usr/bin/python
#
# Copyright (C) 2001 The LyX Team.
#
# This file is distributed under the GPL license.
#
# This script will convert a chess position in the FEN
# format to a chunk of LaTeX to be used with the skak.sty
# style.

import sys,string,os

os.close(0)
os.close(1)
sys.stdin = open(sys.argv[1],"r")
sys.stdout = open(sys.argv[2],"w")

line = sys.stdin.readline()
if line[-1] == '\n':
    line = line[:-1]

print "\\fenboard{"+line+"}" 
print "\\showboard%"

