#! /usr/bin/env python

###############
import sys, os, shutil

if os.stat(sys.argv[1]).st_size > 0 or not os.path.isfile(sys.argv[3]):
    shutil.copyfile(sys.argv[2], sys.argv[3])
