#!/usr/bin/python
# This is a general wrapper script that will allow
# us to maintain security in the external material
# insets.
# Use like this:
#   general_command_wrapper.py stdin-filename stdout-filename command args
# Use "-" for stdin-filename and stdout-filename to use the normal stdio

import sys
import os

if sys.argv[1] != "-":
	os.close(0)
	sys.stdin = open(sys.argv[1],"r")
if sys.argv[2] != "-":
	print "Redirecting" + sys.argv[2]
	os.close(1)
	os.close(2)
	sys.stdout = open(sys.argv[2],"w")
	sys.stderr = open(sys.argv[2],"w")

os.execvp(sys.argv[3], sys.argv[3:])
print "Could not run " + sys.argv[3]

