#!/usr/bin/python
# This script converts a xfig file into Postscript/LaTeX files

import sys
import os

filename = sys.argv[1]
basename = os.path.splitext(filename)[0]
parameters = sys.argv[2:]

pid = os.fork()
if pid == 0:
	os.execvp("fig2dev", ["fig2dev", "-Lpstex"] + parameters + [filename, basename + ".eps"])
	print "fig2dev did not work"
	os.exit(1)
os.wait()

pid = os.fork()
if pid == 0:
	os.execvp("fig2dev", ["fig2dev", "-Lpstex_t"] + parameters + [filename, basename + ".pstex_t"])
	print "convert did not work second time"
	os.exit(1)
os.wait()
