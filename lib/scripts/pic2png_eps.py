#!/usr/bin/python
# This script converts a raster format picture into a PNG and EPS file

import sys
import os

if len(sys.argv) > 2:
	pars = sys.argv[2]

pid = os.fork()
if pid == 0:
	os.execvp("convert", ["convert", pars, sys.argv[1], os.path.splitext(sys.argv[1])[0] + ".eps"])
	print "convert did not work"
	os.exit(1)
os.wait()

pid = os.fork()
if pid == 0:
	os.execvp("convert", ["convert", pars, sys.argv[1], os.path.splitext(sys.argv[1])[0] + ".png"])
	print "convert did not work second time"
	os.exit(1)
os.wait()
