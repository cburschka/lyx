#!/usr/bin/python
# This script converts a raster format picture into an ascii representation
# with the suffix .asc

import sys
import os
import os.path
import string

pid = os.fork()
if pid == 0:
	os.execvp("convert", ["convert", sys.argv[1], "temporary_filename_that_is_long.gif"])
	print "Could not run convert"
	os.exit(1)
os.wait()

os.system("identify temporary_filename_that_is_long.gif > temp.dim")

fp = open("temp.dim", "r")
line = fp.readline()
lines = string.split(line,' ')
dims = string.split(lines[1],'x')
xsize = float(dims[0])
ysize = float(string.split(dims[1],'+')[0])

aspect_ratio = xsize / ysize

if len(sys.argv) > 2:
	resulting_x = int(sys.argv[2])
else:
	resulting_x = 40
resulting_y = int(resulting_x / aspect_ratio)

os.system("echo s | gifscii temporary_filename_that_is_long.gif %d %d" % (resulting_x, resulting_y))

os.system("tail +3 temporary_filename_that_is_long.asc > temporary_filename_that_is_long2.asc")

pid = os.fork()
if pid == 0:
	os.execvp("mv", ["mv", "temporary_filename_that_is_long2.asc", os.path.splitext(sys.argv[1])[0] + ".asc"])
	print "Could not rename file"
	os.exit(1)
os.wait(pid)

os.system("rm temporary_filename_that_is_long.gif temporary_filename_that_is_long.asc")

