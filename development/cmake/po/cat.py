#! /usr/bin/env python

import sys
from getopt import getopt

usage = '''
python cat.py -o OUTFILE FILE1 FILE2 .... FILEn

Replacement for:
	cat FILE1 FILE2 ... .FILEn > OUTFILE
If the -o argument is not given, writes to stdout.
'''

outfile = ""

(options, args) = getopt(sys.argv[1:], "ho:")
for (opt, param) in options:
	if opt == "-o":
		outfile = param
	elif opt == "-h":
		print usage
		sys.exit(0)

out = sys.stdout
if outfile:
	out = open(outfile, "w")

for f in args:
	fil = open(f, "r")
	for l in fil:
		out.write(l)
	fil.close()

if outfile:
	out.close()
