#! /usr/bin/env python

###############
import sys, os, shutil

if os.stat(sys.argv[1]).st_size > 0:
    shutil.copyfile(sys.argv[2], sys.argv[3])
