#! /usr/bin/env python

from __future__ import print_function

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
		print(usage)
		sys.exit(0)

out = sys.stdout
if outfile:
	# always write unix line endings, even on windows
	out = open(outfile, "wb")

for f in args:
	# accept both windows and unix line endings, since it can happen that we
	# are on unix, but the file has been written on windows or vice versa.
	fil = open(f, "rU")
	for l in fil:
		# this does always write unix line endings since the file has
		# been opened in binary mode. This is needed since both gettext
		# and our .pot file manipulation scripts assume unix line ends.
		out.write(l)
	fil.close()

if outfile:
	out.close()
