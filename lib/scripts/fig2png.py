#! /usr/bin/env python
# This script converts a xfig file into PNG files

import sys
import os

filename = sys.argv[1]
basename = os.path.splitext(filename)[0]
parameters = sys.argv[2:]

pid = os.fork()
if pid == 0:
	os.execvp("fig2dev", ["fig2dev", "-Lpng"] + parameters + [filename, basename + ".png"])
	print "fig2dev did not work"
	os.exit(1)
os.wait()
