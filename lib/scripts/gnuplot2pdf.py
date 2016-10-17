#!/usr/bin/python

from subprocess import Popen, PIPE
from sys import argv, stderr, exit
import os
import shutil

if (len(argv) != 3):
    stderr.write("Usage: %s <src_file> <dst_file>\n" % argv[0])
    exit(1)

with open(argv[1], 'rb') as fsrc:
    subproc = Popen("gnuplot", shell=True, stdin=PIPE)
    subproc.stdin.write("set terminal pdf\nset output '%s'\n" % argv[2])
    shutil.copyfileobj(fsrc, subproc.stdin)
    subproc.stdin.write("exit\n")
    subproc.wait()
